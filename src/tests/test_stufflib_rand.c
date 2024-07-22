#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib/args/args.h"
#include "stufflib/macros/macros.h"
#include "stufflib/rand/rand.h"

static bool test_rand_fill(const bool) {
  const size_t n = 1000;
  double x[n];
  sl_rand_fill_double(n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return false;
    }
  }
  sl_rand_fill_double(n, x, 10);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 10) {
      return false;
    }
  }
  return true;
}

static bool test_rand_set_zero(const bool) {
  const size_t n = 1000;
  double x[n];
  for (size_t i = 0; i < n; ++i) {
    x[i] = (double)i + 1;
  }
  sl_rand_set_zero_double(n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) < 1) {
      return false;
    }
  }
  sl_rand_set_zero_double(n, x, 1);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return false;
    }
  }
  return true;
}

static bool test_randint(const bool) {
  assert(sl_rand_int(0, 0) == 0);
  assert(sl_rand_int(1, 0) == 1);
  assert(sl_rand_int(0, 1) == 0);
  for (size_t a = 1; a < 100; ++a) {
    for (size_t b = a + 1; b < 100; ++b) {
      int freq[100] = {0};
      for (size_t i = 0; i < 2000; ++i) {
        size_t x = sl_rand_int(a, b);
        assert(a <= x && x < b);
        freq[x] += 1;
      }
      for (size_t x = a; x < b; ++x) {
        assert(0 < freq[x] && freq[x] <= 2000);
      }
    }
  }
  return true;
}

static bool test_rand_shuffle(const bool verbose) {
  {
    unsigned char v[] = {0};
    sl_rand_shuffle(v, 1, 1);
    assert(v[0] == 0);
  }
  {
    unsigned char v[16] = {0};
    int freq_unshuffled[SL_ARRAY_LEN(v)] = {0};
    const size_t n = SL_ARRAY_LEN(v);
    for (unsigned char i = 0; i < n; ++i) {
      v[i] = i;
    }

    for (int iter = 0; iter < 100; ++iter) {
      unsigned char buf[16] = {0};
      memcpy(buf, v, n);
      sl_rand_shuffle(buf, 1, n);
      if (verbose) {
        printf("shuffle %02d: ", iter);
        for (size_t i = 0; i < n; ++i) {
          printf("%d ", buf[i]);
        }
        printf("\n");
      }
      for (size_t i = 0; i < n; ++i) {
        freq_unshuffled[i] += buf[i] == v[i];
      }
    }

    if (verbose) {
      printf("seen counts\n");
      for (size_t i = 0; i < n; ++i) {
        printf("  %d: %d\n", v[i], freq_unshuffled[i]);
      }
      printf("\n");
    }
    for (size_t i = 0; i < n; ++i) {
      assert(freq_unshuffled[i] < 50);
    }
  }
  return true;
}

static bool test_rand_shuffle_together(const bool verbose) {
  for (int iter = 0; iter < 100; ++iter) {
    size_t v1[16] = {0};
    char v2[16] = {0};
    const size_t n = SL_ARRAY_LEN(v1);
    for (size_t i = 0; i < n; ++i) {
      v1[i] = i;
      v2[i] = (char)i;
    }
    sl_rand_shuffle_together(v1, v2, sizeof(size_t), 1, n);
    if (verbose) {
      printf("shuffle %02d: ", iter);
      for (size_t i = 0; i < n; ++i) {
        printf("%zu,%d ", v1[i], v2[i]);
      }
      printf("\n");
    }
    for (size_t i = 0; i < n; ++i) {
      assert(v1[i] == (size_t)v2[i]);
    }
  }
  return true;
}

SL_TEST_MAIN(test_rand_fill,
             test_rand_set_zero,
             test_randint,
             test_rand_shuffle,
             test_rand_shuffle_together)
