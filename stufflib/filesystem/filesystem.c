#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stufflib/context/context.h>
#include <stufflib/filesystem/filesystem.h>
#include <stufflib/io/io.h>
#include <stufflib/span/span.h>
#include <stufflib/string/string.h>
#include <stufflib/unicode/unicode.h>

struct sl_span sl_fs_read_file(
    struct sl_context ctx[static 1],
    const char path[const static 1],
    struct sl_span buffer[const static 1]
) {
  struct sl_file f    = {0};
  struct sl_span data = {0};
  if (!sl_file_open(ctx, &f, path, "rb")) {
    goto done;
  }
  // TODO try seeking to end to get exact size,
  // fall back to incremental dynamic alloc if stdin
  if (!sl_span_create(ctx, 0, &data)) {
    SL_ERROR(ctx, "failed creating buffer");
    goto done;
  }
  for (size_t n_read = 0; sl_file_can_read(&f) && (n_read = sl_file_read(ctx, &f, buffer));) {
    struct sl_span chunk = sl_span_view(n_read, buffer->data);
    if (!sl_span_extend(ctx, &data, &chunk)) {
      SL_ERROR(ctx, "failed reading '%s'", path);
      goto done;
    }
  }
done:
  sl_file_close(&f);
  return data;
}

struct sl_string sl_fs_read_file_utf8(
    struct sl_context ctx[static 1],
    const char path[const static 1],
    struct sl_span buffer[const static 1]
) {
  struct sl_span utf8_data = sl_fs_read_file(ctx, path, buffer);

  if (!sl_unicode_is_valid_utf8(&utf8_data)) {
    SL_ERROR(ctx, "cannot decode '%s' as UTF-8", path);
    sl_span_destroy(&utf8_data);
    return (struct sl_string){0};
  }

  struct sl_string content = {0};
  if (!sl_string_from_utf8(ctx, &utf8_data, &content)) {
    sl_span_destroy(&utf8_data);
    return (struct sl_string){0};
  }
  sl_span_destroy(&utf8_data);
  return content;
}

bool sl_fs_read_int64(
    struct sl_context ctx[static 1],
    const char path[const static 1],
    const size_t count,
    int64_t values[const count]
) {
  bool ok          = false;
  struct sl_file f = {0};
  if (!sl_file_open(ctx, &f, path, "rb")) {
    goto done;
  }
  if (count != sl_file_parse_int64(ctx, &f, count, values)) {
    goto done;
  }
  ok = true;
done:
  sl_file_close(&f);
  return ok;
}
