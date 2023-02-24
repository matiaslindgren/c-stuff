#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_hash.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"

bool test_single_byte(const bool verbose) {
  for (uint8_t i = 0; i < 10; ++i) {
    unsigned char data[] = {
        i,
    };
    const uint32_t crc32 = stufflib_hash_crc32_bytes(1, data);
    const uint32_t expected = (uint32_t[]){
        3523407757,
        2768625435,
        1007455905,
        1259060791,
        3580832660,
        2724731650,
        996231864,
        1281784366,
        3705235391,
        2883475241,
    }[i];
    assert(crc32 == expected);
  }
  return true;
}

bool test_two_bytes_big_endian(const bool verbose) {
  for (uint16_t i = 0x100; i < 0x100 + 10; ++i) {
    unsigned char data[] = {
        (i & 0xff00) >> 8,
        i & 0x00ff,
    };
    const uint32_t crc32 = stufflib_hash_crc32_bytes(2, data);
    const uint32_t expected = (uint32_t[]){
        1489118142,
        801444648,
        3066839698,
        3251335684,
        1605363623,
        682153777,
        2980152971,
        3332814365,
        1444522892,
        555653914,
    }[i - 0x100];
    assert(crc32 == expected);
  }
  return true;
}

bool test_small_strings(const bool verbose) {
  const char* inputs[] = {
      "",
      "0",
      "1",
      "10",
      "hello",
      " ",
      "\n",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(inputs); ++i) {
    const uint32_t crc32 = stufflib_hash_crc32_str(inputs[i]);
    const uint32_t expected = (uint32_t[]){
        0,
        4108050209,
        2212294583,
        2707236321,
        907060870,
        3916222277,
        852952723,
    }[i];
    assert(crc32 == expected);
  }
  return true;
}

STUFFLIB_TEST_MAIN(test_single_byte,
                   test_two_bytes_big_endian,
                   test_small_strings)
