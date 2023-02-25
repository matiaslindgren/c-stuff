#ifndef _STUFFLIB_DATA_H_INCLUDED
#define _STUFFLIB_DATA_H_INCLUDED
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"

typedef struct stufflib_data stufflib_data;
struct stufflib_data {
  bool owned;
  size_t size;
  unsigned char* data;
};

stufflib_data stufflib_data_view(const size_t size, unsigned char data[size]) {
  assert(size);
  return (stufflib_data){
      .owned = false,
      .size = size,
      .data = data,
  };
}

stufflib_data stufflib_data_create(const size_t size) {
  return (stufflib_data){
      .owned = true,
      .size = size,
      .data = size ? stufflib_alloc(size, sizeof(unsigned char)) : nullptr,
  };
}

void stufflib_data_delete(stufflib_data data[static 1]) {
  if (data->owned && data->data) {
    stufflib_free(data->data);
  }
  *data = (stufflib_data){0};
}

stufflib_data stufflib_data_copy(const stufflib_data src[const static 1]) {
  stufflib_data dst = stufflib_data_create(src->size);
  if (dst.size) {
    memcpy(dst.data, src->data, dst.size);
  }
  return dst;
}

stufflib_data stufflib_data_from_str(const char str[const static 1]) {
  const size_t num_chars = strlen(str);
  if (num_chars > 2 && strncmp(str, "0x", 2) == 0) {
    const size_t num_bytes = STUFFLIB_MAX(1, (num_chars - 2) / 2);
    stufflib_data dst = stufflib_data_create(num_bytes);
    for (size_t i = 0; i < dst.size; ++i) {
      const char byte[] = {str[2 * i + 2], str[2 * i + 3], 0};
      dst.data[i] = strtoul(byte, 0, 16);
    }
    return dst;
  }
  stufflib_data str_view = stufflib_data_view(num_chars, (unsigned char*)str);
  return stufflib_data_copy(&str_view);
}

stufflib_data stufflib_data_concat(
    const stufflib_data data1[const restrict static 1],
    const stufflib_data data2[const restrict static 1]) {
  stufflib_data dst = stufflib_data_create(data1->size + data2->size);
  if (data1->size) {
    memcpy(dst.data, data1->data, data1->size);
  }
  if (data2->size) {
    memcpy(dst.data + data1->size, data2->data, data2->size);
  }
  return dst;
}

void stufflib_data_extend(stufflib_data dst[restrict static 1],
                          const stufflib_data src[const restrict static 1]) {
  stufflib_data tmp = stufflib_data_concat(dst, src);
  stufflib_data_delete(dst);
  *dst = tmp;
}

stufflib_data stufflib_data_slice(const stufflib_data data[const static 1],
                                  const size_t begin,
                                  const size_t end_or_max) {
  const size_t end = STUFFLIB_MIN(end_or_max, data->size);
  if (begin >= end) {
    return (stufflib_data){0};
  }
  const size_t slice_size = end - begin;
  unsigned char* const slice_begin = data->data + begin;
  return stufflib_data_view(slice_size, slice_begin);
}

stufflib_data stufflib_data_find(const stufflib_data data[const static 1],
                                 const stufflib_data pattern[const static 1]) {
  if (data->size && pattern->size) {
    for (size_t begin = 0; begin + pattern->size <= data->size; ++begin) {
      unsigned char* subseq = data->data + begin;
      if (memcmp(subseq, pattern->data, pattern->size) == 0) {
        const size_t subseq_size = data->size - begin;
        return stufflib_data_view(subseq_size, subseq);
      }
    }
  }
  return (stufflib_data){0};
}

int stufflib_data_compare(const stufflib_data lhs[const restrict static 1],
                          const stufflib_data rhs[const restrict static 1]) {
  if (lhs->data && rhs->data) {
    return memcmp(lhs->data, rhs->data, STUFFLIB_MIN(lhs->size, rhs->size));
  }
  return (!rhs->data) - (!lhs->data);
}

void stufflib_data_fdump(FILE stream[const static 1],
                         const stufflib_data data,
                         const size_t bytes_per_line) {
  for (size_t i = 0; i < data.size; ++i) {
    if (i) {
      if (i % bytes_per_line == 0) {
        fprintf(stream, "\n");
      } else if (i % 2 == 0) {
        fprintf(stream, " ");
      }
    }
    fprintf(stream, "%02x", data.data[i]);
  }
  fprintf(stream, "\n");
}

void* stufflib_data_iter_get_item(stufflib_iterator iter[const static 1]) {
  stufflib_data* data = iter->data;
  return data->data + iter->index;
}

void stufflib_data_iter_advance(stufflib_iterator iter[const static 1]) {
  ++(iter->index);
  ++(iter->pos);
}

bool stufflib_data_iter_is_done(stufflib_iterator iter[const static 1]) {
  const stufflib_data* data = iter->data;
  return iter->index == data->size;
}

stufflib_iterator stufflib_data_iter(const stufflib_data data[const static 1]) {
  return (stufflib_iterator){
      .data = (void*)data,
      .get_item = stufflib_data_iter_get_item,
      .advance = stufflib_data_iter_advance,
      .is_done = stufflib_data_iter_is_done,
  };
}

#endif  // _STUFFLIB_DATA_H_INCLUDED
