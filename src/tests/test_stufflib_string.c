#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

#include "_utf8_test_data.h"
#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_macros.h"
#include "stufflib_string.h"

bool test_string_init(const bool verbose) {
  for (size_t i = 0; i < SL_ARRAY_LEN(hello_utf8); ++i) {
    struct sl_string str = sl_string_from_utf8(hello_utf8 + i);
    assert(str.length == decoded_lengths[i]);
    const size_t data_size = str.utf8_data.size;
    assert(data_size == hello_utf8[i].size + 1);
    for (size_t c = 0; c < data_size - 1; ++c) {
      assert(str.utf8_data.data[c] == hello_utf8[i].data[c]);
    }
    assert(str.utf8_data.data[data_size - 1] == 0);
    sl_string_delete(&str);
  }
  return true;
}

bool test_string_utf8_view(const bool verbose) {
  struct sl_string empty_str = sl_string_from_utf8(&(struct sl_data){0});
  assert(empty_str.length == 0);
  assert(empty_str.utf8_data.size == 1);
  assert(empty_str.utf8_data.data[0] == 0);
  struct sl_data empty_view = sl_string_view_utf8_data(&empty_str);
  assert(empty_view.size == 0);
  assert(empty_view.data == nullptr);
  sl_string_delete(&empty_str);
  for (size_t i = 0; i < SL_ARRAY_LEN(hello_utf8); ++i) {
    struct sl_string str = sl_string_from_utf8(hello_utf8 + i);
    struct sl_data view = sl_string_view_utf8_data(&str);
    assert(view.size == hello_utf8[i].size);
    for (size_t c = 0; c < view.size; ++c) {
      assert(view.data[c] == hello_utf8[i].data[c]);
    }
    sl_string_delete(&str);
  }
  return true;
}

bool test_string_slice(const bool verbose) {
  size_t decoded_pos = 0;
  for (size_t i = 0; i < SL_ARRAY_LEN(hello_utf8); ++i) {
    struct sl_string str = sl_string_from_utf8(hello_utf8 + i);
    for (size_t substr_len = 0; substr_len <= str.length; ++substr_len) {
      for (size_t begin = 0; begin + substr_len <= str.length; ++begin) {
        struct sl_string substr =
            sl_string_slice(&str, begin, begin + substr_len);
        assert(substr.length == substr_len);
        struct sl_data view = sl_string_view_utf8_data(&substr);
        for (struct sl_iterator iter = sl_unicode_iter(&view);
             !iter.is_done(&iter);
             iter.advance(&iter)) {
          const char32_t codepoint = sl_unicode_iter_decode_item(&iter);
          assert(codepoint == decoded_strings[decoded_pos + begin + iter.pos]);
          // TODO assert iterator slice address equals underlying string
        }
        sl_string_delete(&substr);
      }
    }
    sl_string_delete(&str);
    decoded_pos += decoded_lengths[i];
  }
  return true;
}

bool test_init_from_file(const bool verbose) {
  const char* languages[] = {
      "ar", "bg",  "cs", "de",  "el", "fa", "fi", "fr",  "he",  "hi", "is",
      "ja", "ka",  "ki", "ko",  "ku", "lt", "lv", "nah", "nqo", "pl", "pt",
      "ru", "shi", "sl", "szl", "ta", "tr", "uk", "vep", "vi",  "zh",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(languages); ++i) {
    char input_path[200] = {0};
    sprintf(input_path, "./test-data/txt/wikipedia/water_%s.txt", languages[i]);
    if (verbose) {
      printf("%s\n", input_path);
    }

    char length_path[200] = {0};
    sprintf(length_path,
            "./test-data/txt/wikipedia/water_%s_length.txt",
            languages[i]);
    FILE* restrict fp = fopen(length_path, "rb");
    assert(fp);
    char tmp[200] = {0};
    assert(fread(tmp, 1, 200, fp));
    fclose(fp);
    const size_t expected_str_length = strtoull(tmp, 0, 10);

    struct sl_string str = sl_string_from_file(input_path);
    assert(str.length == expected_str_length);
    sl_string_delete(&str);
  }
  return true;
}

SL_TEST_MAIN(test_string_init,
             test_string_utf8_view,
             test_string_slice,
             test_init_from_file)
