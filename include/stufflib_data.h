#ifndef _STUFFLIB_DATA_H_INCLUDED
#define _STUFFLIB_DATA_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"

typedef struct stufflib_data stufflib_data;
struct stufflib_data {
  size_t size;
  unsigned char* data;
};

stufflib_data stufflib_data_new(const size_t size) {
  return (stufflib_data){
      .size = size,
      .data = stufflib_alloc(size, 1),
  };
}

void stufflib_data_copy(stufflib_data dst[restrict static 1],
                        const stufflib_data src[restrict static 1]) {
  *dst = stufflib_data_new(src->size);
  memcpy(dst->data, src->data, dst->size);
}

void stufflib_data_destroy(stufflib_data data[static 1]) {
  free(data->data);
  *data = (stufflib_data){0};
}

stufflib_data* stufflib_data_concat(stufflib_data dst[static 1],
                                    const stufflib_data src[static 1]) {
  // TODO
  unsigned char* tmp =
      stufflib_realloc(dst->data, dst->size, dst->size + src->size, 1);
  dst->data = tmp;
  memcpy(dst->data + dst->size, src->data, src->size);
  dst->size += src->size;
  return dst;
}

stufflib_data stufflib_data_slice(const stufflib_data data[const static 1],
                                  size_t begin,
                                  size_t end) {
  end = STUFFLIB_MIN(end, data->size);
  if (begin >= end) {
    return (stufflib_data){0};
  }
  return (stufflib_data){
      .size = end - begin,
      .data = data->data + begin,
  };
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

void* stufflib_data_iter_get(stufflib_iterator iter[const static 1]) {
  return ((stufflib_data*)iter->begin)->data + iter->index;
}

void stufflib_data_iter_advance(stufflib_iterator iter[const static 1]) {
  ++(iter->index);
  ++(iter->pos);
}

bool stufflib_data_iter_end(stufflib_iterator iter[const static 1]) {
  const stufflib_data* data = (const stufflib_data*)(iter->begin);
  return iter->index == data->size;
}

stufflib_iterator stufflib_data_iter(const stufflib_data data[const static 1]) {
  return (stufflib_iterator){
      .begin = (void*)data,
      .get = stufflib_data_iter_get,
      .advance = stufflib_data_iter_advance,
      .end = stufflib_data_iter_end,
  };
}

#endif  // _STUFFLIB_DATA_H_INCLUDED
