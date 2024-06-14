#ifndef SL_RAND_H_INCLUDED
#define SL_RAND_H_INCLUDED
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "stufflib_misc.h"

void sl_rand_fill_double(const size_t n, double dst[n], double scale) {
  for (size_t i = 0; i < n; ++i) {
    double r = rand() - RAND_MAX / 2;
    dst[i] = scale * (r / RAND_MAX);
  }
}

void sl_rand_set_zero_double(const size_t n,
                             double dst[n],
                             double probability) {
  probability = fmax(0, fmin(1, probability));
  for (size_t i = 0; i < n; ++i) {
    if ((1.0 * rand() / RAND_MAX) < probability) {
      dst[i] = 0;
    }
  }
}

int sl_rand_int(const int a, const int b) {
  // return random integer i such that a <= i < b
  if (a >= b) {
    return a;
  }
  return a + (rand() % (b - a));
}

void sl_rand_shuffle(void* data, const int count, const int size) {
  // https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
  // 2024-06-14
  if (size <= 0 || count < 2) {
    return;
  }
  unsigned char* v = data;
  for (int i = 0; i < count - 1; ++i) {
    const int j = sl_rand_int(i, count);
    sl_misc_swap(v + i * size, v + j * size, (size_t)size);
  }
}

#endif  // SL_RAND_H_INCLUDED
