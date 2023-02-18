#ifndef _STUFFLIB_MISC_H_INCLUDED
#define _STUFFLIB_MISC_H_INCLUDED
#include <limits.h>
#include <stdlib.h>

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

#endif  // _STUFFLIB_MISC_H_INCLUDED
