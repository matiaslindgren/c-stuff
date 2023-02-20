#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_data.h"
#include "stufflib_string.h"
#include "stufflib_test.h"

// clang-format off
stufflib_data hello_utf8[] = {
  // hello
  {.size=5,  .data=(unsigned char[]){0x68,0x65,0x6c,0x6c,0x6f}},
  // नमस्ते
  {.size=18, .data=(unsigned char[]){0xe0,0xa4,0xa8,0xe0,0xa4,0xae,0xe0,0xa4,0xb8,0xe0,0xa5,0x8d,0xe0,0xa4,0xa4,0xe0,0xa5,0x87}},
  // გამარჯობა
  {.size=27, .data=(unsigned char[]){0xe1,0x83,0x92,0xe1,0x83,0x90,0xe1,0x83,0x9b,0xe1,0x83,0x90,0xe1,0x83,0xa0,0xe1,0x83,0xaf,0xe1,0x83,0x9d,0xe1,0x83,0x91,0xe1,0x83,0x90}},
  // مرحبًا
  {.size=12, .data=(unsigned char[]){0xd9,0x85,0xd8,0xb1,0xd8,0xad,0xd8,0xa8,0xd9,0x8b,0xd8,0xa7}},
  // こんにちは
  {.size=15, .data=(unsigned char[]){0xe3,0x81,0x93,0xe3,0x82,0x93,0xe3,0x81,0xab,0xe3,0x81,0xa1,0xe3,0x81,0xaf}},
  // 今日は
  {.size=9,  .data=(unsigned char[]){0xe4,0xbb,0x8a,0xe6,0x97,0xa5,0xe3,0x81,0xaf}},
  // 안녕하세요
  {.size=15, .data=(unsigned char[]){0xec,0x95,0x88,0xeb,0x85,0x95,0xed,0x95,0x98,0xec,0x84,0xb8,0xec,0x9a,0x94}},
  // привіт
  {.size=12, .data=(unsigned char[]){0xd0,0xbf,0xd1,0x80,0xd0,0xb8,0xd0,0xb2,0xd1,0x96,0xd1,0x82}},
  // สวัสดี
  {.size=18, .data=(unsigned char[]){0xe0,0xb8,0xaa,0xe0,0xb8,0xa7,0xe0,0xb8,0xb1,0xe0,0xb8,0xaa,0xe0,0xb8,0x94,0xe0,0xb8,0xb5}},
};

char32_t decoded_strings[] = {
  // hello
  0x68,0x65,0x6c,0x6c,0x6f,
  // नमस्ते
  0x928,0x92e,0x938,0x94d,0x924,0x947,
  // გამარჯობა
  0x10d2,0x10d0,0x10db,0x10d0,0x10e0,0x10ef,0x10dd,0x10d1,0x10d0,
  // مرحبًا
  0x645,0x631,0x62d,0x628,0x64b,0x627,
  // こんにちは
  0x3053,0x3093,0x306b,0x3061,0x306f,
  // 今日は
  0x4eca,0x65e5,0x306f,
  // 안녕하세요
  0xc548,0xb155,0xd558,0xc138,0xc694,
  // привіт
  0x43f,0x440,0x438,0x432,0x456,0x442,
  // สวัสดี
  0xe2a,0xe27,0xe31,0xe2a,0xe14,0xe35,
};

size_t decoded_lengths[] = {
  // hello
  5,
  // नमस्ते
  6,
  // გამარჯობა
  9,
  // مرحبًا
  6,
  // こんにちは
  5,
  // 今日は
  3,
  // 안녕하세요
  5,
  // привіт
  6,
  // สวัสดี
  6,
};
// clang-format on

bool test_string_init(const bool verbose) {
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(hello_utf8); ++i) {
    const stufflib_string* const str =
        stufflib_string_from_utf8(&((stufflib_string){0}), hello_utf8 + i);
    assert(str);
    assert(str->length == decoded_lengths[i]);
  }
  return true;
}

bool test_string_slice(const bool verbose) {
  size_t decoded_pos = 0;
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(hello_utf8); ++i) {
    const stufflib_string str =
        *stufflib_string_from_utf8(&((stufflib_string){0}), hello_utf8 + i);
    for (size_t substr_len = 0; substr_len <= str.length; ++substr_len) {
      for (size_t begin = 0; begin + substr_len <= str.length; ++begin) {
        const stufflib_string substr =
            stufflib_string_slice(&str, begin, begin + substr_len);
        assert(substr.length == substr_len);
        for (stufflib_iterator iter =
                 stufflib_unicode_iter(&(substr.utf8_data));
             !stufflib_unicode_iter_end(&iter);
             stufflib_unicode_iter_advance(&iter)) {
          const char32_t codepoint = stufflib_unicode_iter_decode_item(&iter);
          assert(codepoint == decoded_strings[decoded_pos + begin + iter.pos]);
          // TODO assert iterator slice address equals underlying string
        }
      }
    }
    decoded_pos += decoded_lengths[i];
  }
  return true;
}

bool test_string_strstr_no_match(const bool verbose) {
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(hello_utf8); ++i) {
    const stufflib_string str =
        *stufflib_string_from_utf8(&((stufflib_string){0}), hello_utf8 + i);
    for (size_t j = i + 1; j < STUFFLIB_ARRAY_LEN(hello_utf8); ++j) {
      const stufflib_string substr =
          *stufflib_string_from_utf8(&((stufflib_string){0}), hello_utf8 + j);
      const stufflib_string match = stufflib_string_strstr(&str, &substr);
      assert(match.length == 0);
      assert(match.utf8_data.size == 0);
      assert(match.utf8_data.data == 0);
    }
  }
  return true;
}

STUFFLIB_TEST_MAIN(test_string_init,
                   test_string_slice,
                   test_string_strstr_no_match)
