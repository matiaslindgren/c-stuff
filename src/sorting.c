#include <assert.h>
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

void _merge_sort(size_t n, double src[n], double dst[n], size_t begin, size_t end) {
  assert(begin <= end);
  if (end - begin <= 1) {
    return;
  }
  // WARNING mid is wrong if (begin + end) overflows
  size_t mid = (begin + end) / 2;
  _merge_sort(n, dst, src, begin, mid);
  _merge_sort(n, dst, src, mid, end);
  _merge(n, src, dst, begin, mid, end);
}

void _copy(size_t n, double src[n], double dst[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = src[i];
  }
}

double* merge_sort(size_t n, double src[n]) {
  double* dst = malloc(n * sizeof(double));
  if (!dst) {
    return NULL;
  }
  _copy(n, src, dst);
  _merge_sort(n, src, dst, 0, n);
  _copy(n, dst, src);
  free(dst);
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

int main() {
  size_t num_tests_per_size = 8;
  size_t array_sizes[6] = {
      1, 2, 10, 1000, 10000, 1000000,
  };
  printf("%4s %10s %6s\n", "test", "array_size", "sort");
  for (size_t i = 0; i < sizeof(array_sizes) / sizeof(size_t); ++i) {
    size_t n = array_sizes[i];
    double* x = malloc(n * sizeof(double));
    if (!x) {
      fputs("failed allocating memory for test data", stderr);
      goto error;
    }
    for (size_t test = 0; test < num_tests_per_size; ++test) {
      printf("%4zu %10zu %6s\n", test, n, "merge");
      _fill_rand(n, x);
      if (!merge_sort(n, x)) {
        fputs("failed allocating working memory", stderr);
        free(x);
        goto error;
      }
      if (!is_sorted(n, x)) {
        fputs("result is not sorted", stderr);
        free(x);
        goto error;
      }
    }
    free(x);
  }
  return 0;
error:
  return 1;
}
