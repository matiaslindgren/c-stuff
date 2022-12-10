#ifndef _STUFFLIB_INT_H_INCLUDED
#define _STUFFLIB_INT_H_INCLUDED
#include <inttypes.h>
#include <stdlib.h>

uint32_t stufflib_int_parse_big_endian(const unsigned char buf[static 4]) {
  uint32_t x = 0;
  for (size_t i = 0; i < 4; ++i) {
    const size_t offset = 8 * (4 - (i + 1));
    x |= (uint32_t)(buf[i]) << offset;
  }
  return x;
}

#endif  // _STUFFLIB_INT_H_INCLUDED
