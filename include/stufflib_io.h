#ifndef _STUFFLIB_IO_H_INCLUDED
#define _STUFFLIB_IO_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>

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

char* stufflib_io_slurp_file(const char fname[const static 1]) {
  char* content = 0;
  FILE* fp = fopen(fname, "r");
  if (!fp) {
    STUFFLIB_PRINT_ERROR("cannot open %s", fname);
    goto done;
  }

  const size_t size = stufflib_io_file_size(fname);
  content = calloc(size + 1, 1);
  if (!content) {
    STUFFLIB_PRINT_ERROR("failed allocating buffer for reading %s", fname);
    goto done;
  }

  if (fread(content, 1, size, fp) != size) {
    STUFFLIB_PRINT_ERROR("failed reading %zu bytes from %s", size, fname);
    free(content);
    content = 0;
    goto done;
  }

done:
  if (fp) {
    fclose(fp);
  }
  return content;
}

#endif  // _STUFFLIB_IO_H_INCLUDED
