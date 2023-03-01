#ifndef __UTF8_TEST_DATA_H_INCLUDED
#define __UTF8_TEST_DATA_H_INCLUDED
#include <uchar.h>

#include "stufflib_data.h"

// clang-format off
stufflib_data hello_utf8[] = {
  {.size=5,  .data=(char8_t*)u8"hello"},
  {.size=18, .data=(char8_t*)u8"नमस्ते"},
  {.size=27, .data=(char8_t*)u8"გამარჯობა"},
  {.size=12, .data=(char8_t*)u8"مرحبًا"},
  {.size=15, .data=(char8_t*)u8"こんにちは"},
  {.size=9,  .data=(char8_t*)u8"今日は"},
  {.size=15, .data=(char8_t*)u8"안녕하세요"},
  {.size=12, .data=(char8_t*)u8"привіт"},
  {.size=18, .data=(char8_t*)u8"สวัสดี"},
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

#endif  // __UTF8_TEST_DATA_H_INCLUDED
