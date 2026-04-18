#ifndef SL_SORT_H_INCLUDED
#define SL_SORT_H_INCLUDED

#include <stddef.h>

#include <stufflib/context/context.h>

typedef int sl_sort_compare(const void*, const void*);

typedef void* sl_sort(void*, const size_t, const size_t, sl_sort_compare*);
typedef double* sl_sort_double(struct sl_context[static 1], const size_t, double*);
typedef char** sl_sort_str(struct sl_context[static 1], const size_t, char**);

void* sl_sort_insertsort(
    struct sl_context ctx[static 1],
    void* src,
    size_t count,
    size_t size,
    sl_sort_compare* compare
);
void* sl_sort_mergesort(
    struct sl_context ctx[static 1],
    void* src,
    size_t count,
    size_t size,
    sl_sort_compare* compare
);
void* sl_sort_quicksort(
    struct sl_context ctx[static 1],
    void* src,
    size_t count,
    size_t size,
    sl_sort_compare* compare
);
int sl_sort_compare_double(const void* lhs_data, const void* rhs_data);
double* sl_sort_insertsort_double(struct sl_context ctx[static 1], size_t count, double src[count]);
double* sl_sort_quicksort_double(struct sl_context ctx[static 1], size_t count, double src[count]);
double* sl_sort_mergesort_double(struct sl_context ctx[static 1], size_t count, double src[count]);
int sl_sort_compare_str(const void* lhs_data, const void* rhs_data);
char** sl_sort_insertsort_str(struct sl_context ctx[static 1], size_t count, char* src[count]);
char** sl_sort_quicksort_str(struct sl_context ctx[static 1], size_t count, char* src[count]);
char** sl_sort_mergesort_str(struct sl_context ctx[static 1], size_t count, char* src[count]);

#endif  // SL_SORT_H_INCLUDED
