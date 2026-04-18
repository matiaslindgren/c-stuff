#include <stdlib.h>

#include <stufflib/context/context.h>
#include <stufflib/memory/memory.h>

void sl_memset_explicit(
    const size_t size,
    unsigned char data[const size],
    const unsigned char value
) {
  for (size_t i = 0; i < size; ++i) {
    data[i] = value;
  }
}

void* sl_alloc(struct sl_context ctx[static 1], const size_t num, const size_t size) {
  if (num * size == 0) {
    SL_ERROR(ctx, "will not attempt allocation of 0 bytes");
    return nullptr;
  }
  void* mem = calloc(num, size);
  if (!mem) {
    SL_ERROR(ctx, "failed allocating %zu bytes", num * size);
    return nullptr;
  }
  return mem;
}

void* sl_realloc(
    struct sl_context ctx[static 1],
    void* data,
    const size_t old_count,
    const size_t new_count,
    const size_t size
) {
  if (!data && new_count * size == 0) {
    SL_ERROR(ctx, "will not realloc nullptr to size 0");
    return nullptr;
  }
  void* new_data = realloc(data, new_count * size);
  if (!new_data) {
    SL_ERROR(ctx, "failed resizing %p to %zu bytes", data, new_count * size);
    return nullptr;
  }
  if (new_count > old_count) {
    const size_t tail_size = (new_count - old_count) * size;
    unsigned char* tail    = ((unsigned char*)new_data) + (old_count * size);
    sl_memset_explicit(tail_size, tail, 0);
  }
  return new_data;
}

void sl_free(void* data) {
  if (data) {
    free(data);
  }
}
