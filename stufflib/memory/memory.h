#ifndef SL_MEMORY_H_INCLUDED
#define SL_MEMORY_H_INCLUDED

#include <stddef.h>

#include <stufflib/context/context.h>

void sl_memset_explicit(size_t size, unsigned char data[const size], unsigned char value);
void* sl_alloc(struct sl_context ctx[static 1], size_t num, size_t size);
void* sl_realloc(
    struct sl_context ctx[static 1],
    void* data,
    size_t old_count,
    size_t new_count,
    size_t size
);
void sl_free(void* data);

#endif  // SL_MEMORY_H_INCLUDED
