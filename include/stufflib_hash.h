#ifndef _STUFFLIB_HASH_H_INCLUDED
#define _STUFFLIB_HASH_H_INCLUDED
// Adapted from the PNG specification
// http://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html
// Accessed 2023-02-05
#include <inttypes.h>
#include <limits.h>
#include <string.h>

#include "stufflib_misc.h"

static uint32_t stufflib_hash_crc32_lut[0xff + 1] = {0};
static int stufflib_hash_crc32_lut_computed = 0;

void stufflib_hash_crc32_lut_init() {
  for (size_t n = 0; n < STUFFLIB_ARRAY_LEN(stufflib_hash_crc32_lut); ++n) {
    uint32_t c = n;
    for (size_t k = 0; k < CHAR_BIT; ++k) {
      if (c & 1) {
        c = 0xedb88320L ^ (c >> 1);
      } else {
        c = c >> 1;
      }
    }
    stufflib_hash_crc32_lut[n] = c;
  }
}

uint32_t stufflib_hash_crc32(const uint32_t crc32_init,
                             const size_t count,
                             const void* data) {
  if (!stufflib_hash_crc32_lut_computed) {
    stufflib_hash_crc32_lut_init();
    stufflib_hash_crc32_lut_computed = 1;
  }
  uint32_t crc32 = crc32_init;
  for (size_t i = 0; i < count; ++i) {
    const size_t lut_index = (crc32 ^ ((unsigned char*)data)[i]) & 0xff;
    crc32 = stufflib_hash_crc32_lut[lut_index] ^ (crc32 >> CHAR_BIT);
  }
  return crc32;
}

uint32_t stufflib_hash_crc32_bytes(const size_t count,
                                   const unsigned char data[const count]) {
  return stufflib_hash_crc32(0xffffffff, count, data) ^ 0xffffffff;
}

uint32_t stufflib_hash_crc32_str(const char str[const static 1]) {
  return stufflib_hash_crc32(0xffffffff, strlen(str), str) ^ 0xffffffff;
}

#endif  // _STUFFLIB_HASH_H_INCLUDED
