#ifndef _STUFFLIB_H_INCLUDED
#define _STUFFLIB_H_INCLUDED
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
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

#ifndef UINT32_WIDTH
#define UINT32_WIDTH (32)
#endif
#ifndef UINT32_BYTES
#define UINT32_BYTES (UINT32_WIDTH / 8)
#endif

uint32_t stufflib_int_parse_big_endian(const unsigned char buf[static UINT32_BYTES]) {
  uint32_t x = 0;
  for (size_t i = 0; i < UINT32_BYTES; ++i) {
    const size_t offset = 8 * (UINT32_BYTES - (i + 1));
    x |= (uint32_t)(buf[i]) << offset;
  }
  return x;
}

#endif  // _STUFFLIB_H_INCLUDED
