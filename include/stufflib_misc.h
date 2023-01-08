#ifndef _STUFFLIB_MISC_H_INCLUDED
#define _STUFFLIB_MISC_H_INCLUDED
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define STUFFLIB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define STUFFLIB_MAX(x, y) ((x) < (y) ? (y) : (x))

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
  const size_t size;
  unsigned char* data;
};

#endif  // _STUFFLIB_MISC_H_INCLUDED
