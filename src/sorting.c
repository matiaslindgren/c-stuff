#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib.h"

void _merge(size_t n, double src[n], double dst[n], size_t begin, size_t mid, size_t end) {
  size_t lhs = begin;
  size_t rhs = mid;
  size_t out = begin;
  while (lhs < mid && rhs < end) {
    if (src[lhs] < src[rhs]) {
      dst[out++] = src[lhs++];
    } else {
      dst[out++] = src[rhs++];
    }
  }
  while (lhs < mid) {
    dst[out++] = src[lhs++];
  }
  while (rhs < end) {
    dst[out++] = src[rhs++];
  }
}

void _merge_sort(size_t n, double src[n], double tmp[n], size_t begin, size_t end) {
  assert(begin <= end);
  if (end - begin <= 1) {
    return;
  }
  // stupid way of ensuring (begin + end) does not wrap around
  assert(begin < SIZE_MAX / 2);
  assert(end < SIZE_MAX / 2);
  size_t mid = (begin + end) / 2;
  _merge_sort(n, tmp, src, begin, mid);
  _merge_sort(n, tmp, src, mid, end);
  _merge(n, src, tmp, begin, mid, end);
}

void _copy(size_t n, double src[n], double dst[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = src[i];
  }
}

double* merge_sort(size_t n, double src[n]) {
  assert(n);
  double* tmp = malloc(n * sizeof(*src));
  if (!tmp) {
    return NULL;
  }
  _copy(n, src, tmp);
  _merge_sort(n, src, tmp, 0, n);
  _copy(n, tmp, src);
  free(tmp);
  return src;
}

void _swap(double a[static 1], double b[static 1]) {
  double tmp = *a;
  *a = *b;
  *b = tmp;
}

size_t _hoare_partition(size_t n, double src[n], size_t lo, size_t hi) {
  double pivot = src[(lo + hi) / 2];
  size_t lhs = lo - 1;
  size_t rhs = hi + 1;
  while (1) {
    do {
      ++lhs;
    } while (src[lhs] < pivot);
    do {
      --rhs;
    } while (src[rhs] > pivot);
    if (lhs >= rhs) {
      return rhs;
    }
    _swap(&src[lhs], &src[rhs]);
  }
}

void _quick_sort(size_t n, double src[n], size_t lo, size_t hi) {
  if (lo >= hi || lo >= n || hi >= n) {
    return;
  }
  size_t pivot_idx = _hoare_partition(n, src, lo, hi);
  _quick_sort(n, src, lo, pivot_idx);
  _quick_sort(n, src, pivot_idx + 1, hi);
}

double* quick_sort(size_t n, double src[n]) {
  assert(n);
  double* tmp = malloc(n * sizeof(*src));
  if (!tmp) {
    return NULL;
  }
  _copy(n, src, tmp);
  _quick_sort(n, tmp, 0, n - 1);
  _copy(n, tmp, src);
  free(tmp);
  return src;
}

int _is_sorted(size_t n, double x[n]) {
  for (size_t i = 1; i < n; ++i) {
    if (x[i - 1] > x[i]) {
      return 0;
    }
  }
  return 1;
}

typedef double* (*sort_function_t)(size_t, double[]);

enum sort_type { quick, merge, num_sort_types };

int main(int argc, char* const argv[argc + 1]) {
  int verbose = stufflib_parse_argv_flag(argc, argv, "-v");

  const size_t num_tests_per_size = 4;
  const size_t array_sizes[] = {1, 2, 10, 1000, 10000, 100000};
  const char* const sort_types[num_sort_types] = {
      [quick] = "quick",
      [merge] = "merge",
  };
  const sort_function_t sort_funcs[num_sort_types] = {
      [quick] = quick_sort,
      [merge] = merge_sort,
  };

  if (verbose) {
    printf("%7s %5s %8s\n", "func_t", "test", "n");
  }
  for (size_t f = 0; f < sizeof(sort_types) / sizeof(*sort_types); ++f) {
    for (size_t s = 0; s < sizeof(array_sizes) / sizeof(*array_sizes); ++s) {
      const size_t n = array_sizes[s];

      double* x = malloc(n * sizeof(double));
      if (!x) {
        fprintf(stderr, "failed allocating memory for test data\n");
        goto error;
      }

      for (size_t test = 0; test < num_tests_per_size; ++test) {
        if (verbose) {
          printf("%7s %5zu %8zu\n", sort_types[f], test + 1, n);
        }

        stufflib_fill_rand(n, x, 1e6);

        if (!sort_funcs[f](n, x)) {
          fprintf(stderr, "failed allocating working memory\n");
          free(x);
          goto error;
        }

        if (!_is_sorted(n, x)) {
          fprintf(stderr, "result is not sorted\n");
          free(x);
          goto error;
        }
      }

      free(x);
    }
  }

  return 0;

error:
  return 1;
}
