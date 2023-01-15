#ifndef _STUFFLIB_MISC_H_INCLUDED
#define _STUFFLIB_MISC_H_INCLUDED
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STUFFLIB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define STUFFLIB_MAX(x, y) ((x) < (y) ? (y) : (x))
#define STUFFLIB_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define UINT16_BYTES (16 / CHAR_BIT)
#define UINT32_BYTES (32 / CHAR_BIT)

uint16_t stufflib_misc_parse_lil_endian_u16(
    const unsigned char buf[static UINT16_BYTES]) {
  uint16_t x = 0;
  for (size_t i = 0; i < UINT16_BYTES; ++i) {
    const uint16_t buf_value = buf[i];
    x |= buf_value << (CHAR_BIT * i);
  }
  return x;
}

uint32_t stufflib_misc_parse_big_endian_u32(
    const unsigned char buf[static UINT32_BYTES]) {
  uint32_t x = 0;
  for (size_t i = 0; i < UINT32_BYTES; ++i) {
    const uint32_t buf_value = buf[i];
    const size_t byte_offset = CHAR_BIT * (UINT32_BYTES - (i + 1));
    x |= buf_value << byte_offset;
  }
  return x;
}

#undef UINT16_BYTES
#undef UINT32_BYTES

// https://en.wikipedia.org/wiki/Adler-32
uint32_t stufflib_misc_adler32(const size_t n, unsigned char data[n]) {
  const uint32_t mod_adler = 65521;
  uint32_t a = 1;
  uint32_t b = 0;
  for (size_t index = 0; index < n; ++index) {
    a = (a + data[index]) % mod_adler;
    b = (b + a) % mod_adler;
  }
  return (b << 16) | a;
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
  unsigned char* data = calloc(size, sizeof(unsigned char));
  return (stufflib_data){
      .size = data ? size : 0,
      .data = data,
  };
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

#endif  // _STUFFLIB_MISC_H_INCLUDED
