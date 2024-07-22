#ifndef SL_SORT_H_INCLUDED
#define SL_SORT_H_INCLUDED

typedef int sl_sort_compare(const void*, const void*);

typedef void* sl_sort(void*, const size_t, const size_t, sl_sort_compare*);
typedef double* sl_sort_double(const size_t, double*);
typedef char** sl_sort_str(const size_t, char**);

void sl_sort_internal_mergesort_merge(const size_t size,
                                      void* restrict src_raw,
                                      void* restrict dst_raw,
                                      const size_t begin,
                                      const size_t mid,
                                      const size_t end,
                                      sl_sort_compare* const compare);

void sl_sort_internal_mergesort(const size_t size,
                                void* restrict src,
                                void* restrict dst,
                                const size_t begin,
                                const size_t end,
                                sl_sort_compare* const compare);

void* sl_sort_mergesort(void* src,
                        const size_t count,
                        const size_t size,
                        sl_sort_compare* const compare);

size_t sl_sort_internal_hoare_partition(const size_t size,
                                        void* restrict src_raw,
                                        void* restrict tmp_raw,
                                        const size_t lo,
                                        const size_t hi,
                                        sl_sort_compare* const compare);

void sl_sort_internal_quicksort(const size_t count,
                                const size_t size,
                                void* src,
                                void* tmp,
                                const size_t lo,
                                const size_t hi,
                                sl_sort_compare* const compare);

void* sl_sort_quicksort(void* src,
                        const size_t count,
                        const size_t size,
                        sl_sort_compare* const compare);

int sl_sort_compare_double(const void* lhs_data, const void* rhs_data);

double* sl_sort_quicksort_double(const size_t count, double src[count]);

double* sl_sort_mergesort_double(const size_t count, double src[count]);

int sl_sort_compare_str(const void* lhs_data, const void* rhs_data);

char** sl_sort_quicksort_str(const size_t count, char* src[count]);

char** sl_sort_mergesort_str(const size_t count, char* src[count]);

#endif  // SL_SORT_H_INCLUDED
