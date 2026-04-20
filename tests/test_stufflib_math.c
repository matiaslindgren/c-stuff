#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/logging/logging.h>
#include <stufflib/macros/macros.h>
#include <stufflib/math/math.h>
#include <stufflib/memory/memory.h>
#include <stufflib/random/random.h>
#include <stufflib/testing/testing.h>

#define SL_ASSERT_FACTORIZATION_OK(x, factors)                     \
  do {                                                             \
    SL_ASSERT_TRUE((factors));                                     \
    SL_LOG_INFO("%zu factors:", (x));                              \
    for (size_t f_index = 0; (factors)[f_index] != 0; ++f_index) { \
      SL_LOG_INFO("  %zu: %zu", f_index, (factors)[f_index]);      \
    }                                                              \
  } while (false)

static const size_t SL_TEST_PRIMES[] = {
    2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,
    61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131, 137, 139,
    149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
    239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
    347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
    443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
};

SL_TEST(test_is_prime) {
  (void)ctx;
  for (size_t x = 0, i = 0; i < SL_ARRAY_LEN(SL_TEST_PRIMES); ++x) {
    if (x < SL_TEST_PRIMES[i]) {
      SL_ASSERT_FALSE(sl_math_is_prime(x));  // TODO abort on fail is too harsh
    } else if (x == SL_TEST_PRIMES[i]) {
      SL_ASSERT_TRUE(sl_math_is_prime(x));
      ++i;
    }
  }
  return true;
}

SL_TEST(test_factorize_primes) {
  for (size_t i = 0; i < SL_ARRAY_LEN(SL_TEST_PRIMES); ++i) {
    size_t* f = nullptr;
    SL_ASSERT_TRUE(sl_math_factorize(ctx, SL_TEST_PRIMES[i], &f));
    SL_ASSERT_FACTORIZATION_OK(SL_TEST_PRIMES[i], f);
    SL_ASSERT_TRUE(f[0] == SL_TEST_PRIMES[i]);
    SL_ASSERT_TRUE(f[1] == 0);
    sl_free(f);
  }
  return true;
}

SL_TEST(test_factorize_4) {
  const size_t n = 4;
  size_t* f      = nullptr;
  SL_ASSERT_TRUE(sl_math_factorize(ctx, n, &f));
  SL_ASSERT_FACTORIZATION_OK(n, f);
  SL_ASSERT_TRUE(f[0] == 2);
  SL_ASSERT_TRUE(f[1] == 2);
  SL_ASSERT_TRUE(f[2] == 0);
  sl_free(f);
  return true;
}

SL_TEST(test_factorize_25) {
  const size_t n = 25;
  size_t* f      = nullptr;
  SL_ASSERT_TRUE(sl_math_factorize(ctx, n, &f));
  SL_ASSERT_FACTORIZATION_OK(n, f);
  SL_ASSERT_TRUE(f[0] == 5);
  SL_ASSERT_TRUE(f[1] == 5);
  SL_ASSERT_TRUE(f[2] == 0);
  sl_free(f);
  return true;
}

SL_TEST(test_factorize_30) {
  const size_t n = 30;
  size_t* f      = nullptr;
  SL_ASSERT_TRUE(sl_math_factorize(ctx, n, &f));
  SL_ASSERT_FACTORIZATION_OK(n, f);
  SL_ASSERT_TRUE(f[0] == 2);
  SL_ASSERT_TRUE(f[1] == 3);
  SL_ASSERT_TRUE(f[2] == 5);
  SL_ASSERT_TRUE(f[3] == 0);
  sl_free(f);
  return true;
}

SL_TEST(test_factorize_864) {
  const size_t n = 864;
  size_t* f      = nullptr;
  SL_ASSERT_TRUE(sl_math_factorize(ctx, n, &f));
  SL_ASSERT_FACTORIZATION_OK(n, f);
  SL_ASSERT_TRUE(f[0] == 2);
  SL_ASSERT_TRUE(f[1] == 2);
  SL_ASSERT_TRUE(f[2] == 2);
  SL_ASSERT_TRUE(f[3] == 2);
  SL_ASSERT_TRUE(f[4] == 2);
  SL_ASSERT_TRUE(f[5] == 3);
  SL_ASSERT_TRUE(f[6] == 3);
  SL_ASSERT_TRUE(f[7] == 3);
  SL_ASSERT_TRUE(f[8] == 0);
  sl_free(f);
  return true;
}

SL_TEST(test_factorize_2022) {
  const size_t n = 2022;
  size_t* f      = nullptr;
  SL_ASSERT_TRUE(sl_math_factorize(ctx, n, &f));
  SL_ASSERT_FACTORIZATION_OK(n, f);
  SL_ASSERT_TRUE(f[0] == 2);
  SL_ASSERT_TRUE(f[1] == 3);
  SL_ASSERT_TRUE(f[2] == 337);
  SL_ASSERT_TRUE(f[3] == 0);
  sl_free(f);
  return true;
}

SL_TEST(test_factorize_202212) {
  const size_t n = 202212;
  size_t* f      = nullptr;
  SL_ASSERT_TRUE(sl_math_factorize(ctx, n, &f));
  SL_ASSERT_FACTORIZATION_OK(n, f);
  SL_ASSERT_TRUE(f[0] == 2);
  SL_ASSERT_TRUE(f[1] == 2);
  SL_ASSERT_TRUE(f[2] == 3);
  SL_ASSERT_TRUE(f[3] == 3);
  SL_ASSERT_TRUE(f[4] == 41);
  SL_ASSERT_TRUE(f[5] == 137);
  SL_ASSERT_TRUE(f[6] == 0);
  sl_free(f);
  return true;
}

SL_TEST(test_factorize_20221210) {
  const size_t n = 20221210;
  size_t* f      = nullptr;
  SL_ASSERT_TRUE(sl_math_factorize(ctx, n, &f));
  SL_ASSERT_FACTORIZATION_OK(n, f);
  SL_ASSERT_TRUE(f[0] == 2);
  SL_ASSERT_TRUE(f[1] == 5);
  SL_ASSERT_TRUE(f[2] == 101);
  SL_ASSERT_TRUE(f[3] == 20021);
  SL_ASSERT_TRUE(f[4] == 0);
  sl_free(f);
  return true;
}

SL_TEST(test_linalg) {
  (void)ctx;
  const double cmp_eps = 1e-16;

  const double v1[] = {1, 2, 3, 4};
  const double v2[] = {0, -2, 4, -6};
  const size_t n    = SL_ARRAY_LEN(v1);
  const double dot  = sl_math_linalg_dot(n, v1, v2);
  SL_ASSERT_TRUE(sl_math_double_almost(dot, -16, cmp_eps));

  double m[3][4] = {
      {1, 2, 3, 4},
      {5},
      {9, 10, 11, 12},
  };
  double res[3] = {0};
  sl_math_linalg_matmul(3, 4, m, v2, res);
  SL_ASSERT_TRUE(sl_math_double_almost(res[0], -16, cmp_eps));
  SL_ASSERT_TRUE(sl_math_double_almost(res[1], 0, cmp_eps));
  SL_ASSERT_TRUE(sl_math_double_almost(res[2], -48, cmp_eps));

  return true;
}

double pow2(double x) {
  return pow(x, 2);
}

SL_TEST(test_numerical_diff) {
  (void)ctx;
  const size_t n = 10000;
  double x[n];
  uint64_t prng = 0;
  sl_random_pcg32_init(&prng, 0);
  sl_random_fill_double(&prng, n, x, 10);
  sl_random_set_zero_double(&prng, n, x, 0.01);

  for (size_t i = 0; i < n; ++i) {
    double cos_x    = cos(x[i]);
    double sin_x    = sin(x[i]);
    double d_cos_x  = sl_math_diff(cos, x[i]);
    double d_sin_x  = sl_math_diff(sin, x[i]);
    double d_pow2_x = sl_math_diff(pow2, x[i]);
    SL_LOG_INFO(
        "%4zu %8.1g %8.3g %8.3g %9.3g %9.3g %9.3g %9.3f",
        i,
        x[i],
        cos_x,
        sin_x,
        2 * x[i],
        d_cos_x,
        d_sin_x,
        d_pow2_x
    );
    SL_ASSERT_TRUE(sl_math_double_almost(d_cos_x, -sin_x, 1e-3));
    SL_ASSERT_TRUE(sl_math_double_almost(d_sin_x, cos_x, 1e-3));
    SL_ASSERT_TRUE(sl_math_double_almost(d_pow2_x, 2 * x[i], 1e-3));
  }

  return true;
}

SL_TEST(test_double_almost) {
  (void)ctx;
  if (!sl_math_double_almost(1, 1.5, 1)) {
    return false;
  }
  if (sl_math_double_almost(1, 1.5, 0.1)) {
    return false;
  }
  return true;
}

SL_TEST(test_round_up_pow2) {
  (void)ctx;
  SL_ASSERT_TRUE(sl_math_next_power_of_two(0) == 1);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(1) == 2);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(2) == 4);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(3) == 4);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(4) == 8);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(5) == 8);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(6) == 8);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(7) == 8);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(8) == 16);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(0xfe) == 0x100);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(0xff) == 0x100);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(0xffff) == 0x10000);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(0xffffffff) == 0x100000000);
  // boundary: largest value whose next power of two still fits in size_t
  const size_t half = SIZE_MAX / 2;  // 2^63 - 1 on 64-bit
  SL_ASSERT_TRUE(sl_math_next_power_of_two(half - 1) == half + 1);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(half) == half + 1);
  // overflow: next power of two would exceed SIZE_MAX, returns 0
  SL_ASSERT_TRUE(sl_math_next_power_of_two(half + 1) == 0);
  SL_ASSERT_TRUE(sl_math_next_power_of_two(SIZE_MAX) == 0);
  return true;
}

SL_TEST(test_is_finite) {
  (void)ctx;
  SL_ASSERT_TRUE(sl_math_is_finite(1, (float[]){0}));
  SL_ASSERT_TRUE(sl_math_is_finite(1, (float[]){1}));
  SL_ASSERT_TRUE(sl_math_is_finite(1, (float[]){-1}));
  SL_ASSERT_TRUE(sl_math_is_finite(10, (float[10]){0}));
  SL_ASSERT_TRUE(!sl_math_is_finite(4, (float[]){0, 1, 2, (float)exp(1000)}));
  SL_ASSERT_TRUE(!sl_math_is_finite(1, (float[]){NAN}));
  SL_ASSERT_TRUE(!sl_math_is_finite(1, (float[]){INFINITY}));
  SL_ASSERT_TRUE(!sl_math_is_finite(3, (float[]){0, NAN, 0}));
  SL_ASSERT_TRUE(!sl_math_is_finite(3, (float[]){0, INFINITY, 0}));
  return true;
}

SL_TEST_MAIN()
