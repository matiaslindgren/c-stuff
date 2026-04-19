#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/errno.h>

#include <stufflib/context/context.h>
#include <stufflib/io/io.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>

bool sl_io_read(
    struct sl_context ctx[static const 1],
    const char path[static const 1],
    unsigned char out[static const 1],
    size_t count
) {
  FILE* file = nullptr;
  bool ok    = true;

  if (!(file = fopen(path, "r"))) {
    SL_ERROR(ctx, "cannot open %s, ERRNO=%d", path, errno);
    goto end;
  }

  if (count != fread(out, 1, count, file)) {
    SL_ERROR(ctx, "failed reading %zu bytes from %s", count, path);
    goto end;
  }

end:
  if (file) {
    fclose(file);
  }
  return ok;
}

bool sl_file_format_path(
    const size_t bufsize,
    char buffer[const bufsize],
    const char path[const static 1],
    const char name[const static 1],
    const char suffix[const static 1]
) {
  memset(buffer, 0, bufsize);
  if (path[0] && name[0] && 3 <= snprintf(buffer, bufsize, "%s/%s%s", path, name, suffix)) {
    return true;
  }
  return false;
}

bool sl_file_open(
    struct sl_context ctx[static 1],
    struct sl_file f[const static 1],
    const char path[const static 1],
    const char mode[const static 1]
) {
  f->file = fopen(path, mode);
  if (!f->file) {
    SL_ERROR(ctx, "cannot open '%s'", path);
    return false;
  }
  memset(f->path, 0, SL_ARRAY_LEN(f->path));
  strncpy(f->path, path, SL_ARRAY_LEN(f->path));
  return true;
}

void sl_file_close(struct sl_file f[const static 1]) {
  if (f->file) {
    fclose(f->file);
    *f = (struct sl_file){0};
  }
}

size_t sl_file_read(
    struct sl_context ctx[static 1],
    struct sl_file f[const static 1],
    struct sl_span buffer[const static 1]
) {
  const size_t nread = fread(buffer->data, sizeof(unsigned char), buffer->size, f->file);
  if (ferror(f->file)) {
    SL_ERROR(ctx, "failed reading %zu bytes from '%s'", buffer->size, f->path);
    return 0;
  }
  return nread;
}

size_t sl_file_parse_int64(
    struct sl_context ctx[static 1],
    struct sl_file f[const static 1],
    const size_t count,
    int64_t buffer[const count]
) {
  size_t pos = 0;
  for (; pos < count && sl_file_can_read(f); ++pos) {
    if (EOF == fscanf(f->file, " %" PRId64, buffer + pos)) {
      SL_ERROR(ctx, "failed parsing int64 at index %zu from '%s'", pos, f->path);
      goto done;
    }
  }
done:
  return pos;
}
