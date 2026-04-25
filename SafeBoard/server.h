#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <fstream>
#include <iostream>
#include <vector>

constexpr int MAX_PATTERN_FOUND = 50;
constexpr const char *fifo_path = "/tmp/myfifo";


struct DetectedPatterns {
  size_t quantity = 0;
  std::vector<std::string> detectted_patterns{};
};

struct Stats {
  uint32_t file_checked = 0;
  uint32_t pattern_found[MAX_PATTERN_FOUND];
};
DetectedPatterns find_pattern(std::string &file,
                              const std::vector<std::string> &patterns);
void read_patterns(std::vector<std::string> &file_patterns, char *path);
void handler(int);
void send_to_stat_utils(const Stats *stats_pointer,
                        const std::vector<std::string> &patterns);
void sending_response_to_client(std::string file,
                                std::vector<std::string> &patterns, sem_t *sem,
                                Stats *stats_pointer, int clientSocketFD);
int handle_client(int serverSocketFD, int clientSocketFD, sem_t *sem,
                  Stats *stats_pointer, std::vector<std::string> &patterns);

int creating_socket(int port);
Stats *creating_shared_memory();
void exiting(int fifo_id, Stats *stats_pointer, sem_t *sem, int serverSocketFD);
int starting_fifo_process(int serverSocketFD, Stats *stats_pointer,
                          const std::vector<std::string> &patterns);
int accepting_client(int serverSocketFD);
void signals_configure();
