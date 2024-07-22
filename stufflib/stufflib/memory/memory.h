#ifndef SL_MEMORY_H_INCLUDED
#define SL_MEMORY_H_INCLUDED

void sl_memset_explicit(const size_t size,
                        unsigned char data[const size],
                        const unsigned char value);

void* sl_alloc(const size_t num, const size_t size);

void* sl_realloc(void* data,
                 const size_t old_count,
                 const size_t new_count,
                 const size_t size);

void sl_free(void* data);

#endif  // SL_MEMORY_H_INCLUDED
