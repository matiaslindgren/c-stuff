#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_test.h"
#include "stufflib_unicode.h"

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

bool test_validate_utf8(const bool verbose) {
  stufflib_data invalid_utf8[] = {
      {.size = 1,                   .data = (unsigned char[]){0x80}},
      {.size = 2,             .data = (unsigned char[]){0xc0, 0x80}},
      {.size = 2,             .data = (unsigned char[]){0xe0, 0x80}},
      {.size = 2,             .data = (unsigned char[]){0xf0, 0x80}},
      {.size = 3,       .data = (unsigned char[]){0xf0, 0x80, 0x80}},
      {.size = 4, .data = (unsigned char[]){0xff, 0x80, 0x80, 0x80}},
      {.size = 5,       .data = (unsigned char[]){0, 1, 2, 0xf4, 0}},
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(invalid_utf8); ++i) {
    assert(!stufflib_unicode_is_valid_utf8(invalid_utf8 + i));
  }
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(hello_utf8); ++i) {
    assert(stufflib_unicode_is_valid_utf8(hello_utf8 + i));
  }
  return true;
}

bool test_decode_codepoints(const bool verbose) {
  size_t codepoint_pos = 0;
  for (size_t i_str = 0; i_str < STUFFLIB_ARRAY_LEN(hello_utf8); ++i_str) {
    stufflib_data utf8_data = hello_utf8[i_str];
    size_t byte_pos = 0;
    while (byte_pos < utf8_data.size) {
      const char32_t expected_codepoint = decoded_strings[codepoint_pos];
      const size_t codepoint_width =
          stufflib_unicode_codepoint_width(utf8_data.size - byte_pos,
                                           utf8_data.data + byte_pos);
      if (expected_codepoint < 0x000080) {
        assert(codepoint_width == 1);
      } else if (expected_codepoint < 0x000800) {
        assert(codepoint_width == 2);
      } else if (expected_codepoint < 0x010000) {
        assert(codepoint_width == 3);
      } else if (expected_codepoint < 0x110000) {
        assert(codepoint_width == 4);
      } else {
        assert(codepoint_width == 0);
      }
      const char32_t codepoint =
          stufflib_unicode_codepoint_from_utf8(codepoint_width,
                                               utf8_data.data + byte_pos);
      assert(codepoint == expected_codepoint);
      ++codepoint_pos;
      byte_pos += codepoint_width;
    }
    assert(byte_pos == utf8_data.size);
  }
  return true;
}

bool test_unicode_iterator(const bool verbose) {
  size_t codepoint_pos = 0;
  for (size_t i_str = 0; i_str < STUFFLIB_ARRAY_LEN(hello_utf8); ++i_str) {
    stufflib_data utf8_data = hello_utf8[i_str];
    size_t str_len = 0;
    size_t byte_pos = 0;
    stufflib_iterator iter = stufflib_unicode_iter(hello_utf8 + i_str);
    for (; !stufflib_unicode_iter_end(&iter);
         stufflib_unicode_iter_advance(&iter)) {
      assert(iter.index == byte_pos);
      char32_t codepoint = 0;
      assert(stufflib_unicode_iter_get(&iter, &codepoint));
      assert(codepoint == decoded_strings[codepoint_pos]);
      const size_t codepoint_width =
          stufflib_unicode_codepoint_width(utf8_data.size - byte_pos,
                                           utf8_data.data + byte_pos);
      byte_pos += codepoint_width;
      ++str_len;
      ++codepoint_pos;
    }
    assert(str_len == decoded_lengths[i_str]);
    assert(str_len == iter.pos);
  }
  return true;
}

bool test_unicode_length(const bool verbose) {
  for (size_t i_str = 0; i_str < STUFFLIB_ARRAY_LEN(hello_utf8); ++i_str) {
    const size_t str_len = stufflib_unicode_length(hello_utf8 + i_str);
    assert(str_len == decoded_lengths[i_str]);
  }
  return true;
}

bool test_decode_utf8_files(const bool verbose) {
  const char* languages[] = {
      "ar", "bg",  "cs", "de",  "el", "fa", "fi", "fr",  "he",  "hi", "is",
      "ja", "ka",  "ki", "ko",  "ku", "lt", "lv", "nah", "nqo", "pl", "pt",
      "ru", "shi", "sl", "szl", "ta", "tr", "uk", "vep", "vi",  "zh",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(languages); ++i) {
    stufflib_data utf8_data = {
        .size = 0,
        .data = (unsigned char[20000]){0},
    };
    {
      char input_path[200] = {0};
      sprintf(input_path,
              "./test-data/txt/wikipedia/water_%s.txt",
              languages[i]);
      if (verbose) {
        printf("%s\n", input_path);
      }
      FILE* fp = fopen(input_path, "rb");
      if (!fp) {
        STUFFLIB_LOG_ERROR("cannot open %s", input_path);
        continue;
      }
      utf8_data.size = fread(utf8_data.data, 1, 20000, fp);
      fclose(fp);
      if (verbose) {
        printf("read %zu bytes\n", utf8_data.size);
      }
      assert(utf8_data.size > 0);
    }

    char32_t expected_codepoints[10000] = {0};
    size_t str_len = 0;
    {
      char codepoints_path[200] = {0};
      sprintf(codepoints_path,
              "./test-data/txt/wikipedia/water_%s_codepoints.txt",
              languages[i]);
      FILE* fp = fopen(codepoints_path, "rb");
      if (!fp) {
        STUFFLIB_LOG_ERROR("cannot open %s", codepoints_path);
        continue;
      }
      char buf[1000] = {0};
      while (fgets(buf, sizeof(buf), fp)) {
        expected_codepoints[str_len] = strtoul(buf, 0, 10);
        memset(buf, 0, sizeof(buf));
        ++str_len;
      }
      assert(feof(fp));
      fclose(fp);
    }

    size_t codepoint_pos = 0;
    for (stufflib_iterator iter = stufflib_unicode_iter(&utf8_data);
         !stufflib_unicode_iter_end(&iter);
         stufflib_unicode_iter_advance(&iter)) {
      assert(codepoint_pos < str_len);
      char32_t codepoint = 0;
      assert(stufflib_unicode_iter_get(&iter, &codepoint));
      assert(codepoint == expected_codepoints[codepoint_pos]);
      ++codepoint_pos;
    }
    assert(codepoint_pos == str_len);
  }
  return true;
}

STUFFLIB_TEST_MAIN(test_validate_utf8,
                   test_decode_codepoints,
                   test_unicode_iterator,
                   test_unicode_length,
                   test_decode_utf8_files)
