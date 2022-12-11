#ifndef _STUFFLIB_RAND_H_INCLUDED
#define _STUFFLIB_RAND_H_INCLUDED
#include <math.h>
#include <stdlib.h>
#include <time.h>

void stufflib_rand_fill(const size_t n, double dst[n], double scale) {
  srand(time(NULL));
  for (size_t i = 0; i < n; ++i) {
    double r = rand() - RAND_MAX / 2;
    dst[i] = scale * (r / RAND_MAX);
  }
}

void stufflib_rand_set_zero(const size_t n, double dst[n], double probability) {
  probability = fmax(0, fmin(1, probability));
  srand(time(NULL));
  for (size_t i = 0; i < n; ++i) {
    if ((1.0 * rand() / RAND_MAX) < probability) {
      dst[i] = 0;
    }
  }
}

#endif  // _STUFFLIB_RAND_H_INCLUDED
