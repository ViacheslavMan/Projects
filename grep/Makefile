CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET = s21_grep

SRCS = grep.c
all: 
	$(CC) $(CFLAGS) $(SANITIZE) $(SRCS) -o $(TARGET)
s21_grep:
	$(CC) $(CFLAGS) $(SANITIZE) $(SRCS) -o $(TARGET)
clean: 
	rm -rf $(TARGET)
rebuild:	clean all