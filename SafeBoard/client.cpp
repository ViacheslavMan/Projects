#include "client.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    return -1;
  }
  char* path = argv[1];
  char* port = argv[2];

  std::string file;
  if (reading_file(file, path) == -1) {
    return -1;
  }

  int socket_fd =
      socket(AF_INET, SOCK_STREAM, 0); 
  if (socket_fd == -1) {
    return 1;
  }

  sockaddr_in address{};
  constract_socket(address, port);

  int connect_status = connect(socket_fd, (sockaddr*)&address, sizeof(address));
  if (connect_status == -1) {
    return 2;
  }

  uint32_t lenght_file = file.size();
  send(socket_fd, &lenght_file, sizeof(uint32_t), 0);
  int send_status = send(socket_fd, file.c_str(), lenght_file, 0);
  if (send_status == -1) {
    return 3;
  }

  std::string response;
  uint32_t response_lenght;
  recv(socket_fd, &response_lenght, sizeof(uint32_t), 0);
  response.resize(response_lenght);
  int recv_status = recv(socket_fd, (void*)response.data(), response_lenght, 0);
  while (recv_status != response_lenght) {
    int recv_add = recv(socket_fd, &response[recv_status],
                        response_lenght - recv_status, 0);
    if (recv_add <= 0) {
      return 4;
    }
    recv_status += recv_add;
  }
  std::cout << response;

  close(socket_fd);
}

int reading_file(std::string& file, const char* path) {
  std::ifstream in(path);
  if (in.is_open()) {
    std::string line;
    while (getline(in, line)) {
      file += line;
      file += "\n";
    }
  } else {
    return -1;
  }
  in.close();
  return 0;
}

void constract_socket(sockaddr_in& address, char* port) {
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port =
      htons(std::stoi(port)); 
}
