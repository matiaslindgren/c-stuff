#ifndef SL_DATA_H_INCLUDED
#define SL_DATA_H_INCLUDED
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"

struct sl_span {
  bool owned;
  size_t size;
  unsigned char* data;
};

struct sl_span sl_span_view(const size_t size, unsigned char data[size]) {
  assert(size);
  return (struct sl_span){
      .owned = false,
      .size = size,
      .data = data,
  };
}

struct sl_span sl_span_create(const size_t size) {
  return (struct sl_span){
      .owned = true,
      .size = size,
      .data = size ? sl_alloc(size, sizeof(unsigned char)) : nullptr,
  };
}

void sl_span_delete(struct sl_span data[static 1]) {
  if (data->owned && data->data) {
    sl_free(data->data);
  }
  *data = (struct sl_span){0};
}

struct sl_span sl_span_copy(const struct sl_span src[const static 1]) {
  struct sl_span dst = sl_span_create(src->size);
  if (dst.size) {
    memcpy(dst.data, src->data, dst.size);
  }
  return dst;
}

struct sl_span sl_span_from_str(const char str[const static 1]) {
  const size_t num_chars = strlen(str);
  if (!num_chars) {
    return sl_span_create(0);
  }
  struct sl_span str_view = sl_span_view(num_chars, (unsigned char*)str);
  return sl_span_copy(&str_view);
}

bool sl_span_is_hexadecimal_str(const struct sl_span src[const static 1]) {
  return src->size > 2 && memcmp(src->data, "0x", 2) == 0;
}

struct sl_span sl_span_parse_hex(const struct sl_span src[const static 1]) {
  assert(sl_span_is_hexadecimal_str(src));
  const size_t num_bytes = ((src->size - 2) + 1) / 2;
  struct sl_span dst = sl_span_create(num_bytes);
  for (size_t i_byte = 0; i_byte < dst.size; ++i_byte) {
    const size_t i1 = 2 * i_byte + 2;
    const size_t i2 = 2 * i_byte + 3;
    const char char1 = src->data[i1];
    const char char2 = i2 < src->size ? src->data[i2] : 0;
    const char byte[] = {char1, char2, 0};
    dst.data[i_byte] = strtoul(byte, 0, 16);
  }
  return dst;
}

struct sl_span sl_span_concat(const struct sl_span data1[const static 1],
                              const struct sl_span data2[const static 1]) {
  struct sl_span dst = sl_span_create(data1->size + data2->size);
  if (data1->size) {
    memcpy(dst.data, data1->data, data1->size);
  }
  if (data2->size) {
    memcpy(dst.data + data1->size, data2->data, data2->size);
  }
  return dst;
}

void sl_span_extend(struct sl_span dst[static 1],
                    const struct sl_span src[const static 1]) {
  struct sl_span tmp = sl_span_concat(dst, src);
  sl_span_delete(dst);
  *dst = tmp;
}

struct sl_span sl_span_slice(const struct sl_span data[const static 1],
                             const size_t begin,
                             const size_t end_or_max) {
  const size_t end = SL_MIN(end_or_max, data->size);
  if (begin >= end) {
    return (struct sl_span){0};
  }
  const size_t slice_size = end - begin;
  unsigned char* const slice_begin = data->data + begin;
  return sl_span_view(slice_size, slice_begin);
}

struct sl_span sl_span_find(const struct sl_span data[const static 1],
                            const struct sl_span pattern[const static 1]) {
  if (data->size && pattern->size) {
    for (size_t begin = 0; begin + pattern->size <= data->size; ++begin) {
      unsigned char* subseq = data->data + begin;
      if (memcmp(subseq, pattern->data, pattern->size) == 0) {
        const size_t subseq_size = data->size - begin;
        return sl_span_view(subseq_size, subseq);
      }
    }
  }
  return (struct sl_span){0};
}

int sl_span_compare(const struct sl_span lhs[const static 1],
                    const struct sl_span rhs[const static 1]) {
  if (lhs->data && rhs->data) {
    return memcmp(lhs->data, rhs->data, SL_MIN(lhs->size, rhs->size));
  }
  return (!rhs->data) - (!lhs->data);
}

void* sl_span_iter_get_item(struct sl_iterator iter[const static 1]) {
  struct sl_span* data = iter->data;
  return data->data + iter->index;
}

void sl_span_iter_advance(struct sl_iterator iter[const static 1]) {
  ++(iter->index);
  ++(iter->pos);
}

bool sl_span_iter_is_done(struct sl_iterator iter[const static 1]) {
  const struct sl_span* data = iter->data;
  return iter->index == data->size;
}

struct sl_iterator sl_span_iter(const struct sl_span data[const static 1]) {
  return (struct sl_iterator){
      .data = (void*)data,
      .get_item = sl_span_iter_get_item,
      .advance = sl_span_iter_advance,
      .is_done = sl_span_iter_is_done,
  };
}

#endif  // SL_DATA_H_INCLUDED
