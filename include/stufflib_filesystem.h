#ifndef SL_FILESYSTEM_H_INCLUDED
#define SL_FILESYSTEM_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_io.h"
#include "stufflib_macros.h"
#include "stufflib_span.h"
#include "stufflib_string.h"
#include "stufflib_tokenizer.h"

struct sl_span sl_fs_read_file(const char filename[const static 1]) {
  struct sl_iterator file_iter = sl_file_iter_open(filename);
  // TODO try seeking to end to get exact size,
  // fall back to incremental dynamic alloc if stdin
  struct sl_span data = sl_span_create(0);
  for (; !sl_file_iter_is_done(&file_iter); sl_file_iter_advance(&file_iter)) {
    struct sl_span* buffer = sl_file_iter_get(&file_iter);
    sl_span_extend(&data, buffer);
  }
  sl_file_iter_close(&file_iter);
  return data;
}

struct sl_string sl_fs_read_file_utf8(const char filename[const static 1]) {
  struct sl_span utf8_data = sl_fs_read_file(filename);

  if (!sl_unicode_is_valid_utf8(&utf8_data)) {
    SL_LOG_ERROR("cannot decode '%s' as UTF-8", filename);
    sl_span_delete(&utf8_data);
    return (struct sl_string){0};
  }

  struct sl_string content = sl_string_from_utf8(&utf8_data);
  sl_span_delete(&utf8_data);

  return content;
}

#endif  // SL_FILESYSTEM_H_INCLUDED
