#ifndef SL_HASH_H_INCLUDED
#define SL_HASH_H_INCLUDED
// CRC-32 adapted from the PNG specification
// http://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html
// Accessed 2023-02-05
//
// Adler-32 adapted from Wikipedia
// https://en.wikipedia.org/wiki/Adler-32#Example_implementation
// Accessed 2023-02-07
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "stufflib/macros/macros.h"

static uint32_t sl_hash_crc32_lut[0xff + 1] = {0};
static int sl_hash_crc32_lut_computed = 0;

void sl_hash_crc32_lut_init(void) {
  for (uint32_t n = 0; n < SL_ARRAY_LEN(sl_hash_crc32_lut); ++n) {
    uint32_t c = n;
    for (uint32_t k = 0; k < CHAR_BIT; ++k) {
      if (c & 1) {
        c = 0xedb88320L ^ (c >> 1);
      } else {
        c = c >> 1;
      }
    }
    sl_hash_crc32_lut[n] = c;
  }
}

uint32_t sl_hash_crc32(const uint32_t crc32_init,
                       const size_t count,
                       const void* raw_data) {
  if (!sl_hash_crc32_lut_computed) {
    sl_hash_crc32_lut_init();
    sl_hash_crc32_lut_computed = 1;
  }
  uint32_t crc32 = crc32_init;
  const unsigned char* data = raw_data;
  for (size_t i = 0; i < count; ++i) {
    const size_t lut_index = (crc32 ^ data[i]) & 0xff;
    crc32 = sl_hash_crc32_lut[lut_index] ^ (crc32 >> CHAR_BIT);
  }
  return crc32;
}

uint32_t sl_hash_crc32_bytes(const size_t count,
                             const unsigned char data[const count]) {
  return sl_hash_crc32(0xffffffff, count, data) ^ 0xffffffff;
}

uint32_t sl_hash_crc32_str(const char str[const static 1]) {
  return sl_hash_crc32(0xffffffff, strlen(str), str) ^ 0xffffffff;
}

uint32_t sl_hash_adler32(const size_t count,
                         const unsigned char data[const count]) {
  const uint32_t mod_adler = 65521;
  uint32_t a = 1;
  uint32_t b = 0;
  for (size_t i = 0; i < count; ++i) {
    a = (a + data[i]) % mod_adler;
    b = (b + a) % mod_adler;
  }
  return (b << 16) | a;
}

#endif  // SL_HASH_H_INCLUDED
