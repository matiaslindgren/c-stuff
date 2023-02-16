#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_math.h"
#include "stufflib_rand.h"
#include "stufflib_test.h"

void _check_factorization(int verbose, size_t x, size_t* factors) {
  assert(factors);
  if (verbose) {
    printf("%zd factors:\n", x);
    for (size_t i = 0; factors[i] != 0; ++i) {
      printf("  %zu: %zu\n", i, factors[i]);
    }
  }
}

int test_factorize_primes(const int verbose) {
  const size_t primes[] = {2, 3, 5, 7, 11, 13, 17};
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(primes); ++i) {
    size_t* f = stufflib_math_factorize(primes[i]);
    _check_factorization(verbose, primes[i], f);
    assert(f[0] == primes[i]);
    assert(f[1] == 0);
    free(f);
  }
  return 1;
}

int test_factorize_4(const int verbose) {
  const size_t n = 4;
  size_t* f = stufflib_math_factorize(n);
  _check_factorization(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 2);
  assert(f[2] == 0);
  free(f);
  return 1;
}

int test_factorize_25(const int verbose) {
  const size_t n = 25;
  size_t* f = stufflib_math_factorize(n);
  _check_factorization(verbose, n, f);
  assert(f[0] == 5);
  assert(f[1] == 5);
  assert(f[2] == 0);
  free(f);
  return 1;
}

int test_factorize_30(const int verbose) {
  const size_t n = 30;
  size_t* f = stufflib_math_factorize(n);
  _check_factorization(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 3);
  assert(f[2] == 5);
  assert(f[3] == 0);
  free(f);
  return 1;
}

int test_factorize_864(const int verbose) {
  const size_t n = 864;
  size_t* f = stufflib_math_factorize(n);
  _check_factorization(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 2);
  assert(f[2] == 2);
  assert(f[3] == 2);
  assert(f[4] == 2);
  assert(f[5] == 3);
  assert(f[6] == 3);
  assert(f[7] == 3);
  assert(f[8] == 0);
  free(f);
  return 1;
}

int test_factorize_2022(const int verbose) {
  const size_t n = 2022;
  size_t* f = stufflib_math_factorize(n);
  _check_factorization(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 3);
  assert(f[2] == 337);
  assert(f[3] == 0);
  free(f);
  return 1;
}

int test_factorize_202212(const int verbose) {
  const size_t n = 202212;
  size_t* f = stufflib_math_factorize(n);
  _check_factorization(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 2);
  assert(f[2] == 3);
  assert(f[3] == 3);
  assert(f[4] == 41);
  assert(f[5] == 137);
  assert(f[6] == 0);
  free(f);
  return 1;
}

int test_factorize_20221210(const int verbose) {
  const size_t n = 20221210;
  size_t* f = stufflib_math_factorize(n);
  _check_factorization(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 5);
  assert(f[2] == 101);
  assert(f[3] == 20021);
  assert(f[4] == 0);
  free(f);
  return 1;
}

int test_linalg(const int verbose) {
  const double cmp_eps = 1e-16;

  const double v1[] = {1, 2, 3, 4};
  const double v2[] = {0, -2, 4, -6};
  const size_t n = STUFFLIB_ARRAY_LEN(v1);
  const double dot = stufflib_math_linalg_dot(n, v1, v2);
  assert(stufflib_math_double_almost(dot, -16, cmp_eps));

  double m[3][4] = {
      {1,  2,  3,  4},
      {5   },
      {9, 10, 11, 12 },
  };
  double res[3] = {0};
  stufflib_math_linalg_matmul(3, 4, m, v2, res);
  assert(stufflib_math_double_almost(res[0], -16, cmp_eps));
  assert(stufflib_math_double_almost(res[1], 0, cmp_eps));
  assert(stufflib_math_double_almost(res[2], -48, cmp_eps));

  return 1;
}

double pow2(double x) { return pow(x, 2); }

int test_numerical_diff(const int verbose) {
  const size_t n = 10000;
  double x[n];
  stufflib_rand_fill_double(n, x, 10);
  stufflib_rand_set_zero_double(n, x, 0.01);

  if (verbose) {
    printf("%4s %8s %8s %8s %9s %9s %9s %9s\n",
           "i",
           "x[i]",
           "cos(x)",
           "sin(x)",
           "2x",
           "cos'(x)",
           "sin'(x)",
           "pow2'(x)");
  }
  for (size_t i = 0; i < n; ++i) {
    double cos_x = cos(x[i]);
    double sin_x = sin(x[i]);
    double d_cos_x = stufflib_math_diff(cos, x[i]);
    double d_sin_x = stufflib_math_diff(sin, x[i]);
    double d_pow2_x = stufflib_math_diff(pow2, x[i]);
    if (verbose) {
      printf("%4zd %8.1g %8.3g %8.3g %9.3g %9.3g %9.3g %9.3f\n",
             i,
             x[i],
             cos_x,
             sin_x,
             2 * x[i],
             d_cos_x,
             d_sin_x,
             d_pow2_x);
    }
    assert(stufflib_math_double_almost(d_cos_x, -sin_x, 1e-3));
    assert(stufflib_math_double_almost(d_sin_x, cos_x, 1e-3));
    assert(stufflib_math_double_almost(d_pow2_x, 2 * x[i], 1e-3));
  }

  return 1;
}

int test_double_almost(const int verbose) {
  if (!stufflib_math_double_almost(1, 1.5, 1)) {
    return 0;
  }
  if (stufflib_math_double_almost(1, 1.5, 0.1)) {
    return 0;
  }
  return 1;
}

int test_round_up_pow2(const int verbose) {
  assert(stufflib_math_next_power_of_two(0) == 1);
  assert(stufflib_math_next_power_of_two(1) == 2);
  assert(stufflib_math_next_power_of_two(2) == 4);
  assert(stufflib_math_next_power_of_two(3) == 4);
  assert(stufflib_math_next_power_of_two(4) == 8);
  assert(stufflib_math_next_power_of_two(5) == 8);
  assert(stufflib_math_next_power_of_two(6) == 8);
  assert(stufflib_math_next_power_of_two(7) == 8);
  assert(stufflib_math_next_power_of_two(8) == 16);
  assert(stufflib_math_next_power_of_two(0xfe) == 0x100);
  assert(stufflib_math_next_power_of_two(0xff) == 0x100);
  assert(stufflib_math_next_power_of_two(0xffff) == 0x10000);
  assert(stufflib_math_next_power_of_two(0xffffffff) == 0x100000000);
  return 1;
}

STUFFLIB_TEST_MAIN(test_factorize_primes,
                   test_factorize_4,
                   test_factorize_25,
                   test_factorize_30,
                   test_factorize_864,
                   test_factorize_2022,
                   test_factorize_202212,
                   test_factorize_20221210,
                   test_double_almost,
                   test_numerical_diff,
                   test_linalg,
                   test_round_up_pow2);
