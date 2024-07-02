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

struct sl_span sl_fs_read_file(const char path[const static 1],
                               struct sl_span buffer[const static 1]) {
  struct sl_file f = {0};
  struct sl_span data = {0};
  if (!sl_file_open(&f, path, "rb")) {
    goto done;
  }
  // TODO try seeking to end to get exact size,
  // fall back to incremental dynamic alloc if stdin
  data = sl_span_create(0);
  for (size_t n_read = 0;
       sl_file_can_read(&f) && (n_read = sl_file_read(&f, buffer));) {
    struct sl_span chunk = sl_span_view(n_read, buffer->data);
    sl_span_extend(&data, &chunk);
  }
done:
  sl_file_close(&f);
  return data;
}

struct sl_string sl_fs_read_file_utf8(const char path[const static 1],
                                      struct sl_span buffer[const static 1]) {
  struct sl_span utf8_data = sl_fs_read_file(path, buffer);

  if (!sl_unicode_is_valid_utf8(&utf8_data)) {
    SL_LOG_ERROR("cannot decode '%s' as UTF-8", path);
    sl_span_delete(&utf8_data);
    return (struct sl_string){0};
  }

  struct sl_string content = sl_string_from_utf8(&utf8_data);
  sl_span_delete(&utf8_data);

  return content;
}

bool sl_fs_read_int64(const char path[const static 1],
                      const size_t count,
                      int64_t values[const count]) {
  bool ok = false;
  struct sl_file f = {0};
  if (!sl_file_open(&f, path, "rb")) {
    goto done;
  }
  if (count != sl_file_parse_int64(&f, count, values)) {
    goto done;
  }
  ok = true;
done:
  sl_file_close(&f);
  return ok;
}

#endif  // SL_FILESYSTEM_H_INCLUDED
