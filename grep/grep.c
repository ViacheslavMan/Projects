#include "grep.h"

int main(int argc, char* argv[]) {
  int exit_code = 0;
  if (argc > 1) {
    Options options = {0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (parser(argc, argv, &options) == 1) {
      exit_code = 1;
    };
  } else {
    printf("No pattern or file");
    exit_code = 1;
  }
  return exit_code;
}

int compile_regex(regex_t* reg_exp, const char* pattern, int flag) {
  if (regcomp(reg_exp, pattern, flag)) {
    printf("Unable to compile regular expression");
    return 1;
  }
  return 0;
}

int is_dir(const char* filename) {
  struct stat file_stat = {0};
  if (stat(filename, &file_stat) != -1) {
    return S_ISDIR(file_stat.st_mode);
  }
  return -1;
}
void delete_memory(Options* options) { free(options->pattern); }

int enable_f(const char* pattern_file, Options* options) {
  FILE* f = NULL;
  f = fopen(pattern_file, "r");
  if (f == NULL) {
    if (options->s == 0) {
      fprintf(stderr, "grep: %s: No such file or directory\n", pattern_file);
    }
    return 1;
  }
  ssize_t read;
  size_t len = 0;
  char* line = NULL;
  while ((read = getline(&line, &len, f)) != -1) {
    if (read > 0 && line[read - 1] == '\n') {
      line[read - 1] = '\0';
    }
    char* pattern_copy = strdup(line);
    process_pattern_file(pattern_copy, &(options->pattern));
    free(pattern_copy);
  }

  free(line);
  fclose(f);

  return 0;
}
void process_pattern_file(const char* pattern, char** old_pattern) {
  if (*old_pattern != NULL) {
    char* tmp_pattern =
        realloc(*old_pattern, strlen(*old_pattern) + 1 + strlen(pattern) + 1);
    if (tmp_pattern == NULL) {
      return;
    }
    *old_pattern = tmp_pattern;
    strcat(*old_pattern, "|");
    strcat(*old_pattern, pattern);
  } else {
    char* tmp_pattern = realloc(NULL, strlen(pattern) + 1);
    if (tmp_pattern == NULL) {
      return;
    }
    *old_pattern = tmp_pattern;
    strcpy(*old_pattern, pattern);
  }
}

int read_file(int argc, char* argv[], Options* options) {
  if (options->pattern == NULL) {
    return 1;
  }
  int exit_code = 0;
  regex_t reg_exp;
  int regex_flags = REG_EXTENDED;
  int error_check = 1;
  int regex_compiled = 1;
  if (options->i == 1) {
    regex_flags |= REG_ICASE;
  }
  if (compile_regex(&reg_exp, options->pattern, regex_flags) == 1) {
    regex_compiled = 0;
    free(options->pattern);
    return 1;
  }

  for (int i = optind; i < argc; ++i) {
    FILE* f = NULL;
    int is_dir_v = is_dir(argv[i]);
    if (is_dir_v != 0 && is_dir_v != -1) {
      if (options->s == 0) {
        fflush(stdout);
        fprintf(stderr, "grep: %s: Is a directory\n", argv[i]);
      }
      error_check = 0;
    }
    if (error_check != 0) {
      f = fopen(argv[i], "r");
      if (f == NULL) {
        if (options->s == 0) {
          fflush(stdout);
          fprintf(stderr, "grep: %s: No such file or directory\n", argv[i]);
        }
        error_check = 0;
      }
    }
    if (error_check != 0) {
      int file_number = argc - optind;
      int print_file_name = 1;
      if (file_number == 1 || options->h == 1) {
        print_file_name = 0;
      }
      char* line = NULL;
      size_t len_str = 0;
      int check;
      int count_line = 0;
      int org_line_number = 0;
      int new_line = 1;

      while ((check = getline(&line, &len_str, f)) != -1) {
        ++org_line_number;
        if (options->c == 0 && options->l == 0 && options->o == 0) {
          int result = process_regex(&reg_exp, line);
          if (result == 1 && options->v == 0) {
            if (print_file_name) {
              printf("%s:", argv[i]);
            }
            (options->n == 1) ? printf("%d:%s", org_line_number, line)
                              : printf("%s", line);
            new_line = (line[check - 1] == '\n');
          } else if (result == 0 && options->v == 1) {
            if (print_file_name) {
              printf("%s:", argv[i]);
            }
            (options->n == 1) ? printf("%d:%s", org_line_number, line)
                              : printf("%s", line);
            new_line = (line[check - 1] == '\n');
          } else if (result == -1) {
            exit_code = 1;
          }
        } else if (options->c == 1 && options->l == 0) {
          int result = process_regex(&reg_exp, line);
          if (result == 1 && options->v == 0) {
            ++count_line;
          } else if (result == 0 && options->v == 1) {
            ++count_line;
          }
        } else if (options->c == 1 && options->l == 1) {
          int result = process_regex(&reg_exp, line);
          if ((result == 1 && options->v == 0) ||
              (result == 0 && options->v == 1)) {
            printf("%s\n", argv[i]);
            break;
          }
        } else if (options->l == 1) {
          int result = process_regex(&reg_exp, line);
          if ((result == 1 && options->v == 0) ||
              (result == 0 && options->v == 1)) {
            printf("%s\n", argv[i]);
            break;
          }

        } else if (options->o == 1 && options->v == 0) {
          proccess_option_o(options, &reg_exp, line, org_line_number, argv[i],
                            print_file_name);
        }
      }
      if (options->c == 1 && options->l == 0) {
        if (print_file_name) {
          printf("%s:", argv[i]);
        }
        printf("%d\n", count_line);
      }
      if (new_line == 0 && options->c == 0 && options->l == 0) {
        printf("\n");
      }
      free(line);
      fclose(f);
    }
    error_check = 1;
  }
  delete_memory(options);
  if (regex_compiled == 1) {
    regfree(&reg_exp);
  }
  return exit_code;
}

int process_regex(regex_t* reg_exp, const char* string) {
  int result = regexec(reg_exp, string, 0, NULL, 0);
  int exit_code;
  if (!result) {
    exit_code = 1;
  }  // found
  else if (result == REG_NOMATCH) {
    exit_code = 0;
  } else {
    exit_code = -1;
  }
  return exit_code;
}

void proccess_option_o(const Options* options, regex_t* reg_exp,
                       const char* string, int org_line_number,
                       const char* filename, int print_file_name) {
  regmatch_t match[1];
  const char* cur = string;
  int result;
  while ((result = regexec(reg_exp, cur, 1, match, 0)) == 0) {
    if (print_file_name) {
      printf("%s:", filename);
    }
    if (options->n == 1) {
      printf("%d:", org_line_number);
    }
    if (match[0].rm_so != -1) {
      printf("%.*s\n", (int)(match[0].rm_eo - match[0].rm_so),
             cur + match[0].rm_so);
    }
    cur += match[0].rm_eo;
  }
}
char* add_pattern(char* pattern, char* argv[]) {
  const char* src = (optarg != NULL) ? optarg : argv[optind];
  if (src == NULL) return pattern;
  if (pattern != NULL) {
    char* tmp_pattern = realloc(pattern, strlen(pattern) + 1 + strlen(src) + 1);
    if (tmp_pattern == NULL) {
      return NULL;
    }
    pattern = tmp_pattern;
    strcat(pattern, "|");
    strcat(pattern, src);
  } else {
    char* tmp_pattern = realloc(pattern, strlen(src) + 1);
    if (tmp_pattern == NULL) {
      return NULL;
    }
    pattern = tmp_pattern;
    strcpy(pattern, src);
  }
  return pattern;
}

int activating_no_flag(Options* options, char* argv[]) {
  if (options->e == 0) {
    if (options->f == 0) {
      char* pattern = add_pattern(options->pattern, argv);
      if (pattern == NULL) {
        return 1;
      }
      options->pattern = pattern;
      ++optind;
    }
  }
  return 0;
}
int parser(int argc, char* argv[], Options* options) {
  int exit_code = 0;
  int opt;
  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (opt) {
      case 'o':
        options->o = 1;
        break;
      case 'e':
        options->e = 1;
        char* pattern = add_pattern(options->pattern, argv);
        if (pattern == NULL) {
          free(options->pattern);
          return 1;
        }
        options->pattern = pattern;
        break;
      case 'i':
        options->i = 1;
        break;
      case 'f':
        options->f = 1;
        if (enable_f(optarg, options) == 1) {
          free(options->pattern);
          return 1;
        }
        break;
      case 'v':
        options->v = 1;
        break;
      case 's':
        options->s = 1;
        break;
      case 'c':
        options->c = 1;
        break;
      case 'l':
        options->l = 1;
        break;
      case 'n':
        options->n = 1;
        break;
      case 'h':
        options->h = 1;
        break;
      case '?':
        printf("Unknown option");
        free(options->pattern);
        return 1;
    }
  }
  if (options->e == 0) {
    if (options->f == 0) {
      char* pattern = add_pattern(options->pattern, argv);
      if (pattern == NULL) {
        free(options->pattern);
        return 1;
      }
      options->pattern = pattern;
      ++optind;
    }
  }

  if (read_file(argc, argv, options) != 0) {
    exit_code = 1;
  }
  return exit_code;
}
