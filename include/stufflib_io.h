#ifndef _STUFFLIB_IO_H_INCLUDED
#define _STUFFLIB_IO_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_macros.h"
#include "stufflib_str.h"

size_t stufflib_io_file_size(const char fname[const static 1]) {
  size_t size = 0;

  FILE* fp = fopen(fname, "r");
  if (!fp) {
    STUFFLIB_LOG_ERROR("cannot open %s", fname);
    goto done;
  }

  {
    int chr = 0;
    while ((chr = fgetc(fp)) != EOF) {
      ++size;
    }
  }
  if (ferror(fp)) {
    STUFFLIB_LOG_ERROR("failed reading from %s", fname);
    goto done;
  }

done:
  if (fp) {
    fclose(fp);
  }
  return size;
}

char* stufflib_io_slurp_file(const char fname[const static 1]) {
  char* content = calloc(1, 1);
  FILE* fp = fopen(fname, "r");
  if (!content) {
    STUFFLIB_LOG_ERROR("failed allocating slurp buffer");
    goto error;
  }
  if (!fp) {
    STUFFLIB_LOG_ERROR("cannot open %s", fname);
    goto error;
  }

  {
    size_t size = 0;
    int chr = 0;
    while ((chr = fgetc(fp)) != EOF) {
      ++size;
      {
        char* tmp = realloc(content, size + 1);
        if (!tmp) {
          STUFFLIB_LOG_ERROR("failed resizing slurp buffer");
          goto error;
        }
        content = tmp;
      }
      content[size - 1] = chr;
      content[size] = 0;
    }
  }
  if (ferror(fp)) {
    STUFFLIB_LOG_ERROR("failed reading from %s", fname);
    goto error;
  }

  fclose(fp);
  return content;

error:
  if (fp) {
    fclose(fp);
  }
  if (content) {
    free(content);
  }
  return nullptr;
}

char** stufflib_io_slurp_lines(const char fname[const static 1],
                               const char line_ending[const static 1]) {
  char* file_content = nullptr;
  char** lines = nullptr;

  file_content = stufflib_io_slurp_file(fname);
  if (!file_content) {
    goto error;
  }

  lines = stufflib_str_split(file_content, line_ending);
  if (!lines) {
    goto error;
  }

  const size_t num_lines = stufflib_str_chunks_count(lines);
  if (strcmp(lines[num_lines - 1], "") == 0) {
    char** tmp = stufflib_str_slice_chunks(lines, 0, num_lines - 1);
    if (!tmp) {
      goto error;
    }
    stufflib_str_chunks_destroy(lines);
    lines = tmp;
  }

  free(file_content);
  return lines;

error:
  if (file_content) {
    free(file_content);
  }
  if (lines) {
    stufflib_str_chunks_destroy(lines);
  }
  return nullptr;
}

#endif  // _STUFFLIB_IO_H_INCLUDED
