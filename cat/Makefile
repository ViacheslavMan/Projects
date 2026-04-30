CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c11 -D_POSIX_C_SOURCE=200809L
SANITIZE = -fsanitize=address -fsanitize=undefined -fsanitize=unreachable
TARGET = s21_cat

SRCS = cat.c options.c
all: 
	$(CC) $(CFLAGS) $(SANITIZE) $(SRCS) -o $(TARGET)
clean: 
	rm -rf $(TARGET)
rebuild:	clean all