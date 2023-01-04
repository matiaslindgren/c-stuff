#ifndef _STUFFLIB_MISC_H_INCLUDED
#define _STUFFLIB_MISC_H_INCLUDED
#include <inttypes.h>
#include <stdlib.h>

#ifndef UINT32_WIDTH
#define UINT32_WIDTH (32)
#endif
#ifndef UINT32_BYTES
#define UINT32_BYTES (UINT32_WIDTH / 8)
#endif

uint32_t stufflib_misc_parse_big_endian_u32(
    const unsigned char buf[static UINT32_BYTES]) {
  uint32_t x = 0;
  for (size_t i = 0; i < UINT32_BYTES; ++i) {
    const size_t byte_offset = 8 * (UINT32_BYTES - (i + 1));
    const uint32_t buf_value = buf[i];
    x |= buf_value << byte_offset;
  }
  return x;
}

size_t stufflib_misc_midpoint(const size_t lo, const size_t hi) {
  return lo + (hi - lo) / 2;
}

size_t stufflib_misc_vmax_size_t(const size_t n,
                                 const size_t v[const static n]) {
  size_t res = 0;
  for (size_t i = 0; i < n; ++i) {
    res = v[i] > res ? v[i] : res;
  }
  return res;
}

#endif  // _STUFFLIB_MISC_H_INCLUDED
