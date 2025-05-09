#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "stufflib/args/args.h"
#include "stufflib/macros/macros.h"
#include "stufflib/rand/rand.h"
#include "stufflib/sort/sort.h"

static bool test_compare_doubles(const bool verbose) {
  (void)verbose;
  double* a = calloc(2, sizeof(double));
  double* b = calloc(2, sizeof(double));
  a[0] = -1e12;
  a[1] = 1;
  b[0] = 0;
  b[1] = 1e12;

  assert(sl_sort_compare_double((void*)(&a[0]), (void*)(&a[0])) == 0);
  assert(sl_sort_compare_double((void*)(&a[1]), (void*)(&a[1])) == 0);
  assert(sl_sort_compare_double((void*)(&b[0]), (void*)(&b[0])) == 0);
  assert(sl_sort_compare_double((void*)(&b[1]), (void*)(&b[1])) == 0);

  assert(sl_sort_compare_double((void*)(&a[0]), (void*)(&a[1])) == -1);
  assert(sl_sort_compare_double((void*)(&a[1]), (void*)(&a[0])) == 1);
  assert(sl_sort_compare_double((void*)(&b[0]), (void*)(&b[1])) == -1);
  assert(sl_sort_compare_double((void*)(&b[1]), (void*)(&b[0])) == 1);

  assert(sl_sort_compare_double((void*)(&a[0]), (void*)(&b[0])) == -1);
  assert(sl_sort_compare_double((void*)(&a[0]), (void*)(&b[1])) == -1);
  assert(sl_sort_compare_double((void*)(&a[1]), (void*)(&b[0])) == 1);
  assert(sl_sort_compare_double((void*)(&a[1]), (void*)(&b[1])) == -1);

  free(a);
  free(b);
  return true;
}

static bool test_sort_doubles(sl_sort_double* sort_doubles,
                              const bool verbose,
                              const size_t max_len) {
  const size_t num_tests_per_size = 5;
  const size_t array_sizes[] = {1, 2, 10, 1000, 10000, 100000, 500000};

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
      sl_rand_fill_double(n, x, 1e6);
      sl_rand_set_zero_double(n, x, 0.01);

      clock_t start_time = clock();
      if (!sort_doubles(n, x)) {
        fprintf(stderr, "sort failed\n");
        free(x);
        return false;
      }
      clock_t end_time = clock();
      double sort_msec =
          1e3 * ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

      for (size_t i = 1; i < n; ++i) {
        if (x[i - 1] > x[i]) {
          fprintf(stderr, "array of %zu doubles is not sorted after sort\n", n);
          free(x);
          return false;
        }
      }

      if (verbose) {
        printf("%5zu %8zu %6.1f\n", test + 1, n, sort_msec);
      }
    }

    free(x);
  }

  return true;
}

double* test_stdlib_qsort_double(const size_t count, double src[count]) {
  qsort((void*)src, count, sizeof(double), sl_sort_compare_double);
  return src;
}

static bool test_insertsort_doubles(const bool verbose) {
  if (verbose) {
    printf("test insertsort doubles\n");
  }
  return test_sort_doubles(sl_sort_insertsort_double, verbose, 10000);
}

static bool test_quicksort_doubles(const bool verbose) {
  if (verbose) {
    printf("test quicksort doubles\n");
  }
  return test_sort_doubles(sl_sort_quicksort_double, verbose, 1000000);
}

static bool test_mergesort_doubles(const bool verbose) {
  if (verbose) {
    printf("test mergesort doubles\n");
  }
  return test_sort_doubles(sl_sort_mergesort_double, verbose, 1000000);
}

static bool test_qsort_doubles(const bool verbose) {
  if (verbose) {
    printf("test stdlib qsort doubles\n");
  }
  return test_sort_doubles(test_stdlib_qsort_double, verbose, 1000000);
}

static bool test_compare_strings(const bool verbose) {
  (void)verbose;
  const char* a[] = {"hello", "there"};
  const char* b[] = {"ok", " "};

  assert(sl_sort_compare_str((void*)(&a[0]), (void*)(&a[0])) == 0);
  assert(sl_sort_compare_str((void*)(&a[1]), (void*)(&a[1])) == 0);
  assert(sl_sort_compare_str((void*)(&b[0]), (void*)(&b[0])) == 0);
  assert(sl_sort_compare_str((void*)(&b[1]), (void*)(&b[1])) == 0);

  assert(sl_sort_compare_str((void*)(&a[0]), (void*)(&a[1])) == -1);
  assert(sl_sort_compare_str((void*)(&a[1]), (void*)(&a[0])) == 1);
  assert(sl_sort_compare_str((void*)(&b[0]), (void*)(&b[1])) == 1);
  assert(sl_sort_compare_str((void*)(&b[1]), (void*)(&b[0])) == -1);

  assert(sl_sort_compare_str((void*)(&a[0]), (void*)(&b[0])) == -1);
  assert(sl_sort_compare_str((void*)(&a[0]), (void*)(&b[1])) == 1);
  assert(sl_sort_compare_str((void*)(&a[1]), (void*)(&b[0])) == 1);
  assert(sl_sort_compare_str((void*)(&a[1]), (void*)(&b[1])) == 1);

  return true;
}

static bool test_sort_strings(sl_sort_str* sort_strings, const bool verbose) {
  const size_t n = 5;

  char** s = calloc(n, sizeof(char*));
  s[0] = "hello";
  s[1] = "";
  s[2] = " ";
  s[3] = "123";
  s[4] = "there";

  clock_t start_time = clock();
  if (!sort_strings(n, s)) {
    fprintf(stderr, "sort failed\n");
    free(s);
    return false;
  }
  clock_t end_time = clock();
  double sort_msec = 1e3 * ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  for (size_t i = 1; i < n; ++i) {
    if (strcmp(s[i - 1], s[i]) > 0) {
      fprintf(stderr, "array of %zu strings is not sorted after sort\n", n);
      free(s);
      return false;
    }
  }

  size_t test = 0;
  if (verbose) {
    printf("%5zu %8zu %6.1f\n", test + 1, n, sort_msec);
  }

  free(s);
  return true;
}

char** test_stdlib_qsort_str(const size_t count, char* src[count]) {
  qsort((void*)src, count, sizeof(char*), sl_sort_compare_str);
  return src;
}

static bool test_insertsort_strings(const bool verbose) {
  if (verbose) {
    printf("test insertsort strings\n");
  }
  return test_sort_strings(sl_sort_insertsort_str, verbose);
}

static bool test_quicksort_strings(const bool verbose) {
  if (verbose) {
    printf("test quicksort strings\n");
  }
  return test_sort_strings(sl_sort_quicksort_str, verbose);
}

static bool test_mergesort_strings(const bool verbose) {
  if (verbose) {
    printf("test mergesort strings\n");
  }
  return test_sort_strings(sl_sort_mergesort_str, verbose);
}

static bool test_qsort_strings(const bool verbose) {
  if (verbose) {
    printf("test stdlib qsort strings\n");
  }
  return test_sort_strings(test_stdlib_qsort_str, verbose);
}

struct sl_test_named_vec3 {
  long x;
  long y;
  long z;
  const char* name;
};

typedef struct sl_test_named_vec3* sl_test_sort_named_vec3(
    const size_t,
    struct sl_test_named_vec3*);

int sl_test_compare_named_vec3(const void* a, const void* b) {
  const struct sl_test_named_vec3 lhs =
      ((const struct sl_test_named_vec3*)a)[0];
  const struct sl_test_named_vec3 rhs =
      ((const struct sl_test_named_vec3*)b)[0];
  int res = 0;
  if (!res) res = (lhs.x > rhs.x) - (lhs.x < rhs.x);
  if (!res) res = (lhs.y > rhs.y) - (lhs.y < rhs.y);
  if (!res) res = (lhs.z > rhs.z) - (lhs.z < rhs.z);
  if (!res) res = strcmp(lhs.name, rhs.name);
  return res;
}

static bool test_sort_named_vec3(
    sl_test_sort_named_vec3* sl_test_sort_named_vec3,
    const bool verbose) {
  struct sl_test_named_vec3 items[] = {
      {-1, 0,   0,  "f"},
      {-1, 0,   1,  "e"},
      { 0, 0,   0,  "d"},
      { 1, 0, 1e9,  "c"},
      { 1, 1,   0,  "b"},
      { 0, 0,   0,  "a"},
      { 0, 0,   0, " a"},
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

  clock_t start_time = clock();
  if (!sl_test_sort_named_vec3(n, items)) {
    fprintf(stderr, "sort failed\n");
    return false;
  }
  clock_t end_time = clock();
  double sort_msec = 1e3 * ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  for (size_t i = 0; i < n; ++i) {
    assert(items[i].x == sorted_items[i].x);
    assert(items[i].y == sorted_items[i].y);
    assert(items[i].z == sorted_items[i].z);
    assert(strcmp(items[i].name, sorted_items[i].name) == 0);
  }

  size_t test = 0;
  if (verbose) {
    printf("%5zu %8zu %6.1f\n", test + 1, n, sort_msec);
  }

  return true;
}

struct sl_test_named_vec3* sl_test_insertsort_named_vec3s(
    const size_t count,
    struct sl_test_named_vec3 src[count]) {
  return sl_sort_insertsort((void*)src,
                            count,
                            sizeof(struct sl_test_named_vec3),
                            sl_test_compare_named_vec3);
}

struct sl_test_named_vec3* sl_test_quicksort_named_vec3s(
    const size_t count,
    struct sl_test_named_vec3 src[count]) {
  return sl_sort_quicksort((void*)src,
                           count,
                           sizeof(struct sl_test_named_vec3),
                           sl_test_compare_named_vec3);
}

struct sl_test_named_vec3* sl_test_mergesort_named_vec3s(
    const size_t count,
    struct sl_test_named_vec3 src[count]) {
  return sl_sort_mergesort((void*)src,
                           count,
                           sizeof(struct sl_test_named_vec3),
                           sl_test_compare_named_vec3);
}

struct sl_test_named_vec3* sl_test_stdlib_qsort_named_vec3s(
    const size_t count,
    struct sl_test_named_vec3 src[count]) {
  qsort((void*)src,
        count,
        sizeof(struct sl_test_named_vec3),
        sl_test_compare_named_vec3);
  return src;
}

static bool test_insertsort_custom_obj(const bool verbose) {
  if (verbose) {
    printf("test insertsort custom_obj\n");
  }
  return test_sort_named_vec3(sl_test_insertsort_named_vec3s, verbose);
}

static bool test_quicksort_custom_obj(const bool verbose) {
  if (verbose) {
    printf("test quicksort custom_obj\n");
  }
  return test_sort_named_vec3(sl_test_quicksort_named_vec3s, verbose);
}

static bool test_mergesort_custom_obj(const bool verbose) {
  if (verbose) {
    printf("test mergesort custom objects\n");
  }
  return test_sort_named_vec3(sl_test_mergesort_named_vec3s, verbose);
}

static bool test_qsort_custom_obj(const bool verbose) {
  if (verbose) {
    printf("test stdlib qsort custom objects\n");
  }
  return test_sort_named_vec3(sl_test_stdlib_qsort_named_vec3s, verbose);
}

SL_TEST_MAIN(test_compare_doubles,
             test_compare_strings,
             test_insertsort_doubles,
             test_quicksort_doubles,
             test_mergesort_doubles,
             test_qsort_doubles,
             test_insertsort_strings,
             test_quicksort_strings,
             test_mergesort_strings,
             test_qsort_strings,
             test_insertsort_custom_obj,
             test_quicksort_custom_obj,
             test_mergesort_custom_obj,
             test_qsort_custom_obj)
