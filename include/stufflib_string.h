#ifndef _STUFFLIB_STRING_H_INCLUDED
#define _STUFFLIB_STRING_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_unicode.h"

typedef struct stufflib_string stufflib_string;
struct stufflib_string {
  size_t length;
  stufflib_data utf8_data;
};

stufflib_string stufflib_string_from_utf8(
    const stufflib_data utf8_data[const static 1]) {
  if (!stufflib_unicode_is_valid_utf8(utf8_data)) {
    STUFFLIB_LOG_ERROR("cannot create string from invalid UTF-8 data");
    return (stufflib_string){0};
  }
  return (stufflib_string){
      .length = stufflib_unicode_length(utf8_data),
      .utf8_data = *utf8_data,
  };
}

stufflib_string stufflib_string_slice(const stufflib_string str[const static 1],
                                      const size_t begin,
                                      const size_t end) {
  stufflib_iterator begin_iter = stufflib_unicode_iter(&(str->utf8_data));
  while (begin_iter.pos < begin && !stufflib_unicode_iter_end(&begin_iter)) {
    stufflib_unicode_iter_advance(&begin_iter);
  }
  stufflib_iterator end_iter = begin_iter;
  while (end_iter.pos < end && !stufflib_unicode_iter_end(&end_iter)) {
    stufflib_unicode_iter_advance(&end_iter);
  }
  stufflib_data utf8_slice =
      stufflib_data_slice(&(str->utf8_data), begin_iter.index, end_iter.index);
  return stufflib_string_from_utf8(&utf8_slice);
}

stufflib_string stufflib_string_strstr(
    const stufflib_string str[const static 1],
    const stufflib_string substr[const static 1]) {
  for (stufflib_iterator str_iter = stufflib_unicode_iter(&(str->utf8_data));
       !stufflib_unicode_iter_end(&str_iter);
       stufflib_unicode_iter_advance(&str_iter)) {
    stufflib_iterator lhs = str_iter;
    stufflib_iterator rhs = stufflib_unicode_iter(&(substr->utf8_data));
    bool match = true;
    while (match) {
      if (stufflib_unicode_iter_end(&rhs)) {
        break;
      }
      if (stufflib_unicode_iter_end(&lhs)) {
        match = false;
        break;
      }
      const wchar_t lhs_item = stufflib_unicode_iter_decode_item(&lhs);
      const wchar_t rhs_item = stufflib_unicode_iter_decode_item(&rhs);
      match = lhs_item && rhs_item && lhs_item == rhs_item;
      stufflib_unicode_iter_advance(&lhs);
      stufflib_unicode_iter_advance(&rhs);
    }
    if (match) {
      stufflib_data utf8_slice =
          stufflib_data_slice(&(str->utf8_data), str_iter.index, SIZE_MAX);
      return stufflib_string_from_utf8(&utf8_slice);
    }
  }
  return (stufflib_string){0};
}

void stufflib_string_fprint(FILE stream[const static 1],
                            const stufflib_string str[const static 1]) {
  for (stufflib_iterator iter = stufflib_unicode_iter(&(str->utf8_data));
       !stufflib_unicode_iter_end(&iter);
       stufflib_unicode_iter_advance(&iter)) {
    const wchar_t item = stufflib_unicode_iter_decode_item(&iter);
    if (fwprintf(stream, L"%lc", item) < 0) {
      exit(1);
    }
  }
}

#endif  // _STUFFLIB_STRING_H_INCLUDED
