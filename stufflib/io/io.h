#ifndef SL_IO_H_INCLUDED
#define SL_IO_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stufflib/context/context.h>
#include <stufflib/span/span.h>

struct sl_file {
  char path[1'024];
  FILE* file;
};

bool sl_io_read(
    struct sl_context ctx[static 1],
    const char path[static 1],
    unsigned char out[static 1],
    size_t count
);
bool sl_file_format_path(
    size_t bufsize,
    char buffer[const bufsize],
    const char path[const static 1],
    const char name[const static 1],
    const char suffix[const static 1]
);
bool sl_file_open(
    struct sl_context ctx[static 1],
    struct sl_file f[const static 1],
    const char path[const static 1],
    const char mode[const static 1]
);
void sl_file_close(struct sl_file f[const static 1]);
size_t sl_file_read(
    struct sl_context ctx[static 1],
    struct sl_file f[const static 1],
    struct sl_span buffer[const static 1]
);

static inline bool sl_file_can_read(struct sl_file f[const static 1]) {
  return (f->file && !feof(f->file) && ferror(f->file) == 0) != 0;
}
size_t sl_file_parse_int64(
    struct sl_context ctx[static 1],
    struct sl_file f[const static 1],
    size_t count,
    int64_t buffer[const count]
);

#endif  // SL_IO_H_INCLUDED
