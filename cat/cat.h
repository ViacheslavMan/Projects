#pragma once
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "options.h"
typedef struct {
  int v;
  int b;
  int e;
  int E;
  int n;
  int s;
  int t;
  int T;
} Options;
static struct option long_options[] = {
    {"number-nonblank", no_argument, NULL, 'b'},
    {"number", no_argument, NULL, 'n'},
    {"squeeze-blank", no_argument, NULL, 's'},
    {0, 0, 0, 0}};

int parser(int argc, char* argv[], Options* options);
int read_file(int argc, char* argv[], Options* options);
