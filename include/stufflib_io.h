#ifndef _STUFFLIB_IO_H_INCLUDED
#define _STUFFLIB_IO_H_INCLUDED
#include <stdio.h>

#include "stufflib_macros.h"

size_t stufflib_io_file_size(const char fname[const static 1]) {
  size_t size = 0;

  FILE* fp = fopen(fname, "r");
  if (!fp) {
    STUFFLIB_PRINT_ERROR("cannot open %s", fname);
    goto done;
  }

  if (fseek(fp, 0, SEEK_END)) {
    STUFFLIB_PRINT_ERROR("failed seeking to end of %s", fname);
    goto done;
  }

  const long ftell_pos = ftell(fp);
  if (ftell_pos < 0) {
    STUFFLIB_PRINT_ERROR("failed ftell at end of %s", fname);
    goto done;
  }

  size = ftell_pos;

done:
  if (fp) {
    fclose(fp);
  }
  return size;
}

#endif  // _STUFFLIB_IO_H_INCLUDED
