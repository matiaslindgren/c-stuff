#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./test_data.h"
#include "stufflib/args/args.h"
#include "stufflib/iterator/iterator.h"
#include "stufflib/macros/macros.h"
#include "stufflib/span/span.h"
#include "stufflib/unicode/unicode.h"

static bool test_validate_utf8(const bool) {
  struct sl_span invalid_utf8[] = {
      {.size = 1,                   .data = (unsigned char[]){0x80}},
      {.size = 2,             .data = (unsigned char[]){0xc0, 0x80}},
      {.size = 2,             .data = (unsigned char[]){0xe0, 0x80}},
      {.size = 2,             .data = (unsigned char[]){0xf0, 0x80}},
      {.size = 3,       .data = (unsigned char[]){0xf0, 0x80, 0x80}},
      {.size = 4, .data = (unsigned char[]){0xff, 0x80, 0x80, 0x80}},
      {.size = 5,       .data = (unsigned char[]){0, 1, 2, 0xf4, 0}},
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(invalid_utf8); ++i) {
    assert(!sl_unicode_is_valid_utf8(invalid_utf8 + i));
  }
  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_hello_utf8); ++i) {
    assert(sl_unicode_is_valid_utf8(sl_test_data_hello_utf8 + i));
  }
  return true;
}

static bool test_decode_codepoints(const bool) {
  size_t codepoint_pos = 0;
  for (size_t i_str = 0; i_str < SL_ARRAY_LEN(sl_test_data_hello_utf8);
       ++i_str) {
    struct sl_span utf8_data = sl_test_data_hello_utf8[i_str];
    size_t byte_pos = 0;
    while (byte_pos < utf8_data.size) {
      const uint32_t expected_codepoint =
          sl_test_data_decoded_strings[codepoint_pos];
      const size_t codepoint_width =
          sl_unicode_codepoint_width_from_utf8(utf8_data.size - byte_pos,
                                               utf8_data.data + byte_pos);
      assert(codepoint_width == sl_unicode_codepoint_width(expected_codepoint));
      const uint32_t codepoint =
          sl_unicode_codepoint_from_utf8(codepoint_width,
                                         utf8_data.data + byte_pos);
      assert(codepoint == expected_codepoint);
      ++codepoint_pos;
      byte_pos += codepoint_width;
    }
    assert(byte_pos == utf8_data.size);
  }
  return true;
}

static bool test_unicode_iterator(const bool) {
  size_t codepoint_pos = 0;
  for (size_t i_str = 0; i_str < SL_ARRAY_LEN(sl_test_data_hello_utf8);
       ++i_str) {
    struct sl_span utf8_data = sl_test_data_hello_utf8[i_str];
    size_t str_len = 0;
    size_t byte_pos = 0;
    struct sl_iterator iter = sl_unicode_iter(sl_test_data_hello_utf8 + i_str);
    for (; !sl_unicode_iter_is_done(&iter); sl_unicode_iter_advance(&iter)) {
      assert(iter.index == byte_pos);
      const uint32_t codepoint = sl_unicode_iter_decode_item(&iter);
      assert(codepoint == sl_test_data_decoded_strings[codepoint_pos]);
      const size_t codepoint_width =
          sl_unicode_codepoint_width_from_utf8(utf8_data.size - byte_pos,
                                               utf8_data.data + byte_pos);
      byte_pos += codepoint_width;
      ++str_len;
      ++codepoint_pos;
    }
    assert(str_len == sl_test_data_decoded_lengths[i_str]);
    assert(str_len == iter.pos);
  }
  return true;
}

static bool test_unicode_length(const bool) {
  for (size_t i_str = 0; i_str < SL_ARRAY_LEN(sl_test_data_hello_utf8);
       ++i_str) {
    const size_t str_len = sl_unicode_length(sl_test_data_hello_utf8 + i_str);
    assert(str_len == sl_test_data_decoded_lengths[i_str]);
  }
  return true;
}

static bool test_decode_utf8_files(const bool verbose) {
  const char* languages[] = {
      "ar", "bg",  "cs", "de",  "el", "fa", "fi", "fr",  "he",  "hi", "is",
      "ja", "ka",  "ki", "ko",  "ku", "lt", "lv", "nah", "nqo", "pl", "pt",
      "ru", "shi", "sl", "szl", "ta", "tr", "uk", "vep", "vi",  "zh",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(languages); ++i) {
    struct sl_span utf8_data = {
        .size = 0,
        .data = (unsigned char[20000]){0},
    };
    {
      char input_path[200] = {0};
      snprintf(input_path,
               SL_ARRAY_LEN(input_path),
               "./test-data/txt/wikipedia/water_%s.txt",
               languages[i]);
      if (verbose) {
        printf("%s\n", input_path);
      }
      FILE* fp = fopen(input_path, "rb");
      if (!fp) {
        SL_LOG_ERROR("cannot open %s", input_path);
        continue;
      }
      utf8_data.size = fread(utf8_data.data, 1, 20000, fp);
      fclose(fp);
      if (verbose) {
        printf("read %zu bytes\n", utf8_data.size);
      }
      assert(utf8_data.size > 0);
    }

    uint32_t expected_codepoints[10000] = {0};
    size_t str_len = 0;
    {
      char codepoints_path[200] = {0};
      snprintf(codepoints_path,
               SL_ARRAY_LEN(codepoints_path),
               "./test-data/txt/wikipedia/water_%s_codepoints.txt",
               languages[i]);
      FILE* fp = fopen(codepoints_path, "rb");
      if (!fp) {
        SL_LOG_ERROR("cannot open %s", codepoints_path);
        continue;
      }
      char buf[1000] = {0};
      while (fgets(buf, sizeof(buf), fp)) {
        expected_codepoints[str_len] = (uint32_t)strtoul(buf, 0, 10);
        memset(buf, 0, sizeof(buf));
        ++str_len;
      }
      assert(feof(fp));
      fclose(fp);
    }

    size_t codepoint_pos = 0;
    for (struct sl_iterator iter = sl_unicode_iter(&utf8_data);
         !sl_unicode_iter_is_done(&iter);
         sl_unicode_iter_advance(&iter)) {
      assert(codepoint_pos < str_len);
      const uint32_t codepoint = sl_unicode_iter_decode_item(&iter);
      assert(codepoint == expected_codepoints[codepoint_pos]);
      ++codepoint_pos;
    }
    assert(codepoint_pos == str_len);
  }
  return true;
}

SL_TEST_MAIN(test_validate_utf8,
             test_decode_codepoints,
             test_unicode_iterator,
             test_unicode_length,
             test_decode_utf8_files)
