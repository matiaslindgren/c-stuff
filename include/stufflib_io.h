#ifndef SL_IO_H_INCLUDED
#define SL_IO_H_INCLUDED
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"
#include "stufflib_misc.h"
#include "stufflib_span.h"

struct sl_file {
  char path[1024];
  FILE* file;
};

bool sl_file_format_path(const size_t bufsize,
                         char buffer[const bufsize],
                         const char path[const static 1],
                         const char name[const static 1],
                         const char suffix[const static 1]) {
  memset(buffer, 0, bufsize);
  if (path[0] && name[0] &&
      3 <= snprintf(buffer, bufsize, "%s/%s%s", path, name, suffix)) {
    return true;
  }
  return false;
}

bool sl_file_open(struct sl_file f[const static 1],
                  const char path[const static 1],
                  const char mode[const static 1]) {
  f->file = fopen(path, mode);
  if (!f->file) {
    // TODO no logging from base libraries
    SL_LOG_ERROR("cannot open '%s'", path);
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

static inline bool sl_file_can_read(struct sl_file f[const static 1]) {
  return f->file && !feof(f->file) && ferror(f->file) == 0;
}

size_t sl_file_read(struct sl_file f[const static 1],
                    struct sl_span buffer[const static 1]) {
  const size_t nread =
      fread(buffer->data, sizeof(unsigned char), buffer->size, f->file);
  if (ferror(f->file)) {
    SL_LOG_ERROR("failed reading %zu bytes from '%s'", buffer->size, f->path);
    return 0;
  }
  return nread;
}

size_t sl_file_parse_int64(struct sl_file f[const static 1],
                           const size_t count,
                           int64_t buffer[const count]) {
  size_t pos = 0;
  for (; pos < count && sl_file_can_read(f); ++pos) {
    if (EOF == fscanf(f->file, " %" PRId64, buffer + pos)) {
      SL_LOG_ERROR("failed parsing int64 at index %zu from '%s'", pos, f->path);
      goto done;
    }
  }
done:
  return pos;
}

#endif  // SL_IO_H_INCLUDED
