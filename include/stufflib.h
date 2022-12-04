#ifndef _STUFFLIB_H_INCLUDED
#define _STUFFLIB_H_INCLUDED
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int stufflib_parse_argv_flag(int argc, char* const argv[argc + 1], const char* flag) {
  for (size_t i = 1; i < argc; ++i) {
    if (strcmp(argv[i], flag) == 0) {
      return 1;
    }
  }
  return 0;
}

void stufflib_fill_random(const size_t n, double dst[n], double scale) {
  srand(time(NULL));
  for (size_t i = 0; i < n; ++i) {
    double r = rand() - RAND_MAX / 2;
    dst[i] = scale * (r / RAND_MAX);
  }
}

void stufflib_set_zero_random(const size_t n, double dst[n], double probability) {
  probability = fmax(0, fmin(1, probability));
  srand(time(NULL));
  for (size_t i = 0; i < n; ++i) {
    if ((1.0 * rand() / RAND_MAX) < probability) {
      dst[i] = 0;
    }
  }
}

int stufflib_double_almost(const double lhs, const double rhs, const double tolerance) {
  return fabs(lhs - rhs) < tolerance;
}

#endif  // _STUFFLIB_H_INCLUDED
