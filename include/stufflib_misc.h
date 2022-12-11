#ifndef _STUFFLIB_MISC_H_INCLUDED
#define _STUFFLIB_MISC_H_INCLUDED
#include <inttypes.h>
#include <stdlib.h>

void stufflib_copy_doubles(size_t n, double src[n], double dst[n]) {
  for (size_t i = 0; i < n; ++i) {
    dst[i] = src[i];
  }
}

#ifndef UINT32_WIDTH
#define UINT32_WIDTH (32)
#endif
#ifndef UINT32_BYTES
#define UINT32_BYTES (UINT32_WIDTH / 8)
#endif

uint32_t stufflib_int_parse_big_endian(const unsigned char buf[static UINT32_BYTES]) {
  uint32_t x = 0;
  for (size_t i = 0; i < UINT32_BYTES; ++i) {
    const size_t offset = 8 * (UINT32_BYTES - (i + 1));
    x |= (uint32_t)(buf[i]) << offset;
  }
  return x;
}

#endif  // _STUFFLIB_MISC_H_INCLUDED
