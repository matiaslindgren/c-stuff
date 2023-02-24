#ifndef _STUFFLIB_STRING_H_INCLUDED
#define _STUFFLIB_STRING_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "stufflib_data.h"
#include "stufflib_io.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_unicode.h"

typedef struct stufflib_string stufflib_string;
struct stufflib_string {
  size_t length;
  stufflib_data utf8_data;
};

void stufflib_string_delete(stufflib_string string[static 1]) {
  stufflib_data_delete(&(string->utf8_data));
  *string = (stufflib_string){0};
}

stufflib_string stufflib_string_from_utf8(
    const stufflib_data utf8_data[const static 1]) {
  if (!stufflib_unicode_is_valid_utf8(utf8_data)) {
    STUFFLIB_LOG_ERROR("UTF-8 decode error, cannot initialize string");
    return (stufflib_string){0};
  }
  const stufflib_data terminator = stufflib_data_view(1, (unsigned char[]){0});
  return (stufflib_string){
      .length = stufflib_unicode_length(utf8_data),
      .utf8_data = stufflib_data_concat(utf8_data, &terminator),
  };
}

stufflib_data stufflib_string_view_utf8_data(
    const stufflib_string str[const static 1]) {
  return stufflib_data_slice(&(str->utf8_data), 0, str->utf8_data.size - 1);
}

stufflib_string stufflib_string_from_file(const char filename[const static 1]) {
  stufflib_data utf8_data = (stufflib_data){0};
  stufflib_iterator file_iter = stufflib_file_iter_open(filename);
  for (; !file_iter.is_done(&file_iter); file_iter.advance(&file_iter)) {
    stufflib_data* buffer = file_iter.get_item(&file_iter);
    stufflib_data_extend(&utf8_data, buffer);
  }
  stufflib_file_iter_close(&file_iter);
  if (!stufflib_unicode_is_valid_utf8(&utf8_data)) {
    STUFFLIB_LOG_ERROR("cannot decode '%s' as UTF-8", filename);
    stufflib_data_delete(&utf8_data);
    return (stufflib_string){0};
  }
  stufflib_string file_content = stufflib_string_from_utf8(&utf8_data);
  stufflib_data_delete(&utf8_data);
  return file_content;
}

stufflib_string stufflib_string_concat(
    const stufflib_string str1[const static 1],
    const stufflib_string str2[const static 1]) {
  const stufflib_data str1_data = stufflib_string_view_utf8_data(str1);
  const stufflib_data str2_data = stufflib_string_view_utf8_data(str2);
  const stufflib_data data = stufflib_data_concat(&str1_data, &str2_data);
  return stufflib_string_from_utf8(&data);
}

void stufflib_string_extend(stufflib_string str1[const static 1],
                            const stufflib_string str2[const static 1]) {
  stufflib_string result = stufflib_string_concat(str1, str2);
  stufflib_string_delete(str1);
  *str1 = result;
}

stufflib_string stufflib_string_slice(const stufflib_string str[const static 1],
                                      const size_t begin,
                                      const size_t end) {
  const stufflib_data utf8_data = stufflib_string_view_utf8_data(str);
  stufflib_iterator begin_iter = stufflib_unicode_iter(&utf8_data);
  while (begin_iter.pos < begin && !begin_iter.is_done(&begin_iter)) {
    begin_iter.advance(&begin_iter);
  }
  stufflib_iterator end_iter = begin_iter;
  while (end_iter.pos < end && !end_iter.is_done(&end_iter)) {
    end_iter.advance(&end_iter);
  }
  stufflib_data utf8_slice =
      stufflib_data_slice(&(str->utf8_data), begin_iter.index, end_iter.index);
  return stufflib_string_from_utf8(&utf8_slice);
}

stufflib_string stufflib_string_strstr(
    const stufflib_string str[const static 1],
    const stufflib_string substr[const static 1]) {
  const stufflib_data str_utf8_data = stufflib_string_view_utf8_data(str);
  const stufflib_data substr_utf8_data = stufflib_string_view_utf8_data(substr);
  for (stufflib_iterator str_iter = stufflib_unicode_iter(&str_utf8_data);
       !str_iter.is_done(&str_iter);
       str_iter.advance(&str_iter)) {
    stufflib_iterator lhs = str_iter;
    stufflib_iterator rhs = stufflib_unicode_iter(&substr_utf8_data);
    bool match = true;
    while (match) {
      if (rhs.is_done(&rhs)) {
        break;
      }
      if (lhs.is_done(&lhs)) {
        match = false;
        break;
      }
      const wchar_t lhs_item = stufflib_unicode_iter_decode_item(&lhs);
      const wchar_t rhs_item = stufflib_unicode_iter_decode_item(&rhs);
      match = lhs_item && rhs_item && lhs_item == rhs_item;
      lhs.advance(&lhs);
      rhs.advance(&rhs);
    }
    if (match) {
      stufflib_data utf8_slice =
          stufflib_data_slice(&(str->utf8_data), str_iter.index, SIZE_MAX);
      return stufflib_string_from_utf8(&utf8_slice);
    }
  }
  return (stufflib_string){0};
}

int stufflib_string_fprint(FILE stream[const static 1],
                           const stufflib_string str[const static 1],
                           const wchar_t separator[const static 1]) {
  const stufflib_data utf8_data = stufflib_string_view_utf8_data(str);
  for (stufflib_iterator iter = stufflib_unicode_iter(&utf8_data);
       !iter.is_done(&iter);
       iter.advance(&iter)) {
    const wchar_t item = stufflib_unicode_iter_decode_item(&iter);
    const int ret = fwprintf(stream, L"%lc%ls", item, separator);
    if (ret < 0) {
      return ret;
    }
  }
  return 0;
}

#endif  // _STUFFLIB_STRING_H_INCLUDED
