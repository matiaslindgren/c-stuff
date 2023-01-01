#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "stufflib_argv.h"
#include "stufflib_rand.h"
#include "stufflib_sort.h"

enum sort_type { quick, merge, num_sort_types };

typedef int(test_function)(const int);

int _test_compare_doubles(const int verbose) {
  (void)verbose;
  double* a = calloc(2, sizeof(double));
  double* b = calloc(2, sizeof(double));
  a[0] = -1e12;
  a[1] = 1;
  b[0] = 0;
  b[1] = 1e12;

  assert(stufflib_sort_compare_double((void*)(&a[0]), (void*)(&a[0])) == 0);
  assert(stufflib_sort_compare_double((void*)(&a[1]), (void*)(&a[1])) == 0);
  assert(stufflib_sort_compare_double((void*)(&b[0]), (void*)(&b[0])) == 0);
  assert(stufflib_sort_compare_double((void*)(&b[1]), (void*)(&b[1])) == 0);

  assert(stufflib_sort_compare_double((void*)(&a[0]), (void*)(&a[1])) == -1);
  assert(stufflib_sort_compare_double((void*)(&a[1]), (void*)(&a[0])) == 1);
  assert(stufflib_sort_compare_double((void*)(&b[0]), (void*)(&b[1])) == -1);
  assert(stufflib_sort_compare_double((void*)(&b[1]), (void*)(&b[0])) == 1);

  assert(stufflib_sort_compare_double((void*)(&a[0]), (void*)(&b[0])) == -1);
  assert(stufflib_sort_compare_double((void*)(&a[0]), (void*)(&b[1])) == -1);
  assert(stufflib_sort_compare_double((void*)(&a[1]), (void*)(&b[0])) == 1);
  assert(stufflib_sort_compare_double((void*)(&a[1]), (void*)(&b[1])) == -1);

  free(a);
  free(b);
  return 0;
}

int _test_swap_doubles(const int verbose) {
  (void)verbose;
  double* a = calloc(1, sizeof(double));
  double* b = calloc(1, sizeof(double));
  a[0] = -1;
  b[0] = 1;

  _stufflib_sort_swap((void*)(&a[0]), (void*)(&a[0]), sizeof(double));
  assert(a[0] == -1);
  _stufflib_sort_swap((void*)(&b[0]), (void*)(&b[0]), sizeof(double));
  assert(b[0] == 1);
  _stufflib_sort_swap((void*)(&a[0]), (void*)(&b[0]), sizeof(double));
  assert(a[0] == 1);
  assert(b[0] == -1);
  _stufflib_sort_swap((void*)(&a[0]), (void*)(&b[0]), sizeof(double));
  assert(a[0] == -1);
  assert(b[0] == 1);

  free(a);
  free(b);
  return 0;
}

int _test_sort_doubles(stufflib_sort_double* sort_doubles,
                       const char* sort_type,
                       const int verbose) {
  const size_t num_tests_per_size = 5;
  const size_t array_sizes[] = {1, 2, 10, 1000, 10000, 100000};

  for (size_t s = 0; s < sizeof(array_sizes) / sizeof(array_sizes[0]); ++s) {
    const size_t n = array_sizes[s];

    double* x = calloc(n, sizeof(double));
    if (!x) {
      fprintf(stderr, "failed allocating memory for test data of len %zu\n", n);
      return 1;
    }

    for (size_t test = 0; test < num_tests_per_size; ++test) {
      stufflib_rand_fill_double(n, x, 1e6);
      stufflib_rand_set_zero_double(n, x, 0.01);

      clock_t start_time = clock();
      if (!sort_doubles(n, x)) {
        fprintf(stderr, "sort failed\n");
        free(x);
        return 1;
      }
      clock_t end_time = clock();
      double sort_msec =
          1e3 * ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

      for (size_t i = 1; i < n; ++i) {
        if (x[i - 1] > x[i]) {
          fprintf(stderr, "array of %zu doubles is not sorted after sort\n", n);
          free(x);
          return 1;
        }
      }

      if (verbose) {
        printf("%7s %5zu %8zu %6.1f\n", sort_type, test + 1, n, sort_msec);
      }
    }

    free(x);
  }

  return 0;
}

int _test_quicksort_doubles(const int verbose) {
  return _test_sort_doubles(stufflib_sort_quick_double, "quick", verbose);
}

int _test_mergesort_doubles(const int verbose) {
  return _test_sort_doubles(stufflib_sort_merge_double, "merge", verbose);
}

int _test_compare_strings(const int verbose) {
  (void)verbose;
  const char* a[] = {"hello", "there"};
  const char* b[] = {"ok", " "};

  assert(stufflib_sort_compare_str((void*)(&a[0]), (void*)(&a[0])) == 0);
  assert(stufflib_sort_compare_str((void*)(&a[1]), (void*)(&a[1])) == 0);
  assert(stufflib_sort_compare_str((void*)(&b[0]), (void*)(&b[0])) == 0);
  assert(stufflib_sort_compare_str((void*)(&b[1]), (void*)(&b[1])) == 0);

  assert(stufflib_sort_compare_str((void*)(&a[0]), (void*)(&a[1])) == -1);
  assert(stufflib_sort_compare_str((void*)(&a[1]), (void*)(&a[0])) == 1);
  assert(stufflib_sort_compare_str((void*)(&b[0]), (void*)(&b[1])) == 1);
  assert(stufflib_sort_compare_str((void*)(&b[1]), (void*)(&b[0])) == -1);

  assert(stufflib_sort_compare_str((void*)(&a[0]), (void*)(&b[0])) == -1);
  assert(stufflib_sort_compare_str((void*)(&a[0]), (void*)(&b[1])) == 1);
  assert(stufflib_sort_compare_str((void*)(&a[1]), (void*)(&b[0])) == 1);
  assert(stufflib_sort_compare_str((void*)(&a[1]), (void*)(&b[1])) == 1);

  return 0;
}

int _test_swap_strings(const int verbose) {
  (void)verbose;
  const char* a[] = {"hello", "there"};
  const char* b[] = {"ok", " "};

  _stufflib_sort_swap((void*)(&a[0]), (void*)(&a[0]), sizeof(char*));
  assert(strcmp(a[0], "hello") == 0);
  _stufflib_sort_swap((void*)(&a[1]), (void*)(&a[1]), sizeof(char*));
  assert(strcmp(a[1], "there") == 0);
  _stufflib_sort_swap((void*)(&b[0]), (void*)(&b[0]), sizeof(char*));
  assert(strcmp(b[0], "ok") == 0);
  _stufflib_sort_swap((void*)(&b[1]), (void*)(&b[1]), sizeof(char*));
  assert(strcmp(b[1], " ") == 0);

  _stufflib_sort_swap((void*)(&a[0]), (void*)(&a[1]), sizeof(char*));
  assert(strcmp(a[0], "there") == 0);
  assert(strcmp(a[1], "hello") == 0);
  _stufflib_sort_swap((void*)(&a[1]), (void*)(&a[0]), sizeof(char*));
  assert(strcmp(a[0], "hello") == 0);
  assert(strcmp(a[1], "there") == 0);

  _stufflib_sort_swap((void*)(&b[0]), (void*)(&b[1]), sizeof(char*));
  assert(strcmp(b[0], " ") == 0);
  assert(strcmp(b[1], "ok") == 0);
  _stufflib_sort_swap((void*)(&b[1]), (void*)(&b[0]), sizeof(char*));
  assert(strcmp(b[0], "ok") == 0);
  assert(strcmp(b[1], " ") == 0);

  _stufflib_sort_swap((void*)(&a[0]), (void*)(&b[0]), sizeof(char*));
  assert(strcmp(a[0], "ok") == 0);
  assert(strcmp(b[0], "hello") == 0);
  _stufflib_sort_swap((void*)(&a[0]), (void*)(&b[1]), sizeof(char*));
  assert(strcmp(a[0], " ") == 0);
  assert(strcmp(b[1], "ok") == 0);

  return 0;
}

int _test_sort_strings(stufflib_sort_str* sort_strings,
                       const char* sort_type,
                       const int verbose) {
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
    return 1;
  }
  clock_t end_time = clock();
  double sort_msec = 1e3 * ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  for (size_t i = 1; i < n; ++i) {
    if (strcmp(s[i - 1], s[i]) > 0) {
      fprintf(stderr, "array of %zu strings is not sorted after sort\n", n);
      free(s);
      return 1;
    }
  }

  size_t test = 0;
  if (verbose) {
    printf("%7s %5zu %8zu %6.1f\n", sort_type, test + 1, n, sort_msec);
  }

  free(s);
  return 0;
}

int _test_quicksort_strings(const int verbose) {
  return _test_sort_strings(stufflib_sort_quick_str, "quick", verbose);
}

int _test_mergesort_strings(const int verbose) {
  return _test_sort_strings(stufflib_sort_merge_str, "merge", verbose);
}

int main(int argc, char* const argv[argc + 1]) {
  int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  test_function* tests[] = {
      _test_compare_doubles,
      _test_swap_doubles,

      _test_compare_strings,
      _test_swap_strings,

      _test_quicksort_doubles,
      _test_mergesort_doubles,

      _test_quicksort_strings,
      _test_mergesort_strings,
  };

  for (size_t t = 0; t < (sizeof(tests) / sizeof(tests[0])); ++t) {
    if (tests[t](verbose)) {
      fprintf(stderr, "test %zu failed\n", t + 1);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
