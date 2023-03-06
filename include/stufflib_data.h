#ifndef _SL_DATA_H_INCLUDED
#define _SL_DATA_H_INCLUDED
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"

struct sl_data {
  bool owned;
  size_t size;
  unsigned char* data;
};

struct sl_data sl_data_view(const size_t size, unsigned char data[size]) {
  assert(size);
  return (struct sl_data){
      .owned = false,
      .size = size,
      .data = data,
  };
}

struct sl_data sl_data_create(const size_t size) {
  return (struct sl_data){
      .owned = true,
      .size = size,
      .data = size ? sl_alloc(size, sizeof(unsigned char)) : nullptr,
  };
}

void sl_data_delete(struct sl_data data[static 1]) {
  if (data->owned && data->data) {
    sl_free(data->data);
  }
  *data = (struct sl_data){0};
}

struct sl_data sl_data_copy(const struct sl_data src[const static 1]) {
  struct sl_data dst = sl_data_create(src->size);
  if (dst.size) {
    memcpy(dst.data, src->data, dst.size);
  }
  return dst;
}

struct sl_data sl_data_from_str(const char str[const static 1]) {
  const size_t num_chars = strlen(str);
  if (!num_chars) {
    return sl_data_create(0);
  }
  struct sl_data str_view = sl_data_view(num_chars, (unsigned char*)str);
  return sl_data_copy(&str_view);
}

bool sl_data_is_hexadecimal_str(const struct sl_data src[const static 1]) {
  return src->size > 2 && memcmp(src->data, "0x", 2) == 0;
}

struct sl_data sl_data_parse_hex(const struct sl_data src[const static 1]) {
  assert(sl_data_is_hexadecimal_str(src));
  const size_t num_bytes = ((src->size - 2) + 1) / 2;
  struct sl_data dst = sl_data_create(num_bytes);
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

struct sl_data sl_data_concat(
    const struct sl_data data1[const restrict static 1],
    const struct sl_data data2[const restrict static 1]) {
  struct sl_data dst = sl_data_create(data1->size + data2->size);
  if (data1->size) {
    memcpy(dst.data, data1->data, data1->size);
  }
  if (data2->size) {
    memcpy(dst.data + data1->size, data2->data, data2->size);
  }
  return dst;
}

void sl_data_extend(struct sl_data dst[restrict static 1],
                    const struct sl_data src[const restrict static 1]) {
  struct sl_data tmp = sl_data_concat(dst, src);
  sl_data_delete(dst);
  *dst = tmp;
}

struct sl_data sl_data_slice(const struct sl_data data[const static 1],
                             const size_t begin,
                             const size_t end_or_max) {
  const size_t end = SL_MIN(end_or_max, data->size);
  if (begin >= end) {
    return (struct sl_data){0};
  }
  const size_t slice_size = end - begin;
  unsigned char* const slice_begin = data->data + begin;
  return sl_data_view(slice_size, slice_begin);
}

struct sl_data sl_data_find(const struct sl_data data[const static 1],
                            const struct sl_data pattern[const static 1]) {
  if (data->size && pattern->size) {
    for (size_t begin = 0; begin + pattern->size <= data->size; ++begin) {
      unsigned char* subseq = data->data + begin;
      if (memcmp(subseq, pattern->data, pattern->size) == 0) {
        const size_t subseq_size = data->size - begin;
        return sl_data_view(subseq_size, subseq);
      }
    }
  }
  return (struct sl_data){0};
}

int sl_data_compare(const struct sl_data lhs[const restrict static 1],
                    const struct sl_data rhs[const restrict static 1]) {
  if (lhs->data && rhs->data) {
    return memcmp(lhs->data, rhs->data, SL_MIN(lhs->size, rhs->size));
  }
  return (!rhs->data) - (!lhs->data);
}

void* sl_data_iter_get_item(struct sl_iterator iter[const static 1]) {
  struct sl_data* data = iter->data;
  return data->data + iter->index;
}

void sl_data_iter_advance(struct sl_iterator iter[const static 1]) {
  ++(iter->index);
  ++(iter->pos);
}

bool sl_data_iter_is_done(struct sl_iterator iter[const static 1]) {
  const struct sl_data* data = iter->data;
  return iter->index == data->size;
}

struct sl_iterator sl_data_iter(const struct sl_data data[const static 1]) {
  return (struct sl_iterator){
      .data = (void*)data,
      .get_item = sl_data_iter_get_item,
      .advance = sl_data_iter_advance,
      .is_done = sl_data_iter_is_done,
  };
}

#endif  // _SL_DATA_H_INCLUDED
