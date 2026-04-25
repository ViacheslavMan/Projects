#include "stats.h"

int main() {
  int fd = open("/tmp/myfifo", O_RDONLY);
  if (fd == -1) {
    return -1;
  }
  uint32_t statistic_lenght;
  read(fd, &statistic_lenght, sizeof(uint32_t));
  std::string statistic;
  statistic.resize(statistic_lenght);
  read(fd, (void *)statistic.data(), statistic_lenght);
  std::cout << statistic << std::endl;
  close(fd);
}