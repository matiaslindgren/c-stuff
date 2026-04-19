#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/macros/macros.h>
#include <stufflib/random/random.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_random_fill) {
  (void)ctx;
  uint64_t prng = 0;
  sl_random_pcg32_init(&prng, 0);
  const size_t n = 1000;
  double x[n];
  sl_random_fill_double(&prng, n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return false;
    }
  }
  sl_random_fill_double(&prng, n, x, 10);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 10) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_random_set_zero) {
  (void)ctx;
  uint64_t prng = 0;
  sl_random_pcg32_init(&prng, 0);
  const size_t n = 1000;
  double x[n];
  for (size_t i = 0; i < n; ++i) {
    x[i] = (double)i + 1;
  }
  sl_random_set_zero_double(&prng, n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) < 1) {
      return false;
    }
  }
  sl_random_set_zero_double(&prng, n, x, 1);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return false;
    }
  }
  return true;
}

SL_TEST(test_randomint) {
  (void)ctx;
  uint64_t prng = 0;
  sl_random_pcg32_init(&prng, 0);
  SL_ASSERT_TRUE(sl_random_int(&prng, 0, 0) == 0);
  SL_ASSERT_TRUE(sl_random_int(&prng, 1, 0) == 1);
  SL_ASSERT_TRUE(sl_random_int(&prng, 0, 1) == 0);
  for (size_t a = 1; a < 100; ++a) {
    for (size_t b = a + 1; b < 100; ++b) {
      int freq[100] = {0};
      for (size_t i = 0; i < 2000; ++i) {
        size_t x = sl_random_int(&prng, a, b);
        SL_ASSERT_TRUE(a <= x && x < b);
        freq[x] += 1;
      }
      for (size_t x = a; x < b; ++x) {
        SL_ASSERT_TRUE(0 < freq[x] && freq[x] <= 2000);
      }
    }
  }
  return true;
}

SL_TEST(test_random_shuffle) {
  (void)ctx;
  uint64_t prng = 0;
  sl_random_pcg32_init(&prng, 0);
  {
    unsigned char v[] = {0};
    sl_random_shuffle(&prng, v, 1, 1);
    SL_ASSERT_TRUE(v[0] == 0);
  }
  {
    unsigned char v[16]                  = {0};
    int freq_unshuffled[SL_ARRAY_LEN(v)] = {0};
    const size_t n                       = SL_ARRAY_LEN(v);
    for (unsigned char i = 0; i < n; ++i) {
      v[i] = i;
    }

    for (int iter = 0; iter < 100; ++iter) {
      unsigned char buf[16] = {0};
      memcpy(buf, v, n);
      sl_random_shuffle(&prng, buf, 1, n);
      for (size_t i = 0; i < n; ++i) {
        freq_unshuffled[i] += buf[i] == v[i];
      }
    }

    for (size_t i = 0; i < n; ++i) {
      SL_ASSERT_TRUE(freq_unshuffled[i] < 50);
    }
  }
  return true;
}

SL_TEST(test_random_shuffle_together) {
  (void)ctx;
  uint64_t prng = 0;
  sl_random_pcg32_init(&prng, 0);
  for (int iter = 0; iter < 100; ++iter) {
    size_t v1[16]  = {0};
    char v2[16]    = {0};
    const size_t n = SL_ARRAY_LEN(v1);
    for (size_t i = 0; i < n; ++i) {
      v1[i] = i;
      v2[i] = (char)i;
    }
    sl_random_shuffle_together(&prng, v1, v2, sizeof(size_t), 1, n);
    for (size_t i = 0; i < n; ++i) {
      SL_ASSERT_TRUE(v1[i] == (size_t)v2[i]);
    }
  }
  return true;
}

SL_TEST_MAIN()
