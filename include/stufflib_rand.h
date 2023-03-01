#ifndef _SL_RAND_H_INCLUDED
#define _SL_RAND_H_INCLUDED
#include <math.h>
#include <stdlib.h>
#include <time.h>

void sl_rand_fill_double(const size_t n, double dst[n], double scale) {
  srand(time(0));
  for (size_t i = 0; i < n; ++i) {
    double r = rand() - RAND_MAX / 2;
    dst[i] = scale * (r / RAND_MAX);
  }
}

void sl_rand_set_zero_double(const size_t n,
                                   double dst[n],
                                   double probability) {
  probability = fmax(0, fmin(1, probability));
  srand(time(0));
  for (size_t i = 0; i < n; ++i) {
    if ((1.0 * rand() / RAND_MAX) < probability) {
      dst[i] = 0;
    }
  }
}

#endif  // _SL_RAND_H_INCLUDED
