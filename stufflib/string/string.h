#ifndef SL_STRING_H_INCLUDED
#define SL_STRING_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>

#include "stufflib/iterator/iterator.h"
#include "stufflib/macros/macros.h"
#include "stufflib/span/span.h"
#include "stufflib/unicode/unicode.h"

struct sl_string {
  size_t length;
  struct sl_span utf8_data;
};

void sl_string_destroy(struct sl_string string[static 1]) {
  sl_span_destroy(&(string->utf8_data));
  *string = (struct sl_string){0};
}

struct sl_string sl_string_from_utf8(struct sl_span utf8_data[const static 1]) {
  if (!sl_unicode_is_valid_utf8(utf8_data)) {
    SL_LOG_ERROR("UTF-8 decode error, cannot initialize string");
    return (struct sl_string){0};
  }
  struct sl_span terminator = sl_span_view(1, (unsigned char[]){0});
  return (struct sl_string){
      .length = sl_unicode_length(utf8_data),
      .utf8_data = sl_span_concat(utf8_data, &terminator),
  };
}

struct sl_span sl_string_view_utf8_data(struct sl_string str[const static 1]) {
  return sl_span_slice(&(str->utf8_data), 0, str->utf8_data.size - 1);
}

bool sl_string_is_ascii(const struct sl_string str[const static 1]) {
  for (size_t i = 0; i < str->utf8_data.size; ++i) {
    if (sl_unicode_codepoint_width(str->utf8_data.data[i]) != 1) {
      return false;
    }
  }
  return true;
}

void sl_string_copy_ascii(char dst[const static 1],
                          const struct sl_string str[const static 1]) {
  if (str->utf8_data.size > 0) {
    memcpy(dst, str->utf8_data.data, str->utf8_data.size);
  }
}

struct sl_string sl_string_concat(struct sl_string str1[const static 1],
                                  struct sl_string str2[const static 1]) {
  struct sl_span str1_data = sl_string_view_utf8_data(str1);
  struct sl_span str2_data = sl_string_view_utf8_data(str2);
  struct sl_span data = sl_span_concat(&str1_data, &str2_data);
  struct sl_string result = sl_string_from_utf8(&data);
  sl_span_destroy(&data);
  return result;
}

void sl_string_extend(struct sl_string str1[const static 1],
                      struct sl_string str2[const static 1]) {
  struct sl_string result = sl_string_concat(str1, str2);
  sl_string_destroy(str1);
  *str1 = result;
}

struct sl_string sl_string_slice(struct sl_string str[const static 1],
                                 size_t begin,
                                 size_t end) {
  struct sl_span utf8_data = sl_string_view_utf8_data(str);
  struct sl_iterator begin_iter = sl_unicode_iter(&utf8_data);
  while (begin_iter.pos < begin && !sl_unicode_iter_is_done(&begin_iter)) {
    sl_unicode_iter_advance(&begin_iter);
  }
  struct sl_iterator end_iter = begin_iter;
  while (end_iter.pos < end && !sl_unicode_iter_is_done(&end_iter)) {
    sl_unicode_iter_advance(&end_iter);
  }
  struct sl_span utf8_slice =
      sl_span_slice(&(str->utf8_data), begin_iter.index, end_iter.index);
  return sl_string_from_utf8(&utf8_slice);
}

bool sl_string_fprint(FILE stream[const static 1],
                      struct sl_string str[const static 1]) {
  struct sl_span utf8_data = sl_string_view_utf8_data(str);
  return fwrite(utf8_data.data, 1, utf8_data.size, stream) == utf8_data.size;
}

#endif  // SL_STRING_H_INCLUDED
