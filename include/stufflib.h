#ifndef _STUFFLIB_H_INCLUDED
#define _STUFFLIB_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int stufflib_parse_argv_flag(int argc, char* const argv[argc + 1], const char* flag) {
  for (size_t i = 1; i < argc; ++i) {
    if (strcmp(argv[i], flag) == 0) {
      return 1;
    }
  }
  return 0;
}

void stufflib_fill_rand(const size_t n, double dst[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = 1.0 * (rand() - RAND_MAX / 2);
  }
}

#endif  // _STUFFLIB_H_INCLUDED
