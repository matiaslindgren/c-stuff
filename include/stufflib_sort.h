#ifndef _STUFFLIB_SORT_H_INCLUDED
#define _STUFFLIB_SORT_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_memory.h"
#include "stufflib_misc.h"

typedef int stufflib_sort_compare(const void*, const void*);

typedef void* stufflib_sort(void*,
                            const size_t,
                            const size_t,
                            stufflib_sort_compare*);
typedef double* stufflib_sort_double(const size_t, double*);
typedef char** stufflib_sort_str(const size_t, char**);

void _stufflib_sort_mergesort_merge(const size_t size,
                                    void* restrict src_raw,
                                    void* restrict dst_raw,
                                    const size_t begin,
                                    const size_t mid,
                                    const size_t end,
                                    stufflib_sort_compare* const compare) {
  unsigned char* src = (unsigned char*)(src_raw);
  unsigned char* dst = (unsigned char*)(dst_raw);
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

void _stufflib_sort_mergesort(const size_t size,
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
  _stufflib_sort_mergesort(size, dst, src, begin, mid, compare);
  _stufflib_sort_mergesort(size, dst, src, mid, end, compare);
  _stufflib_sort_mergesort_merge(size, src, dst, begin, mid, end, compare);
}

void* stufflib_sort_mergesort(void* src,
                              const size_t count,
                              const size_t size,
                              stufflib_sort_compare* const compare) {
  assert(count);
  assert(src);
  void* tmp = stufflib_alloc(count, size);
  memcpy(tmp, src, count * size);
  _stufflib_sort_mergesort(size, src, tmp, 0, count, compare);
  memcpy(src, tmp, count * size);
  stufflib_free(tmp);
  return src;
}

size_t _stufflib_sort_hoare_partition(const size_t count,
                                      const size_t size,
                                      void* restrict src_raw,
                                      void* restrict tmp_raw,
                                      const size_t lo,
                                      const size_t hi,
                                      stufflib_sort_compare* const compare) {
  unsigned char* src = (unsigned char*)(src_raw);
  unsigned char* tmp = (unsigned char*)(tmp_raw);
  unsigned char* pivot = tmp;
  unsigned char* swap_tmp = tmp + size;

  const size_t pivot_idx = stufflib_misc_midpoint(lo, hi);
  memcpy(pivot, src + pivot_idx * size, size);

  size_t lhs = lo - 1;
  size_t rhs = hi + 1;
  while (true) {
    do {
      ++lhs;
    } while (compare(src + lhs * size, pivot) < 0);
    do {
      --rhs;
    } while (compare(src + rhs * size, pivot) > 0);
    if (lhs >= rhs) {
      return rhs;
    }
    memcpy(swap_tmp, src + lhs * size, size);
    memcpy(src + lhs * size, src + rhs * size, size);
    memcpy(src + rhs * size, swap_tmp, size);
  }
}

void _stufflib_sort_quicksort(const size_t count,
                              const size_t size,
                              void* src,
                              void* tmp,
                              const size_t lo,
                              const size_t hi,
                              stufflib_sort_compare* const compare) {
  if (lo >= hi || lo >= count || hi >= count) {
    return;
  }
  const size_t pivot =
      _stufflib_sort_hoare_partition(count, size, src, tmp, lo, hi, compare);
  _stufflib_sort_quicksort(count, size, src, tmp, lo, pivot, compare);
  _stufflib_sort_quicksort(count, size, src, tmp, pivot + 1, hi, compare);
}

void* stufflib_sort_quicksort(void* src,
                              const size_t count,
                              const size_t size,
                              stufflib_sort_compare* const compare) {
  assert(count);
  assert(size);
  assert(src);
  // pivot and swap space
  void* tmp = stufflib_alloc(2, size);
  _stufflib_sort_quicksort(count, size, src, tmp, 0, count - 1, compare);
  stufflib_free(tmp);
  return src;
}

int stufflib_sort_compare_double(const void* lhs, const void* rhs) {
  const double lhs_num = ((const double*)lhs)[0];
  const double rhs_num = ((const double*)rhs)[0];
  return (lhs_num > rhs_num) - (lhs_num < rhs_num);
}

double* stufflib_sort_quicksort_double(const size_t count, double src[count]) {
  return stufflib_sort_quicksort((void*)src,
                                 count,
                                 sizeof(double),
                                 stufflib_sort_compare_double);
}

double* stufflib_sort_mergesort_double(const size_t count, double src[count]) {
  return stufflib_sort_mergesort((void*)src,
                                 count,
                                 sizeof(double),
                                 stufflib_sort_compare_double);
}

int stufflib_sort_compare_str(const void* lhs, const void* rhs) {
  return strcmp(((const char**)lhs)[0], ((const char**)rhs)[0]);
}

char** stufflib_sort_quicksort_str(const size_t count, char* src[count]) {
  return stufflib_sort_quicksort((void*)src,
                                 count,
                                 sizeof(char*),
                                 stufflib_sort_compare_str);
}

char** stufflib_sort_mergesort_str(const size_t count, char* src[count]) {
  return stufflib_sort_mergesort((void*)src,
                                 count,
                                 sizeof(char*),
                                 stufflib_sort_compare_str);
}

#endif  // _STUFFLIB_SORT_H_INCLUDED
