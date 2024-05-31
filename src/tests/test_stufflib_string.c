#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"
#include "stufflib_span.h"
#include "stufflib_string.h"
#include "stufflib_test_data.h"

bool test_string_init(const bool) {
  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_hello_utf8); ++i) {
    struct sl_string str = sl_string_from_utf8(sl_test_data_hello_utf8 + i);
    assert(str.length == sl_test_data_decoded_lengths[i]);
    const size_t data_size = str.utf8_data.size;
    assert(data_size == sl_test_data_hello_utf8[i].size + 1);
    for (size_t c = 0; c < data_size - 1; ++c) {
      assert(str.utf8_data.data[c] == sl_test_data_hello_utf8[i].data[c]);
    }
    assert(str.utf8_data.data[data_size - 1] == 0);
    sl_string_delete(&str);
  }
  return true;
}

bool test_string_utf8_view(const bool) {
  struct sl_string empty_str = sl_string_from_utf8(&(struct sl_span){0});
  assert(empty_str.length == 0);
  assert(empty_str.utf8_data.size == 1);
  assert(empty_str.utf8_data.data[0] == 0);
  struct sl_span empty_view = sl_string_view_utf8_data(&empty_str);
  assert(empty_view.size == 0);
  assert(empty_view.data == nullptr);
  sl_string_delete(&empty_str);
  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_hello_utf8); ++i) {
    struct sl_string str = sl_string_from_utf8(sl_test_data_hello_utf8 + i);
    struct sl_span view = sl_string_view_utf8_data(&str);
    assert(view.size == sl_test_data_hello_utf8[i].size);
    for (size_t c = 0; c < view.size; ++c) {
      assert(view.data[c] == sl_test_data_hello_utf8[i].data[c]);
    }
    sl_string_delete(&str);
  }
  return true;
}

bool test_string_slice(const bool) {
  size_t decoded_pos = 0;
  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_hello_utf8); ++i) {
    struct sl_string str = sl_string_from_utf8(sl_test_data_hello_utf8 + i);
    for (size_t substr_len = 0; substr_len <= str.length; ++substr_len) {
      for (size_t begin = 0; begin + substr_len <= str.length; ++begin) {
        struct sl_string substr =
            sl_string_slice(&str, begin, begin + substr_len);
        assert(substr.length == substr_len);
        struct sl_span view = sl_string_view_utf8_data(&substr);
        for (struct sl_iterator iter = sl_unicode_iter(&view);
             !sl_unicode_iter_is_done(&iter);
             sl_unicode_iter_advance(&iter)) {
          const uint32_t codepoint = sl_unicode_iter_decode_item(&iter);
          assert(codepoint ==
                 sl_test_data_decoded_strings[decoded_pos + begin + iter.pos]);
          // TODO assert iterator slice address equals underlying string
        }
        sl_string_delete(&substr);
      }
    }
    sl_string_delete(&str);
    decoded_pos += sl_test_data_decoded_lengths[i];
  }
  return true;
}

SL_TEST_MAIN(test_string_init, test_string_utf8_view, test_string_slice)
