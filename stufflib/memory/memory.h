#ifndef SL_MEMORY_H_INCLUDED
#define SL_MEMORY_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib/macros/macros.h"

void sl_memset_explicit(const size_t size,
                        unsigned char data[const size],
                        const unsigned char value) {
  for (size_t i = 0; i < size; ++i) {
    data[i] = value;
  }
}

void* sl_alloc(const size_t num, const size_t size) {
  if (num * size == 0) {
    SL_LOG_ERROR("will not attempt allocation of 0 bytes");
    exit(1);
  }
  void* mem = calloc(num, size);
  if (!mem) {
    SL_LOG_ERROR("failed allocating %zu bytes", num * size);
    exit(1);
  }
  return mem;
}

void* sl_realloc(void* data,
                 const size_t old_count,
                 const size_t new_count,
                 const size_t size) {
  if (!data && new_count * size == 0) {
    SL_LOG_ERROR("will not realloc nullptr to size 0");
    exit(1);
  }
  void* new_data = realloc(data, new_count * size);
  if (!new_data) {
    SL_LOG_ERROR("failed resizing %p to %zu bytes", data, new_count * size);
    exit(1);
  }
  if (new_count > old_count) {
    const size_t tail_size = (new_count - old_count) * size;
    unsigned char* tail = ((unsigned char*)new_data) + old_count * size;
    sl_memset_explicit(tail_size, tail, 0);
  }
  return new_data;
}

void sl_free(void* data) {
  if (data) {
    free(data);
  }
}

#endif  // SL_MEMORY_H_INCLUDED
