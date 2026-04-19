#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <stufflib/context/context.h>
#include <stufflib/memory/memory.h>
#include <stufflib/misc/misc.h>
#include <stufflib/sort/sort.h>

#define SL_INSERTSORT_THRESHOLD 24

static void* sl_sort_internal_insertsort(
    const size_t count,
    const size_t size,
    void* restrict src_raw,
    void* restrict tmp_raw,
    sl_sort_compare* const compare
) {
  unsigned char* src      = src_raw;
  unsigned char* swap_tmp = tmp_raw;
  for (size_t rhs = 1; rhs < count; ++rhs) {
    memcpy(swap_tmp, src + (rhs * size), size);
    size_t lhs = rhs;
    for (; lhs > 0 && compare(src + ((lhs - 1) * size), swap_tmp) > 0; --lhs) {
      memcpy(src + (lhs * size), src + ((lhs - 1) * size), size);
    }
    memcpy(src + (lhs * size), swap_tmp, size);
  }
  return src;
}

void* sl_sort_insertsort(
    struct sl_context ctx[static 1],
    void* src,
    const size_t count,
    const size_t size,
    sl_sort_compare* const compare
) {
  assert(count);
  assert(size);
  assert(src);
  void* tmp = sl_alloc(ctx, 1, size);
  if (!tmp) {
    return nullptr;
  }
  sl_sort_internal_insertsort(count, size, src, tmp, compare);
  sl_free(tmp);
  return src;
}

static void sl_sort_internal_mergesort_merge(
    const size_t size,
    void* restrict src_raw,
    void* restrict dst_raw,
    const size_t begin,
    const size_t mid,
    const size_t end,
    sl_sort_compare* const compare
) {
  unsigned char* src = src_raw;
  unsigned char* dst = dst_raw;
  size_t lhs         = begin;
  size_t rhs         = mid;
  size_t out         = begin;
  while (lhs < mid && rhs < end) {
    if (compare(src + (lhs * size), src + (rhs * size)) < 0) {
      memcpy(dst + (out * size), src + (lhs * size), size);
      ++lhs;
    } else {
      memcpy(dst + (out * size), src + (rhs * size), size);
      ++rhs;
    }
    ++out;
  }
  while (lhs < mid) {
    memcpy(dst + (out * size), src + (lhs * size), size);
    ++lhs;
    ++out;
  }
  while (rhs < end) {
    memcpy(dst + (out * size), src + (rhs * size), size);
    ++rhs;
    ++out;
  }
}

static void sl_sort_internal_mergesort(
    const size_t size,
    void* restrict src,
    void* restrict dst,
    const size_t begin,
    const size_t end,
    sl_sort_compare* const compare
) {
  assert(begin <= end);
  if (end - begin <= 1) {
    return;
  }
  const size_t mid = sl_misc_midpoint(begin, end);
  sl_sort_internal_mergesort(size, dst, src, begin, mid, compare);
  sl_sort_internal_mergesort(size, dst, src, mid, end, compare);
  sl_sort_internal_mergesort_merge(size, src, dst, begin, mid, end, compare);
}

void* sl_sort_mergesort(
    struct sl_context ctx[static 1],
    void* src,
    const size_t count,
    const size_t size,
    sl_sort_compare* const compare
) {
  assert(count);
  assert(src);
  void* tmp = sl_alloc(ctx, count, size);
  if (!tmp) {
    return nullptr;
  }
  memcpy(tmp, src, count * size);
  sl_sort_internal_mergesort(size, src, tmp, 0, count, compare);
  memcpy(src, tmp, count * size);
  sl_free(tmp);
  return src;
}

static size_t sl_sort_internal_hoare_partition(
    const size_t size,
    void* restrict src_raw,
    void* restrict tmp_raw,
    const size_t lo,
    const size_t hi,
    sl_sort_compare* const compare
) {
  unsigned char* src      = src_raw;
  unsigned char* pivot    = tmp_raw;
  unsigned char* swap_tmp = pivot + size;

  const size_t pivot_idx = sl_misc_midpoint(lo, hi);
  memcpy(pivot, src + (pivot_idx * size), size);

  size_t lhs = lo - 1;
  size_t rhs = hi + 1;
  while (true) {
    do {
      ++lhs;
    } while (compare(src + (lhs * size), pivot) < 0);
    do {
      --rhs;
    } while (compare(src + (rhs * size), pivot) > 0);
    if (lhs >= rhs) {
      return rhs;
    }
    memcpy(swap_tmp, src + (lhs * size), size);
    memcpy(src + (lhs * size), src + (rhs * size), size);
    memcpy(src + (rhs * size), swap_tmp, size);
  }
}

static void sl_sort_internal_quicksort(
    struct sl_context ctx[static 1],
    const size_t count,
    const size_t size,
    void* src,
    void* tmp,
    const size_t lo,
    const size_t hi,
    sl_sort_compare* const compare
) {
  if (lo >= hi || lo >= count || hi >= count) {
    return;
  }
  if (hi - lo < SL_INSERTSORT_THRESHOLD) {
    unsigned char* src_bytes = src;
    sl_sort_insertsort(ctx, src_bytes + (lo * size), hi - lo + 1, size, compare);
    return;
  }
  const size_t pivot = sl_sort_internal_hoare_partition(size, src, tmp, lo, hi, compare);
  sl_sort_internal_quicksort(ctx, count, size, src, tmp, lo, pivot, compare);
  sl_sort_internal_quicksort(ctx, count, size, src, tmp, pivot + 1, hi, compare);
}

void* sl_sort_quicksort(
    struct sl_context ctx[static 1],
    void* src,
    const size_t count,
    const size_t size,
    sl_sort_compare* const compare
) {
  assert(count);
  assert(size);
  assert(src);
  // pivot and swap space
  void* tmp = sl_alloc(ctx, 2, size);
  if (!tmp) {
    return nullptr;
  }
  sl_sort_internal_quicksort(ctx, count, size, src, tmp, 0, count - 1, compare);
  sl_free(tmp);
  return src;
}

int sl_sort_compare_double(const void* lhs_data, const void* rhs_data) {
  const double* lhs = lhs_data;
  const double* rhs = rhs_data;
  return (lhs[0] > rhs[0]) - (lhs[0] < rhs[0]);
}

double*
sl_sort_insertsort_double(struct sl_context ctx[static 1], const size_t count, double src[count]) {
  return sl_sort_insertsort(ctx, src, count, sizeof(double), sl_sort_compare_double);
}

double*
sl_sort_quicksort_double(struct sl_context ctx[static 1], const size_t count, double src[count]) {
  return sl_sort_quicksort(ctx, src, count, sizeof(double), sl_sort_compare_double);
}

double*
sl_sort_mergesort_double(struct sl_context ctx[static 1], const size_t count, double src[count]) {
  return sl_sort_mergesort(ctx, src, count, sizeof(double), sl_sort_compare_double);
}

int sl_sort_compare_str(const void* lhs_data, const void* rhs_data) {
  const char* const* lhs = (const char* const*)lhs_data;
  const char* const* rhs = (const char* const*)rhs_data;
  return strcmp(lhs[0], rhs[0]);
}

char**
sl_sort_insertsort_str(struct sl_context ctx[static 1], const size_t count, char* src[count]) {
  return sl_sort_insertsort(ctx, (void*)src, count, sizeof(char*), sl_sort_compare_str);
}

char**
sl_sort_quicksort_str(struct sl_context ctx[static 1], const size_t count, char* src[count]) {
  return sl_sort_quicksort(ctx, (void*)src, count, sizeof(char*), sl_sort_compare_str);
}

char**
sl_sort_mergesort_str(struct sl_context ctx[static 1], const size_t count, char* src[count]) {
  return sl_sort_mergesort(ctx, (void*)src, count, sizeof(char*), sl_sort_compare_str);
}

#undef SL_INSERTSORT_THRESHOLD
