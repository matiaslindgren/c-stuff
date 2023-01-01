#ifndef _STUFFLIB_SORT_H_INCLUDED
#define _STUFFLIB_SORT_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_misc.h"

typedef int stufflib_sort_compare(const void*, const void*);

typedef void* stufflib_sort(void*,
                            const size_t,
                            const size_t,
                            stufflib_sort_compare*);
typedef double* stufflib_sort_double(const size_t, double*);
typedef char** stufflib_sort_str(const size_t, char**);

void _stufflib_sort_merge_merge(const size_t size,
                                void* restrict src,
                                void* restrict dst,
                                const size_t begin,
                                const size_t mid,
                                const size_t end,
                                stufflib_sort_compare* const compare) {
  size_t lhs = begin;
  size_t rhs = mid;
  size_t out = begin;
  while (lhs < mid && rhs < end) {
    if (compare(src + lhs * size, src + rhs * size) < 0) {
      memcpy(dst + out * size, src + lhs * size, size);
      ++lhs;
    } else {
      memcpy(dst + out * size, src + rhs * size, size);
      ++rhs;
    }
    ++out;
  }
  while (lhs < mid) {
    memcpy(dst + out * size, src + lhs * size, size);
    ++lhs;
    ++out;
  }
  while (rhs < end) {
    memcpy(dst + out * size, src + rhs * size, size);
    ++rhs;
    ++out;
  }
}

void _stufflib_sort_merge(const size_t size,
                          void* restrict src,
                          void* restrict dst,
                          const size_t begin,
                          const size_t end,
                          stufflib_sort_compare* const compare) {
  assert(begin <= end);
  if (end - begin <= 1) {
    return;
  }
  const size_t mid = stufflib_misc_midpoint(begin, end);
  _stufflib_sort_merge(size, dst, src, begin, mid, compare);
  _stufflib_sort_merge(size, dst, src, mid, end, compare);
  _stufflib_sort_merge_merge(size, src, dst, begin, mid, end, compare);
}

void* stufflib_sort_merge(void* src,
                          const size_t count,
                          const size_t size,
                          stufflib_sort_compare* const compare) {
  assert(count);
  assert(src);
  void* tmp = calloc(count, size);
  if (!tmp) {
    return 0;
  }
  memcpy(tmp, src, count * size);
  _stufflib_sort_merge(size, src, tmp, 0, count, compare);
  memcpy(src, tmp, count * size);
  free(tmp);
  return src;
}

inline void _stufflib_sort_swap(void* lhs, void* rhs, const size_t size) {
  unsigned char* tmp = malloc(size);
  memcpy(tmp, lhs, size);
  memcpy(lhs, rhs, size);
  memcpy(rhs, tmp, size);
  free(tmp);
}

size_t _stufflib_sort_hoare_partition(const size_t count,
                                      const size_t size,
                                      void* src,
                                      const size_t lo,
                                      const size_t hi,
                                      stufflib_sort_compare* const compare) {
  const size_t pivot = stufflib_misc_midpoint(lo, hi);
  const size_t pivot_pos = count * size;
  memcpy(src + pivot_pos, src + pivot * size, size);
  size_t lhs = lo - 1;
  size_t rhs = hi + 1;
  while (1) {
    do {
      ++lhs;
    } while (compare(src + lhs * size, src + pivot_pos) < 0);
    do {
      --rhs;
    } while (compare(src + rhs * size, src + pivot_pos) > 0);
    if (lhs >= rhs) {
      return rhs;
    }
    _stufflib_sort_swap(src + lhs * size, src + rhs * size, size);
  }
}

void _stufflib_sort_quick(const size_t count,
                          const size_t size,
                          void* src,
                          const size_t lo,
                          const size_t hi,
                          stufflib_sort_compare* const compare) {
  if (lo >= hi || lo >= count || hi >= count) {
    return;
  }
  const size_t pivot =
      _stufflib_sort_hoare_partition(count, size, src, lo, hi, compare);
  _stufflib_sort_quick(count, size, src, lo, pivot, compare);
  _stufflib_sort_quick(count, size, src, pivot + 1, hi, compare);
}

void* stufflib_sort_quick(void* src,
                          const size_t count,
                          const size_t size,
                          stufflib_sort_compare* const compare) {
  assert(count);
  assert(size);
  assert(src);
  // 1 slot for the pivot at the end
  void* tmp = malloc((count + 1) * size);
  if (!tmp) {
    return 0;
  }
  memcpy(tmp, src, count * size);
  _stufflib_sort_quick(count, size, tmp, 0, count - 1, compare);
  memcpy(src, tmp, count * size);
  free(tmp);
  return src;
}

int stufflib_sort_compare_double(const void* lhs, const void* rhs) {
  const double lhs_num = ((const double*)lhs)[0];
  const double rhs_num = ((const double*)rhs)[0];
  return (lhs_num > rhs_num) - (lhs_num < rhs_num);
}

double* stufflib_sort_quick_double(const size_t count, double src[count]) {
  if (!stufflib_sort_quick((void*)src,
                           count,
                           sizeof(double),
                           stufflib_sort_compare_double)) {
    return 0;
  }
  return src;
}

double* stufflib_sort_merge_double(const size_t count, double src[count]) {
  if (!stufflib_sort_merge((void*)src,
                           count,
                           sizeof(double),
                           stufflib_sort_compare_double)) {
    return 0;
  }
  return src;
}

int stufflib_sort_compare_str(const void* lhs, const void* rhs) {
  return strcmp(((const char**)lhs)[0], ((const char**)rhs)[0]);
}

char** stufflib_sort_quick_str(const size_t count, char* src[count]) {
  if (!stufflib_sort_quick((void*)src,
                           count,
                           sizeof(char*),
                           stufflib_sort_compare_str)) {
    return 0;
  }
  return src;
}

char** stufflib_sort_merge_str(const size_t count, char* src[count]) {
  if (!stufflib_sort_merge((void*)src,
                           count,
                           sizeof(char*),
                           stufflib_sort_compare_str)) {
    return 0;
  }
  return src;
}

#endif  // _STUFFLIB_SORT_H_INCLUDED
