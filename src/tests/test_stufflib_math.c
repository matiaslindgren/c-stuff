#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_rand.h"

#define SL_ASSERT_FACTORIZATION_OK(verbose, x, factors)              \
  do {                                                               \
    assert((factors));                                               \
    if ((verbose)) {                                                 \
      printf("%zd factors:\n", (x));                                 \
      for (size_t f_index = 0; (factors)[f_index] != 0; ++f_index) { \
        printf("  %zu: %zu\n", f_index, (factors)[f_index]);         \
      }                                                              \
    }                                                                \
  } while (false)

bool test_factorize_primes(const bool verbose) {
  const size_t primes[] = {2, 3, 5, 7, 11, 13, 17};
  for (size_t i = 0; i < SL_ARRAY_LEN(primes); ++i) {
    size_t* f = sl_math_factorize(primes[i]);
    SL_ASSERT_FACTORIZATION_OK(verbose, primes[i], f);
    assert(f[0] == primes[i]);
    assert(f[1] == 0);
    sl_free(f);
  }
  return true;
}

bool test_factorize_4(const bool verbose) {
  const size_t n = 4;
  size_t* f = sl_math_factorize(n);
  SL_ASSERT_FACTORIZATION_OK(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 2);
  assert(f[2] == 0);
  sl_free(f);
  return true;
}

bool test_factorize_25(const bool verbose) {
  const size_t n = 25;
  size_t* f = sl_math_factorize(n);
  SL_ASSERT_FACTORIZATION_OK(verbose, n, f);
  assert(f[0] == 5);
  assert(f[1] == 5);
  assert(f[2] == 0);
  sl_free(f);
  return true;
}

bool test_factorize_30(const bool verbose) {
  const size_t n = 30;
  size_t* f = sl_math_factorize(n);
  SL_ASSERT_FACTORIZATION_OK(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 3);
  assert(f[2] == 5);
  assert(f[3] == 0);
  sl_free(f);
  return true;
}

bool test_factorize_864(const bool verbose) {
  const size_t n = 864;
  size_t* f = sl_math_factorize(n);
  SL_ASSERT_FACTORIZATION_OK(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 2);
  assert(f[2] == 2);
  assert(f[3] == 2);
  assert(f[4] == 2);
  assert(f[5] == 3);
  assert(f[6] == 3);
  assert(f[7] == 3);
  assert(f[8] == 0);
  sl_free(f);
  return true;
}

bool test_factorize_2022(const bool verbose) {
  const size_t n = 2022;
  size_t* f = sl_math_factorize(n);
  SL_ASSERT_FACTORIZATION_OK(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 3);
  assert(f[2] == 337);
  assert(f[3] == 0);
  sl_free(f);
  return true;
}

bool test_factorize_202212(const bool verbose) {
  const size_t n = 202212;
  size_t* f = sl_math_factorize(n);
  SL_ASSERT_FACTORIZATION_OK(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 2);
  assert(f[2] == 3);
  assert(f[3] == 3);
  assert(f[4] == 41);
  assert(f[5] == 137);
  assert(f[6] == 0);
  sl_free(f);
  return true;
}

bool test_factorize_20221210(const bool verbose) {
  const size_t n = 20221210;
  size_t* f = sl_math_factorize(n);
  SL_ASSERT_FACTORIZATION_OK(verbose, n, f);
  assert(f[0] == 2);
  assert(f[1] == 5);
  assert(f[2] == 101);
  assert(f[3] == 20021);
  assert(f[4] == 0);
  sl_free(f);
  return true;
}

bool test_linalg(const bool) {
  const double cmp_eps = 1e-16;

  const double v1[] = {1, 2, 3, 4};
  const double v2[] = {0, -2, 4, -6};
  const size_t n = SL_ARRAY_LEN(v1);
  const double dot = sl_math_linalg_dot(n, v1, v2);
  assert(sl_math_double_almost(dot, -16, cmp_eps));

  double m[3][4] = {
      {1, 2, 3, 4},
      {5},
      {9, 10, 11, 12},
  };
  double res[3] = {0};
  sl_math_linalg_matmul(3, 4, m, v2, res);
  assert(sl_math_double_almost(res[0], -16, cmp_eps));
  assert(sl_math_double_almost(res[1], 0, cmp_eps));
  assert(sl_math_double_almost(res[2], -48, cmp_eps));

  return true;
}

double pow2(double x) { return pow(x, 2); }

bool test_numerical_diff(const bool verbose) {
  const size_t n = 10000;
  double x[n];
  sl_rand_fill_double(n, x, 10);
  sl_rand_set_zero_double(n, x, 0.01);

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
    double d_cos_x = sl_math_diff(cos, x[i]);
    double d_sin_x = sl_math_diff(sin, x[i]);
    double d_pow2_x = sl_math_diff(pow2, x[i]);
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
    assert(sl_math_double_almost(d_cos_x, -sin_x, 1e-3));
    assert(sl_math_double_almost(d_sin_x, cos_x, 1e-3));
    assert(sl_math_double_almost(d_pow2_x, 2 * x[i], 1e-3));
  }

  return true;
}

bool test_double_almost(const bool) {
  if (!sl_math_double_almost(1, 1.5, 1)) {
    return false;
  }
  if (sl_math_double_almost(1, 1.5, 0.1)) {
    return false;
  }
  return true;
}

bool test_round_up_pow2(const bool) {
  assert(sl_math_next_power_of_two(0) == 1);
  assert(sl_math_next_power_of_two(1) == 2);
  assert(sl_math_next_power_of_two(2) == 4);
  assert(sl_math_next_power_of_two(3) == 4);
  assert(sl_math_next_power_of_two(4) == 8);
  assert(sl_math_next_power_of_two(5) == 8);
  assert(sl_math_next_power_of_two(6) == 8);
  assert(sl_math_next_power_of_two(7) == 8);
  assert(sl_math_next_power_of_two(8) == 16);
  assert(sl_math_next_power_of_two(0xfe) == 0x100);
  assert(sl_math_next_power_of_two(0xff) == 0x100);
  assert(sl_math_next_power_of_two(0xffff) == 0x10000);
  assert(sl_math_next_power_of_two(0xffffffff) == 0x100000000);
  return true;
}

bool test_is_finite(const bool) {
  assert(sl_math_is_finite(1, (float[]){0}));
  assert(sl_math_is_finite(1, (float[]){1}));
  assert(sl_math_is_finite(1, (float[]){-1}));
  assert(sl_math_is_finite(10, (float[10]){0}));
  assert(!sl_math_is_finite(4, (float[]){0, 1, 2, (float)exp(1000)}));
  assert(!sl_math_is_finite(1, (float[]){NAN}));
  assert(!sl_math_is_finite(1, (float[]){INFINITY}));
  assert(!sl_math_is_finite(3, (float[]){0, NAN, 0}));
  assert(!sl_math_is_finite(3, (float[]){0, INFINITY, 0}));
  return true;
}

SL_TEST_MAIN(test_factorize_primes,
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
             test_round_up_pow2,
             test_is_finite)
