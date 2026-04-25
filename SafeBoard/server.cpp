#include "server.h"

int server_running = 1;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    return -1;
  }
  char *path = argv[1];
  char *port = argv[2];
  signals_configure();

  int serverSocketFD = creating_socket(std::stoi(port));
  std::vector<std::string> patterns;
  read_patterns(patterns, path);

  Stats *stats_pointer = creating_shared_memory();

  int fifo_id = starting_fifo_process(serverSocketFD, stats_pointer, patterns);
  sem_t *sem = sem_open("/sem", O_CREAT, 0666, 1);
  while (server_running) {
    int clientSocketFD = accepting_client(serverSocketFD);
    if (clientSocketFD == -1) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }
    int id = fork();
    if (id == -1) {
      return 4;
    }
    if (id == 0) {
      if (handle_client(serverSocketFD, clientSocketFD, sem, stats_pointer,
                        patterns) == 1) {
        return 5;
      }
    } else {
      close(clientSocketFD);
    }
  }
  exiting(fifo_id, stats_pointer, sem, serverSocketFD);
}

DetectedPatterns find_pattern(std::string &file,
                              const std::vector<std::string> &patterns) {
  DetectedPatterns dp;
  for (const std::string &i : patterns) {
    size_t index_found = file.find(i);
    if (index_found != std::string::npos) {
      ++dp.quantity;
      dp.detectted_patterns.push_back(i);
    }
  }
  return dp;
}

void read_patterns(std::vector<std::string> &file_patterns, char *path) {
  std::ifstream in(path);
  if (in.is_open()) {
    std::string line;
    while (getline(in, line)) {
      file_patterns.push_back(line);
    }
  }
  in.close();
}


void handler(int) { server_running = 0; }

void send_to_stat_utils(const Stats *stats_pointer,
                        const std::vector<std::string> &patterns) {
  int fd = open("/tmp/myfifo", O_WRONLY);
  if (fd == -1) {
    return;
  }
  std::string statistic;
  statistic = std::to_string(stats_pointer->file_checked) +
              " files were checked for all time. Here the statistics: \n";
  for (size_t i = 0; i < patterns.size(); ++i) {
    if (stats_pointer->pattern_found[i] > 0)
      statistic += patterns[i] + " :" +
                   std::to_string(stats_pointer->pattern_found[i]) + " times\n";
  }

  uint32_t stat_lenght = statistic.length();
  write(fd, &stat_lenght, sizeof(uint32_t));
  write(fd, statistic.data(), statistic.size());
  close(fd);
}

void sending_response_to_client(std::string file,
                                std::vector<std::string> &patterns, sem_t *sem,
                                Stats *stats_pointer, int clientSocketFD) {
  DetectedPatterns dp = find_pattern(file, patterns);
  sem_wait(sem);
  ++stats_pointer->file_checked;
  std::string response;
  if (dp.quantity == 0) {
    response = "No patterns detected in file\n";
  } else {
    response = "Detected " + std::to_string(dp.quantity) + " patterns:" + "\n";
    for (const std::string &i : dp.detectted_patterns) {
      response += i + "\n";
      int pattern_index =
          std::find(patterns.begin(), patterns.end(), i) - patterns.begin();
      ++stats_pointer->pattern_found[pattern_index];
    }
  }
  sem_post(sem);
  uint32_t response_lenght = response.size();
  send(clientSocketFD, &response_lenght, sizeof(uint32_t), 0);
  send(clientSocketFD, response.c_str(), response_lenght, 0);
}

int handle_client(int serverSocketFD, int clientSocketFD, sem_t *sem,
                  Stats *stats_pointer, std::vector<std::string> &patterns) {
  close(serverSocketFD);
  uint32_t lenght_file;
  recv(clientSocketFD, &lenght_file, sizeof(uint32_t), 0);
  std::string file;
  file.resize(lenght_file);
  uint32_t recv_status =
      recv(clientSocketFD, (void *)file.data(), lenght_file, 0);
  while (recv_status != lenght_file) {
    int recv_add =
        recv(clientSocketFD, &file[recv_status], lenght_file - recv_status, 0);
    if (recv_add <= 0) {
      return 1;
    }
    recv_status += recv_add;
  }

  sending_response_to_client(file, patterns, sem, stats_pointer,
                             clientSocketFD);
  sem_close(sem);
  close(clientSocketFD);
  exit(0);
}

int creating_socket(int port) {
  int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocketFD == -1) {
    return -1;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(port);


  int bind_status = bind(serverSocketFD, (sockaddr *)&address, sizeof(address));
  if (bind_status != 0) {
    return -1;
  }

  int listen_status = listen(serverSocketFD, 10);
  if (listen_status != 0) {
    return -1;
  }
  return serverSocketFD;
}

Stats *creating_shared_memory() {
  Stats *stats_pointer =
      (Stats *)mmap(nullptr, sizeof(Stats), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *stats_pointer = Stats{};
  return stats_pointer;
}

void exiting(int fifo_id, Stats *stats_pointer, sem_t *sem,
             int serverSocketFD) {
  kill(fifo_id, SIGTERM);
  waitpid(fifo_id, nullptr, 0);
  while (waitpid(-1, nullptr, 0) > 0)
    ;
  munmap(stats_pointer, sizeof(Stats));
  sem_close(sem);
  sem_unlink("/sem");
  unlink(fifo_path);
  close(serverSocketFD);
}

int starting_fifo_process(int serverSocketFD, Stats *stats_pointer,
                          const std::vector<std::string> &patterns) {
  if (mkfifo(fifo_path, 0666) == -1) {
    if (errno != EEXIST) {
      return -1;
    }
  }
  int fifo_id = fork();
  if (fifo_id == -1) {
    return -1;
  }
  if (fifo_id == 0) {
    close(serverSocketFD);
    while (server_running) {
      send_to_stat_utils(stats_pointer, patterns);
      sleep(1);
    }
    exit(0);
  }
  return fifo_id;
}

int accepting_client(int serverSocketFD) {
  sockaddr_in client_address{};
  socklen_t address_len = sizeof(client_address);
  int clientSocketFD =
      accept(serverSocketFD, (sockaddr *)&client_address, &address_len);
  return clientSocketFD;
}

void signals_configure() {
  struct sigaction sig_action {};
  sig_action.sa_handler = handler;
  sig_action.sa_flags = 0;

  sigaction(SIGINT, &sig_action, nullptr);
  sigaction(SIGTERM, &sig_action, nullptr);
  signal(SIGCHLD, SIG_IGN);
}
