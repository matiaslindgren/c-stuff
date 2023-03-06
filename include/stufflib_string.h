#ifndef SL_STRING_H_INCLUDED
#define SL_STRING_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_data.h"
#include "stufflib_io.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_unicode.h"

struct sl_string {
  size_t length;
  struct sl_data utf8_data;
};

void sl_string_delete(struct sl_string string[static 1]) {
  sl_data_delete(&(string->utf8_data));
  *string = (struct sl_string){0};
}

struct sl_string sl_string_from_utf8(
    const struct sl_data utf8_data[const static 1]) {
  if (!sl_unicode_is_valid_utf8(utf8_data)) {
    SL_LOG_ERROR("UTF-8 decode error, cannot initialize string");
    return (struct sl_string){0};
  }
  const struct sl_data terminator = sl_data_view(1, (unsigned char[]){0});
  return (struct sl_string){
      .length = sl_unicode_length(utf8_data),
      .utf8_data = sl_data_concat(utf8_data, &terminator),
  };
}

struct sl_data sl_string_view_utf8_data(
    const struct sl_string str[const static 1]) {
  return sl_data_slice(&(str->utf8_data), 0, str->utf8_data.size - 1);
}

struct sl_string sl_string_from_file(const char filename[const static 1]) {
  struct sl_iterator file_iter = sl_file_iter_open(filename);

  struct sl_data utf8_data = {0};
  for (; !file_iter.is_done(&file_iter); file_iter.advance(&file_iter)) {
    struct sl_data* buffer = file_iter.get_item(&file_iter);
    sl_data_extend(&utf8_data, buffer);
  }

  sl_file_iter_close(&file_iter);

  if (!sl_unicode_is_valid_utf8(&utf8_data)) {
    SL_LOG_ERROR("cannot decode '%s' as UTF-8", filename);
    sl_data_delete(&utf8_data);
    return (struct sl_string){0};
  }

  struct sl_string file_content = sl_string_from_utf8(&utf8_data);
  sl_data_delete(&utf8_data);

  return file_content;
}

struct sl_string sl_string_concat(const struct sl_string str1[const static 1],
                                  const struct sl_string str2[const static 1]) {
  const struct sl_data str1_data = sl_string_view_utf8_data(str1);
  const struct sl_data str2_data = sl_string_view_utf8_data(str2);
  struct sl_data data = sl_data_concat(&str1_data, &str2_data);
  struct sl_string result = sl_string_from_utf8(&data);
  sl_data_delete(&data);
  return result;
}

void sl_string_extend(struct sl_string str1[const static 1],
                      const struct sl_string str2[const static 1]) {
  struct sl_string result = sl_string_concat(str1, str2);
  sl_string_delete(str1);
  *str1 = result;
}

struct sl_string sl_string_slice(const struct sl_string str[const static 1],
                                 const size_t begin,
                                 const size_t end) {
  const struct sl_data utf8_data = sl_string_view_utf8_data(str);
  struct sl_iterator begin_iter = sl_unicode_iter(&utf8_data);
  while (begin_iter.pos < begin && !begin_iter.is_done(&begin_iter)) {
    begin_iter.advance(&begin_iter);
  }
  struct sl_iterator end_iter = begin_iter;
  while (end_iter.pos < end && !end_iter.is_done(&end_iter)) {
    end_iter.advance(&end_iter);
  }
  struct sl_data utf8_slice =
      sl_data_slice(&(str->utf8_data), begin_iter.index, end_iter.index);
  return sl_string_from_utf8(&utf8_slice);
}

bool sl_string_fprint(FILE stream[const static 1],
                      const struct sl_string str[const static 1]) {
  const struct sl_data utf8_data = sl_string_view_utf8_data(str);
  return fwrite(utf8_data.data, 1, utf8_data.size, stream) == utf8_data.size;
}

#endif  // SL_STRING_H_INCLUDED
