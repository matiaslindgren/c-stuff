#ifndef _STUFFLIB_SORT_H_INCLUDED
#define _STUFFLIB_SORT_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_misc.h"

typedef int(stufflib_sort_compare)(const void*, const void*);
typedef void(stufflib_sort_swap)(void*, void*);

typedef void*(stufflib_sort)(const size_t, const size_t, void*);
typedef double*(stufflib_sort_double)(const size_t, double*);
typedef char**(stufflib_sort_str)(const size_t, char**);

void _stufflib_sort_merge_partitions(const size_t n,
                                     const size_t offset,
                                     void* restrict src,
                                     void* restrict dst,
                                     const size_t begin,
                                     const size_t mid,
                                     const size_t end,
                                     stufflib_sort_compare* const compare,
                                     stufflib_sort_swap* const swap) {
  size_t lhs = begin;
  size_t rhs = mid;
  size_t out = begin;
  while (lhs < mid && rhs < end) {
    if (compare(src + lhs * offset, src + rhs * offset) < 0) {
      swap(dst + out * offset, src + lhs * offset);
      ++lhs;
    } else {
      swap(dst + out * offset, src + rhs * offset);
      ++rhs;
    }
    ++out;
  }
  while (lhs < mid) {
    swap(dst + out * offset, src + lhs * offset);
    ++lhs;
    ++out;
  }
  while (rhs < end) {
    swap(dst + out * offset, src + rhs * offset);
    ++rhs;
    ++out;
  }
}

void _stufflib_sort_merge(const size_t n,
                          const size_t offset,
                          void* restrict src,
                          void* restrict dst,
                          const size_t begin,
                          const size_t end,
                          stufflib_sort_compare* const compare,
                          stufflib_sort_swap* const swap) {
  assert(begin <= end);
  if (end - begin <= 1) {
    return;
  }
  const size_t mid = stufflib_misc_midpoint(begin, end);
  _stufflib_sort_merge(n, offset, dst, src, begin, mid, compare, swap);
  _stufflib_sort_merge(n, offset, dst, src, mid, end, compare, swap);
  _stufflib_sort_merge_partitions(n,
                                  offset,
                                  src,
                                  dst,
                                  begin,
                                  mid,
                                  end,
                                  compare,
                                  swap);
}

void* stufflib_sort_merge(const size_t n,
                          const size_t offset,
                          void* src,
                          stufflib_sort_compare* const compare,
                          stufflib_sort_swap* const swap) {
  assert(n);
  assert(src);
  void* tmp = calloc(n, offset);
  if (!tmp) {
    return 0;
  }
  memcpy(tmp, src, n * offset);
  _stufflib_sort_merge(n, offset, src, tmp, 0, n, compare, swap);
  memcpy(src, tmp, n * offset);
  free(tmp);
  return src;
}

size_t _stufflib_sort_hoare_partition(const size_t n,
                                      const size_t offset,
                                      void* src,
                                      const size_t lo,
                                      const size_t hi,
                                      stufflib_sort_compare* const compare,
                                      stufflib_sort_swap* const swap) {
  const size_t pivot = stufflib_misc_midpoint(lo, hi);
  memcpy(src + n * offset, src + pivot * offset, offset);
  size_t lhs = lo - 1;
  size_t rhs = hi + 1;
  while (1) {
    do {
      ++lhs;
    } while (compare(src + lhs * offset, src + n * offset) < 0);
    do {
      --rhs;
    } while (compare(src + rhs * offset, src + n * offset) > 0);
    if (lhs >= rhs) {
      return rhs;
    }
    swap(src + lhs * offset, src + rhs * offset);
  }
}

void _stufflib_sort_quick(const size_t n,
                          const size_t offset,
                          void* src,
                          const size_t lo,
                          const size_t hi,
                          stufflib_sort_compare* const compare,
                          stufflib_sort_swap* const swap) {
  if (lo >= hi || lo >= n || hi >= n) {
    return;
  }
  const size_t pivot =
      _stufflib_sort_hoare_partition(n, offset, src, lo, hi, compare, swap);
  _stufflib_sort_quick(n, offset, src, lo, pivot, compare, swap);
  _stufflib_sort_quick(n, offset, src, pivot + 1, hi, compare, swap);
}

void* stufflib_sort_quick(const size_t n,
                          const size_t offset,
                          void* src,
                          stufflib_sort_compare* const compare,
                          stufflib_sort_swap* const swap) {
  assert(n);
  assert(offset);
  assert(src);
  // 1 slot for the pivot at the end
  void* tmp = malloc((n + 1) * offset);
  if (!tmp) {
    return 0;
  }
  memcpy(tmp, src, n * offset);
  _stufflib_sort_quick(n, offset, tmp, 0, n - 1, compare, swap);
  memcpy(src, tmp, n * offset);
  free(tmp);
  return src;
}

int stufflib_sort_compare_double(const void* lhs, const void* rhs) {
  const double lhs_num = ((const double*)lhs)[0];
  const double rhs_num = ((const double*)rhs)[0];
  if (lhs_num < rhs_num) return -1;
  if (lhs_num > rhs_num) return 1;
  return 0;
}

void stufflib_sort_swap_double(void* lhs, void* rhs) {
  unsigned char* tmp = malloc(sizeof(double));
  memcpy(tmp, lhs, sizeof(double));
  memcpy(lhs, rhs, sizeof(double));
  memcpy(rhs, tmp, sizeof(double));
  free(tmp);
}

double* stufflib_sort_quick_double(const size_t n, double src[n]) {
  if (!stufflib_sort_quick(n,
                           sizeof(double),
                           src,
                           stufflib_sort_compare_double,
                           stufflib_sort_swap_double)) {
    return 0;
  }
  return src;
}

double* stufflib_sort_merge_double(const size_t n, double src[n]) {
  if (!stufflib_sort_merge(n,
                           sizeof(double),
                           src,
                           stufflib_sort_compare_double,
                           stufflib_sort_swap_double)) {
    return 0;
  }
  return src;
}

int stufflib_sort_compare_str(const void* lhs, const void* rhs) {
  return strcmp(((const char**)lhs)[0], ((const char**)rhs)[0]);
}

void stufflib_sort_swap_str(void* lhs, void* rhs) {
  const char** lhs_str = (const char**)(lhs);
  const char** rhs_str = (const char**)(rhs);
  const char* tmp = *lhs_str;
  *lhs_str = *rhs_str;
  *rhs_str = tmp;
}

char** stufflib_sort_quick_str(const size_t n, char* src[n]) {
  if (!stufflib_sort_quick(n,
                           sizeof(char*),
                           src,
                           stufflib_sort_compare_str,
                           stufflib_sort_swap_str)) {
    return 0;
  }
  return src;
}

char** stufflib_sort_merge_str(const size_t n, char* src[n]) {
  if (!stufflib_sort_merge(n,
                           sizeof(char*),
                           src,
                           stufflib_sort_compare_str,
                           stufflib_sort_swap_str)) {
    return 0;
  }
  return src;
}

#endif  // _STUFFLIB_SORT_H_INCLUDED
