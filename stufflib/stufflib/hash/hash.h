#ifndef SL_HASH_H_INCLUDED
#define SL_HASH_H_INCLUDED
// CRC-32 adapted from the PNG specification
// http://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html
// Accessed 2023-02-05
//
// Adler-32 adapted from Wikipedia
// https://en.wikipedia.org/wiki/Adler-32#Example_implementation
// Accessed 2023-02-07

#include <stdint.h>

#include "../macros/macros.h"

void sl_hash_crc32_lut_init(void);

uint32_t sl_hash_crc32(const uint32_t crc32_init,
                       const size_t count,
                       const void* raw_data);

uint32_t sl_hash_crc32_bytes(const size_t count,
                             const unsigned char data[const count]);

uint32_t sl_hash_crc32_str(const char str[const static 1]);

uint32_t sl_hash_adler32(const size_t count,
                         const unsigned char data[const count]);

#endif  // SL_HASH_H_INCLUDED
