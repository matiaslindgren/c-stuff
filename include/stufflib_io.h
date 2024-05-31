#ifndef SL_IO_H_INCLUDED
#define SL_IO_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"
#include "stufflib_span.h"

#ifndef SL_FILE_BUFFER_CAPACITY
#define SL_FILE_BUFFER_CAPACITY (1024 << 5)
#endif

struct sl_file_buffer {
  const char* filename;
  FILE* file;
  size_t capacity;
  struct sl_span data;
};

void sl_file_iter_read_data(struct sl_file_buffer buffer[const static 1]) {
  buffer->data.size = fread(buffer->data.data,
                            sizeof(unsigned char),
                            buffer->capacity,
                            buffer->file);
  if (ferror(buffer->file)) {
    SL_LOG_ERROR("failed reading %zu bytes from %s",
                 buffer->capacity,
                 buffer->filename);
  }
}

void* sl_file_iter_get(struct sl_iterator iter[const static 1]) {
  struct sl_file_buffer* buffer = iter->data;
  return &(buffer->data);
}

void sl_file_iter_advance(struct sl_iterator iter[const static 1]) {
  struct sl_file_buffer* buffer = iter->data;
  sl_file_iter_read_data(buffer);
  iter->index += buffer->data.size;
  iter->pos += 1;
}

bool sl_file_iter_is_done(struct sl_iterator iter[const static 1]) {
  struct sl_file_buffer* buffer = iter->data;
  return !iter->data || ferror(buffer->file) != 0 || buffer->data.size == 0;
}

struct sl_iterator sl_file_iter_open(const char filename[const static 1]) {
  struct sl_iterator iter = {};

  FILE* file = fopen(filename, "rb");
  if (!file) {
    goto done;
  }

  struct sl_file_buffer* buffer = sl_alloc(1, sizeof(struct sl_file_buffer));
  *buffer = (struct sl_file_buffer){
      .file = file,
      .filename = filename,
      .capacity = SL_FILE_BUFFER_CAPACITY,
      .data = sl_span_create(SL_FILE_BUFFER_CAPACITY),
  };
  sl_file_iter_read_data(buffer);
  iter.data = buffer;

done:
  return iter;
}

void sl_file_iter_close(struct sl_iterator iter[const static 1]) {
  if (!iter->data) {
    return;
  }
  struct sl_file_buffer* buffer = iter->data;
  fclose(buffer->file);
  sl_span_delete(&buffer->data);
  sl_free(iter->data);
  iter->data = nullptr;
}

#endif  // SL_IO_H_INCLUDED
