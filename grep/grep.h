#pragma once

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  int e;
  char* pattern;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
} Options;
int is_dir(const char* filename);
int process_regex(regex_t* reg_exp, const char* string);
int compile_regex(regex_t* reg_exp, const char* pattern, int flag);
void delete_memory(Options* options);
char* add_pattern(char* pattern, char* argv[]);
void process_pattern_file(const char* pattern, char** old_pattern);
void proccess_option_o(const Options* options, regex_t* reg_exp,
                       const char* string, int org_line_number,
                       const char* filename, int print_file_name);
int parser(int argc, char* argv[], Options* options);
int enable_f(const char* pattern_file, Options* options);
int read_file(int argc, char* argv[], Options* options);
int activating_no_flag(Options* options, char* argv[]);
