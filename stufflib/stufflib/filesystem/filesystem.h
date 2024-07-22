#ifndef SL_FILESYSTEM_H_INCLUDED
#define SL_FILESYSTEM_H_INCLUDED
#include <stdint.h>

#include "../span/span.h"

struct sl_span sl_fs_read_file(const char path[const static 1],
                               struct sl_span buffer[const static 1]);

struct sl_string sl_fs_read_file_utf8(const char path[const static 1],
                                      struct sl_span buffer[const static 1]);

bool sl_fs_read_int64(const char path[const static 1],
                      const size_t count,
                      int64_t values[const count]);

#endif  // SL_FILESYSTEM_H_INCLUDED
