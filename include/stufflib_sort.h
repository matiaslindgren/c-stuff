#ifndef _STUFFLIB_SORT_H_INCLUDED
#define _STUFFLIB_SORT_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "stufflib_misc.h"

typedef double* (*stufflib_sort_function)(size_t, double[]);

void _stufflib_sort_merge_partitions(const size_t n, double src[static n], double dst[static n],
                                     size_t begin, size_t mid, size_t end) {
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

void _stufflib_sort_merge(const size_t n, double src[static n], double tmp[static n], size_t begin,
                          size_t end) {
  assert(begin <= end);
  if (end - begin <= 1) {
    return;
  }
  // stupid way of ensuring (begin + end) does not wrap around
  assert(begin < SIZE_MAX / 2);
  assert(end < SIZE_MAX / 2);
  size_t mid = (begin + end) / 2;
  _stufflib_sort_merge(n, tmp, src, begin, mid);
  _stufflib_sort_merge(n, tmp, src, mid, end);
  _stufflib_sort_merge_partitions(n, src, tmp, begin, mid, end);
}

double* stufflib_sort_merge(const size_t n, double src[static n]) {
  assert(n);
  double* tmp = malloc(n * sizeof(*src));
  if (!tmp) {
    return NULL;
  }
  stufflib_copy_doubles(n, src, tmp);
  _stufflib_sort_merge(n, src, tmp, 0, n);
  stufflib_copy_doubles(n, tmp, src);
  free(tmp);
  return src;
}

size_t _stufflib_sort_hoare_partition(const size_t n, double src[static n], size_t lo, size_t hi) {
  const double pivot = src[(lo + hi) / 2];
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
    const double tmp = src[lhs];
    src[lhs] = src[rhs];
    src[rhs] = tmp;
  }
}

void _stufflib_sort_quick(const size_t n, double src[static n], size_t lo, size_t hi) {
  if (lo >= hi || lo >= n || hi >= n) {
    return;
  }
  size_t pivot_idx = _stufflib_sort_hoare_partition(n, src, lo, hi);
  _stufflib_sort_quick(n, src, lo, pivot_idx);
  _stufflib_sort_quick(n, src, pivot_idx + 1, hi);
}

double* stufflib_sort_quick(const size_t n, double src[static n]) {
  assert(n);
  double* tmp = malloc(n * sizeof(*src));
  if (!tmp) {
    return NULL;
  }
  stufflib_copy_doubles(n, src, tmp);
  _stufflib_sort_quick(n, tmp, 0, n - 1);
  stufflib_copy_doubles(n, tmp, src);
  free(tmp);
  return src;
}

double* stufflib_sort(const size_t n, double src[static n]) {
  if (n == 1) {
    return src;
  }
  if (!stufflib_sort_quick(n, src)) {
    return NULL;
  }
  return src;
}

int stufflib_sort_is_sorted(const size_t n, double x[static n]) {
  for (size_t i = 1; i < n; ++i) {
    if (x[i - 1] > x[i]) {
      return 0;
    }
  }
  return 1;
}

#endif  // _STUFFLIB_SORT_H_INCLUDED
