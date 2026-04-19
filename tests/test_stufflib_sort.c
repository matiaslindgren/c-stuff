#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/logging/logging.h>
#include <stufflib/macros/macros.h>
#include <stufflib/random/random.h>
#include <stufflib/sort/sort.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_compare_doubles) {
  (void)ctx;
  double* a = calloc(2, sizeof(double));
  double* b = calloc(2, sizeof(double));
  a[0]      = -1e12;
  a[1]      = 1;
  b[0]      = 0;
  b[1]      = 1e12;

  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[0]), (void*)(&a[0])) == 0);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[1]), (void*)(&a[1])) == 0);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&b[0]), (void*)(&b[0])) == 0);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&b[1]), (void*)(&b[1])) == 0);

  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[0]), (void*)(&a[1])) == -1);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[1]), (void*)(&a[0])) == 1);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&b[0]), (void*)(&b[1])) == -1);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&b[1]), (void*)(&b[0])) == 1);

  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[0]), (void*)(&b[0])) == -1);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[0]), (void*)(&b[1])) == -1);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[1]), (void*)(&b[0])) == 1);
  SL_ASSERT_TRUE(sl_sort_compare_double((void*)(&a[1]), (void*)(&b[1])) == -1);

  free(a);
  free(b);
  return true;
}

static bool test_sort_doubles(
    struct sl_context ctx[static 1],
    sl_sort_double* sort_doubles,
    const size_t max_len
) {
  const size_t num_tests_per_size = 5;
  const size_t array_sizes[]      = {1, 2, 10, 1000, 10000, 100000, 500000};

  for (size_t s = 0; s < sizeof(array_sizes) / sizeof(array_sizes[0]); ++s) {
    const size_t n = array_sizes[s];
    if (n > max_len) {
      break;
    }

    double* x = calloc(n, sizeof(double));
    if (!x) {
      fprintf(stderr, "failed allocating memory for test data of len %zu\n", n);
      return false;
    }

    for (size_t test = 0; test < num_tests_per_size; ++test) {
      uint64_t prng = 0;
      sl_random_pcg32_init(&prng, 0);
      sl_random_fill_double(&prng, n, x, 1e6);
      sl_random_set_zero_double(&prng, n, x, 0.01);

      if (!sort_doubles(ctx, n, x)) {
        fprintf(stderr, "sort failed\n");
        free(x);
        return false;
      }

      for (size_t i = 1; i < n; ++i) {
        if (x[i - 1] > x[i]) {
          fprintf(stderr, "array of %zu doubles is not sorted after sort\n", n);
          free(x);
          return false;
        }
      }

      SL_LOG_INFO("%5zu %8zu", test + 1, n);
    }

    free(x);
  }

  return true;
}

double*
test_stdlib_qsort_double(struct sl_context ctx[static 1], const size_t count, double src[count]) {
  (void)ctx;
  qsort((void*)src, count, sizeof(double), sl_sort_compare_double);
  return src;
}

SL_TEST(test_insertsort_doubles) {
  SL_LOG_INFO("test insertsort doubles");
  return test_sort_doubles(ctx, sl_sort_insertsort_double, 10000);
}

SL_TEST(test_quicksort_doubles) {
  SL_LOG_INFO("test quicksort doubles");
  return test_sort_doubles(ctx, sl_sort_quicksort_double, 1000000);
}

SL_TEST(test_mergesort_doubles) {
  SL_LOG_INFO("test mergesort doubles");
  return test_sort_doubles(ctx, sl_sort_mergesort_double, 1000000);
}

SL_TEST(test_qsort_doubles) {
  SL_LOG_INFO("test stdlib qsort doubles");
  return test_sort_doubles(ctx, test_stdlib_qsort_double, 1000000);
}

SL_TEST(test_compare_strings) {
  (void)ctx;
  const char* a[] = {"hello", "there"};
  const char* b[] = {"ok", " "};

  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[0]), (void*)(&a[0])) == 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[1]), (void*)(&a[1])) == 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&b[0]), (void*)(&b[0])) == 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&b[1]), (void*)(&b[1])) == 0);

  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[0]), (void*)(&a[1])) < 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[1]), (void*)(&a[0])) > 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&b[0]), (void*)(&b[1])) > 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&b[1]), (void*)(&b[0])) < 0);

  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[0]), (void*)(&b[0])) < 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[0]), (void*)(&b[1])) > 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[1]), (void*)(&b[0])) > 0);
  SL_ASSERT_TRUE(sl_sort_compare_str((void*)(&a[1]), (void*)(&b[1])) > 0);

  return true;
}

static bool test_sort_strings(struct sl_context ctx[static 1], sl_sort_str* sort_strings) {
  const size_t n = 5;

  char** s = calloc(n, sizeof(char*));
  s[0]     = "hello";
  s[1]     = "";
  s[2]     = " ";
  s[3]     = "123";
  s[4]     = "there";

  if (!sort_strings(ctx, n, s)) {
    fprintf(stderr, "sort failed\n");
    free(s);
    return false;
  }

  for (size_t i = 1; i < n; ++i) {
    if (strcmp(s[i - 1], s[i]) > 0) {
      fprintf(stderr, "array of %zu strings is not sorted after sort\n", n);
      free(s);
      return false;
    }
  }

  SL_LOG_INFO("%5d %8zu", 1, n);

  free(s);
  return true;
}

char**
test_stdlib_qsort_str(struct sl_context ctx[static 1], const size_t count, char* src[count]) {
  (void)ctx;
  qsort((void*)src, count, sizeof(char*), sl_sort_compare_str);
  return src;
}

SL_TEST(test_insertsort_strings) {
  SL_LOG_INFO("test insertsort strings");
  return test_sort_strings(ctx, sl_sort_insertsort_str);
}

SL_TEST(test_quicksort_strings) {
  SL_LOG_INFO("test quicksort strings");
  return test_sort_strings(ctx, sl_sort_quicksort_str);
}

SL_TEST(test_mergesort_strings) {
  SL_LOG_INFO("test mergesort strings");
  return test_sort_strings(ctx, sl_sort_mergesort_str);
}

SL_TEST(test_qsort_strings) {
  SL_LOG_INFO("test stdlib qsort strings");
  return test_sort_strings(ctx, test_stdlib_qsort_str);
}

struct sl_test_named_vec3 {
  long x;
  long y;
  long z;
  const char* name;
};

typedef struct sl_test_named_vec3*
sl_test_sort_named_vec3(const size_t, struct sl_test_named_vec3*);

int sl_test_compare_named_vec3(const void* a, const void* b) {
  const struct sl_test_named_vec3 lhs = ((const struct sl_test_named_vec3*)a)[0];
  const struct sl_test_named_vec3 rhs = ((const struct sl_test_named_vec3*)b)[0];
  int res                             = 0;
  if (!res) res = (lhs.x > rhs.x) - (lhs.x < rhs.x);
  if (!res) res = (lhs.y > rhs.y) - (lhs.y < rhs.y);
  if (!res) res = (lhs.z > rhs.z) - (lhs.z < rhs.z);
  if (!res) res = strcmp(lhs.name, rhs.name);
  return res;
}

static bool test_sort_named_vec3(
    struct sl_context ctx[static 1],
    sl_test_sort_named_vec3* sl_test_sort_named_vec3
) {
  (void)ctx;
  struct sl_test_named_vec3 items[] = {
      {-1, 0, 0,   "f" },
      {-1, 0, 1,   "e" },
      {0,  0, 0,   "d" },
      {1,  0, 1e9, "c" },
      {1,  1, 0,   "b" },
      {0,  0, 0,   "a" },
      {0,  0, 0,   " a"},
  };
  struct sl_test_named_vec3 sorted_items[] = {
      items[0],
      items[1],
      items[6],
      items[5],
      items[2],
      items[3],
      items[4],
  };

  const size_t n = sizeof(items) / sizeof(items[0]);

  if (!sl_test_sort_named_vec3(n, items)) {
    fprintf(stderr, "sort failed\n");
    return false;
  }

  for (size_t i = 0; i < n; ++i) {
    SL_ASSERT_TRUE(items[i].x == sorted_items[i].x);
    SL_ASSERT_TRUE(items[i].y == sorted_items[i].y);
    SL_ASSERT_TRUE(items[i].z == sorted_items[i].z);
    SL_ASSERT_TRUE(strcmp(items[i].name, sorted_items[i].name) == 0);
  }

  SL_LOG_INFO("%5d %8zu", 1, n);

  return true;
}

struct sl_test_named_vec3*
sl_test_insertsort_named_vec3s(const size_t count, struct sl_test_named_vec3 src[count]) {
  struct sl_context tmp_ctx = {0};
  return sl_sort_insertsort(
      &tmp_ctx,
      (void*)src,
      count,
      sizeof(struct sl_test_named_vec3),
      sl_test_compare_named_vec3
  );
}

struct sl_test_named_vec3*
sl_test_quicksort_named_vec3s(const size_t count, struct sl_test_named_vec3 src[count]) {
  struct sl_context tmp_ctx = {0};
  return sl_sort_quicksort(
      &tmp_ctx,
      (void*)src,
      count,
      sizeof(struct sl_test_named_vec3),
      sl_test_compare_named_vec3
  );
}

struct sl_test_named_vec3*
sl_test_mergesort_named_vec3s(const size_t count, struct sl_test_named_vec3 src[count]) {
  struct sl_context tmp_ctx = {0};
  return sl_sort_mergesort(
      &tmp_ctx,
      (void*)src,
      count,
      sizeof(struct sl_test_named_vec3),
      sl_test_compare_named_vec3
  );
}

struct sl_test_named_vec3*
sl_test_stdlib_qsort_named_vec3s(const size_t count, struct sl_test_named_vec3 src[count]) {
  qsort((void*)src, count, sizeof(struct sl_test_named_vec3), sl_test_compare_named_vec3);
  return src;
}

SL_TEST(test_insertsort_custom_obj) {
  SL_LOG_INFO("test insertsort custom_obj");
  return test_sort_named_vec3(ctx, sl_test_insertsort_named_vec3s);
}

SL_TEST(test_quicksort_custom_obj) {
  SL_LOG_INFO("test quicksort custom_obj");
  return test_sort_named_vec3(ctx, sl_test_quicksort_named_vec3s);
}

SL_TEST(test_mergesort_custom_obj) {
  SL_LOG_INFO("test mergesort custom objects");
  return test_sort_named_vec3(ctx, sl_test_mergesort_named_vec3s);
}

SL_TEST(test_qsort_custom_obj) {
  SL_LOG_INFO("test stdlib qsort custom objects");
  return test_sort_named_vec3(ctx, sl_test_stdlib_qsort_named_vec3s);
}

SL_TEST_MAIN()
