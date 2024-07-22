#ifndef SL_IO_H_INCLUDED
#define SL_IO_H_INCLUDED
#include <stdint.h>
#include <stdio.h>

#include "../io/io.h"
#include "../span/span.h"

struct sl_file {
  char path[1024];
  FILE* file;
};

bool sl_file_format_path(const size_t bufsize,
                         char buffer[const bufsize],
                         const char path[const static 1],
                         const char name[const static 1],
                         const char suffix[const static 1]);

bool sl_file_open(struct sl_file f[const static 1],
                  const char path[const static 1],
                  const char mode[const static 1]);

void sl_file_close(struct sl_file f[const static 1]);

bool sl_file_can_read(struct sl_file f[const static 1]);

size_t sl_file_read(struct sl_file f[const static 1],
                    struct sl_span buffer[const static 1]);

size_t sl_file_parse_int64(struct sl_file f[const static 1],
                           const size_t count,
                           int64_t buffer[const count]);

#endif  // SL_IO_H_INCLUDED
