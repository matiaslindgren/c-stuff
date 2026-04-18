#ifndef SL_FILESYSTEM_H_INCLUDED
#define SL_FILESYSTEM_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <stufflib/context/context.h>
#include <stufflib/span/span.h>
#include <stufflib/string/string.h>

struct sl_span sl_fs_read_file(
    struct sl_context ctx[static 1],
    const char path[const static 1],
    struct sl_span buffer[const static 1]
);
struct sl_string sl_fs_read_file_utf8(
    struct sl_context ctx[static 1],
    const char path[const static 1],
    struct sl_span buffer[const static 1]
);
bool sl_fs_read_int64(
    struct sl_context ctx[static 1],
    const char path[const static 1],
    size_t count,
    int64_t values[const count]
);

#endif  // SL_FILESYSTEM_H_INCLUDED
