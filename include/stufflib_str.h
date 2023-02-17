#ifndef _STUFFLIB_STR_H_INCLUDED
#define _STUFFLIB_STR_H_INCLUDED
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_macros.h"

void stufflib_str_chunks_destroy(char* chunks[const static 1]) {
  for (size_t i = 0; chunks[i]; ++i) {
    free(chunks[i]);
  }
  free(chunks);
}

size_t stufflib_str_chunks_count(char* chunks[const static 1]) {
  size_t count = 0;
  for (size_t i = 0; chunks[i]; ++i) {
    ++count;
  }
  return count;
}

char** stufflib_str_slice_chunks(char* src_chunks[const static 1],
                                 const size_t begin,
                                 const size_t end) {
  assert(end >= begin);
  char** dst_chunks = calloc(end - begin + 1, sizeof(char*));
  if (!dst_chunks) {
    STUFFLIB_PRINT_ERROR("failed allocating new chunks during slice");
    goto error;
  }
  for (size_t pos = begin; pos < end; ++pos) {
    const size_t chunk_len = strlen(src_chunks[pos]);
    const size_t dst_pos = pos - begin;
    dst_chunks[dst_pos] = calloc(chunk_len + 1, 1);
    if (!dst_chunks[dst_pos]) {
      STUFFLIB_PRINT_ERROR("failed allocating new chunk during slice");
      goto error;
    }
    memcpy(dst_chunks[dst_pos], src_chunks[pos], chunk_len);
  }
  return dst_chunks;

error:
  if (dst_chunks) {
    stufflib_str_chunks_destroy(dst_chunks);
  }
  return nullptr;
}

// TODO flat
char** stufflib_str_split_any(const char str[const static 1],
                              const size_t num_separators,
                              const char* separators[const num_separators]) {
  for (size_t i = 0; i < num_separators; ++i) {
    assert(strlen(separators[i]));
  }

  char** chunks = nullptr;
  size_t num_chunks = 0;

  for (const char* chunk = str; chunk;) {
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

    const char* chunk_end = nullptr;
    size_t min_chunk_len = SIZE_MAX;
    size_t separator_len = 0;
    for (size_t i = 0; i < num_separators; ++i) {
      const char* end = strstr(chunk, separators[i]);
      if (end && end - chunk < min_chunk_len) {
        chunk_end = end;
        min_chunk_len = end - chunk;
        separator_len = strlen(separators[i]);
      }
    }
    const size_t chunk_len = chunk_end ? min_chunk_len : strlen(chunk);

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
    chunk = chunk_end ? chunk_end + separator_len : chunk_end;
  }

  return chunks;

error:
  stufflib_str_chunks_destroy(chunks);
  return nullptr;
}

char** stufflib_str_split(const char str[const static 1],
                          const char separator[const static 1]) {
  return stufflib_str_split_any(str, 1, (const char*[]){separator});
}

char** stufflib_str_split_whitespace(const char str[const static 1]) {
  static const char* whitespace[] = {
      "\t",
      "\n",
      /* "\v" */   // TODO
      /* "\r", */  // TODO
      " ",
  };
  return stufflib_str_split_any(str,
                                STUFFLIB_ARRAY_LEN(whitespace),
                                whitespace);
}

#endif  // _STUFFLIB_STR_H_INCLUDED
