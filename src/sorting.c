#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
  double* tmp = malloc(n * sizeof(double));
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

size_t _qsort_partition(size_t n, double src[n], size_t lo, size_t hi) {
  while (lo < hi) {
    if (src[lo] < src[hi]) {
      ++lo;
    } else {
      _swap(&src[hi - 1], &src[hi]);
      _swap(&src[lo], &src[hi]);
      --hi;
    }
  }
  return hi;
}

void _quick_sort(size_t n, double src[n], size_t lo, size_t hi) {
  if (lo > hi || lo >= n || hi >= n) {
    return;
  }
  size_t pivot = _qsort_partition(n, src, lo, hi);
  _quick_sort(n, src, lo, pivot - 1);
  _quick_sort(n, src, pivot + 1, hi);
}

double* quick_sort(size_t n, double src[n]) {
  double* tmp = malloc(n * sizeof(double));
  if (!tmp) {
    return NULL;
  }
  _copy(n, src, tmp);
  _quick_sort(n, tmp, 0, n - 1);
  _copy(n, tmp, src);
  free(tmp);
  return src;
}

int is_sorted(size_t n, double x[n]) {
  for (size_t i = 1; i < n; ++i) {
    if (x[i - 1] > x[i]) {
      return 0;
    }
  }
  return 1;
}

void _fill_rand(size_t n, double x[n]) {
  for (size_t i = 0; i < n; ++i) {
    x[i] = 1.0 * (rand() - RAND_MAX / 2);
  }
}

typedef double* (*sort_function_t)(size_t, double[]);

int main() {
  size_t num_tests_per_size = 10;
  size_t array_sizes[] = {
      1, 2, 10, 1000, 10000, 100000, 1000000,
  };
  const char* sort_types[] = {
      "merge",
      "quick",
  };
  sort_function_t sort_funcs[] = {
      merge_sort,
      quick_sort,
  };
  printf("%7s %5s %8s\n", "func_t", "test", "n");
  for (size_t f = 0; f < sizeof(sort_types) / sizeof(const char*); ++f) {
    for (size_t s = 0; s < sizeof(array_sizes) / sizeof(size_t); ++s) {
      size_t n = array_sizes[s];
      double* x = malloc(n * sizeof(double));
      if (!x) {
        fprintf(stderr, "failed allocating memory for test data\n");
        goto error;
      }
      for (size_t test = 0; test < num_tests_per_size; ++test) {
        printf("%7s %5zu %8zu\n", sort_types[f], test + 1, n);
        _fill_rand(n, x);
        if (!sort_funcs[f](n, x)) {
          fprintf(stderr, "failed allocating working memory\n");
          free(x);
          goto error;
        }
        if (!is_sorted(n, x)) {
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
