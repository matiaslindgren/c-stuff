#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib.h"

typedef unsigned long long ull;

const ull max_iterations = 2;
const double pi_approx = 3.14159265358979323846;

ull factorial(ull x) {
  ull f = 1;
  for (ull k = 1; k <= x; ++k) {
    f *= k;
  }
  return f;
}

ull power(ull x, ull k) {
  ull p = 1;
  for (ull i = 0; i < k; ++i) {
    p *= x;
  }
  return p;
}

double ramanujan(int num_digits) {
  double s = 0;
  double pi = 0;
  for (ull k = 0; k < max_iterations; ++k) {
    ull a = factorial(4 * k) * (1103 + 26390 * k);
    ull b = power(factorial(k), 4) * power(396, 4 * k);
    s += (double)a / (double)b;
    pi = 9801 / (2 * sqrt(2.0) * s);
    if (fabs(pi - pi_approx) < pow(10, -(num_digits + 1))) {
      break;
    }
  }
  return pi;
}

int main(void) {
  int num_digits = 16;
  double pi = ramanujan(num_digits);
  assert(stufflib_double_almost(pi, pi_approx, 1e-15));
  return EXIT_SUCCESS;
}
