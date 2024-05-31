#ifndef SL_FILESYSTEM_H_INCLUDED
#define SL_FILESYSTEM_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_io.h"
#include "stufflib_span.h"

struct sl_span sl_fs_slurp_file(const char filename[const static 1]) {
  struct sl_iterator file_iter = sl_file_iter_open(filename);
  struct sl_span data = {0};
  for (; !sl_file_iter_is_done(&file_iter); sl_file_iter_advance(&file_iter)) {
    struct sl_span* buffer = sl_file_iter_get(&file_iter);
    sl_span_extend(&data, buffer);
  }
  sl_file_iter_close(&file_iter);
  return data;
}

#endif  // SL_FILESYSTEM_H_INCLUDED
