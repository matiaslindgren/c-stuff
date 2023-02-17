#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_rand.h"
#include "stufflib_test.h"

int test_rand_fill(const int verbose) {
  const size_t n = 1000;
  double x[n];
  stufflib_rand_fill_double(n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return 0;
    }
  }
  stufflib_rand_fill_double(n, x, 10);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 10) {
      return 0;
    }
  }
  return 1;
}

int test_rand_set_zero(const int verbose) {
  const size_t n = 1000;
  double x[n];
  for (size_t i = 0; i < n; ++i) {
    x[i] = i + 1;
  }
  stufflib_rand_set_zero_double(n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) < 1) {
      return 0;
    }
  }
  stufflib_rand_set_zero_double(n, x, 1);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return 0;
    }
  }
  return 1;
}

STUFFLIB_TEST_MAIN(test_rand_fill, test_rand_set_zero);