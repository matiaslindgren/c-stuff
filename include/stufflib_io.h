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

  {
    int chr = 0;
    while ((chr = fgetc(fp)) != EOF) {
      ++size;
    }
  }
  if (ferror(fp)) {
    STUFFLIB_PRINT_ERROR("failed reading from %s", fname);
    goto done;
  }

done:
  if (fp) {
    fclose(fp);
  }
  return size;
}

char* stufflib_io_slurp_file(const char fname[const static 1]) {
  char* content = 0;
  FILE* fp = 0;

  fp = fopen(fname, "r");
  if (!fp) {
    STUFFLIB_PRINT_ERROR("cannot open %s", fname);
    goto done;
  }

  {
    size_t size = 0;
    int chr = 0;
    while ((chr = fgetc(fp)) != EOF) {
      ++size;
      {
        char* tmp = realloc(content, size + 1);
        if (!tmp) {
          STUFFLIB_PRINT_ERROR("failed resizing slurp buffer");
          free(content);
          content = 0;
          goto done;
        }
        content = tmp;
      }
      content[size - 1] = chr;
      content[size] = 0;
    }
  }
  if (ferror(fp)) {
    STUFFLIB_PRINT_ERROR("failed reading from %s", fname);
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
