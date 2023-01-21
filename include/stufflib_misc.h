#ifndef _STUFFLIB_MISC_H_INCLUDED
#define _STUFFLIB_MISC_H_INCLUDED
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"

size_t stufflib_misc_parse_lil_endian(const size_t size,
                                      const unsigned char data[size]) {
  size_t x = 0;
  for (size_t i = size - 1; i < size; --i) {
    x <<= CHAR_BIT;
    x |= data[i];
  }
  return x;
}

size_t stufflib_misc_parse_big_endian(const size_t size,
                                      const unsigned char data[size]) {
  size_t x = 0;
  for (size_t i = 0; i < size; ++i) {
    x <<= CHAR_BIT;
    x |= data[i];
  }
  return x;
}

unsigned char* stufflib_misc_encode_lil_endian(const size_t size,
                                               unsigned char dst[size],
                                               const size_t value) {
  size_t tmp = value;
  for (size_t i = 0; i < size; ++i) {
    dst[i] = tmp & STUFFLIB_ONES(1);
    tmp >>= CHAR_BIT;
  }
  return dst;
}

unsigned char* stufflib_misc_encode_big_endian(const size_t size,
                                               unsigned char dst[size],
                                               const size_t value) {
  size_t tmp = value;
  for (size_t i = size - 1; i < size; --i) {
    dst[i] = tmp & STUFFLIB_ONES(1);
    tmp >>= CHAR_BIT;
  }
  return dst;
}

size_t stufflib_misc_midpoint(const size_t lo, const size_t hi) {
  return lo + (hi - lo) / 2;
}

size_t stufflib_misc_vmax_size_t(const size_t n, const size_t v[n]) {
  size_t res = 0;
  for (size_t i = 0; i < n; ++i) {
    res = STUFFLIB_MAX(res, v[i]);
  }
  return res;
}

typedef struct stufflib_data stufflib_data;
struct stufflib_data {
  size_t size;
  unsigned char* data;
};

stufflib_data stufflib_misc_data_new(const size_t size) {
  unsigned char* data = calloc(size, 1);
  return (stufflib_data){
      .size = data ? size : 0,
      .data = data,
  };
}

int stufflib_misc_data_copy(stufflib_data dst[restrict static 1],
                            const stufflib_data src[restrict static 1]) {
  *dst = stufflib_misc_data_new(src->size);
  if (!dst->size) {
    return 0;
  }
  memcpy(dst->data, src->data, dst->size);
  return 1;
}

void stufflib_misc_data_destroy(stufflib_data data[static 1]) {
  free(data->data);
  *data = (stufflib_data){0};
}

stufflib_data* stufflib_misc_concat(stufflib_data dst[static 1],
                                    const stufflib_data src[static 1]) {
  unsigned char* tmp = realloc(dst->data, dst->size + src->size);
  if (!tmp) {
    return 0;
  }
  dst->data = tmp;
  memcpy(dst->data + dst->size, src->data, src->size);
  dst->size += src->size;
  return dst;
}

void stufflib_misc_data_fdump(FILE stream[const static 1],
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

// https://en.wikipedia.org/wiki/Adler-32
uint32_t stufflib_misc_adler32(const stufflib_data data) {
  const uint32_t mod_adler = 65521;
  uint32_t a = 1;
  uint32_t b = 0;
  for (size_t index = 0; index < data.size; ++index) {
    a = (a + data.data[index]) % mod_adler;
    b = (b + a) % mod_adler;
  }
  return (b << 16) | a;
}

#endif  // _STUFFLIB_MISC_H_INCLUDED
