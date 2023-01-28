#ifndef _STUFFLIB_STR_H_INCLUDED
#define _STUFFLIB_STR_H_INCLUDED
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"

void stufflib_str_split_destroy(char* chunks[const static 1]) {
  for (char** chunk = chunks; *chunk; ++chunk) {
    free(*chunk);
  }
  free(chunks);
}

size_t stufflib_str_count_chunks(char* chunks[const static 1]) {
  size_t count = 0;
  for (char** chunk = chunks; *chunk; ++chunk) {
    ++count;
  }
  return count;
}

// TODO flat
char** stufflib_str_split(const char str[const static 1],
                          const char separator[const static 1]) {
  assert(strlen(separator));
  char** chunks = 0;
  size_t num_chunks = 0;
  for (const char* chunk = str; chunk;) {
    const char* chunk_end = strstr(chunk, separator);
    const size_t chunk_len = chunk_end ? chunk_end - chunk : strlen(chunk);
    {
      char** tmp = realloc(chunks, (num_chunks + 2) * (sizeof(char*)));
      if (!tmp) {
        STUFFLIB_PRINT_ERROR(
            "failed resizing result chunks array during split");
        goto error;
      }
      chunks = tmp;
    }
    chunks[num_chunks + 1] = 0;
    {
      char* tmp = calloc(chunk_len + 1, 1);
      if (!tmp) {
        STUFFLIB_PRINT_ERROR("failed allocating chunk during split");
        goto error;
      }
      chunks[num_chunks] = tmp;
    }
    memcpy(chunks[num_chunks], chunk, chunk_len);
    ++num_chunks;
    chunk = chunk_end ? chunk_end + strlen(separator) : chunk_end;
  }
  return chunks;

error:
  stufflib_str_split_destroy(chunks);
  return 0;
}

#endif  // _STUFFLIB_STR_H_INCLUDED
