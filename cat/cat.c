#include "cat.h"

int main(int argc, char* argv[]) {
  Options options = {0};
  if (argc > 1) {
    if (parser(argc, argv, &options) != 0) {
      return 1;
    }
  } else {
    printf("No file");
    return 1;
  }
}

int parser(int argc, char* argv[], Options* options) {
  int opt;
  while ((opt = getopt_long(argc, argv, "vbeEnstT", long_options, NULL)) !=
         -1) {
    switch (opt) {
      case 'v':
        options->v = 1;
        break;
      case 'b':
        options->b = 1;
        break;
      case 'e':
        options->e = 1;
        break;
      case 'E':
        options->E = 1;
        break;
      case 'n':
        options->n = 1;
        break;
      case 's':
        options->s = 1;
        break;
      case 't':
        options->t = 1;
        break;
      case 'T':
        options->T = 1;
        break;
      case '?':
        printf("Unknnown option");
        return 1;
    }
  }
  if (read_file(argc, argv, options) != 0) {
    return 1;
  };
  return 0;
}

int read_file(int argc, char* argv[], Options* options) {
  FILE* f = NULL;

  for (int i = optind; i < argc; ++i) {
    f = fopen(argv[i], "r");
    if (f == NULL) {
      printf("Unable to open file");
      return 1;
    }
    if (options->v == 1) {
      option_v(f);
    }
    if (options->n == 1) {
      option_n(f);
    }

    if (options->b == 1) {
      option_b(f);
    }
    if (options->e) {
      option_e(f);
    }
    if (options->E) {
      option_E(f);
    }
    if (options->t == 1) {
      option_t(f);
    }
    if (options->T == 1) {
      option_T(f);
    }
    if (options->s == 1) {
      option_s(f);
    } else {
      no_options(f);
    }

    fclose(f);
  }
  return 0;
}
