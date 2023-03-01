#ifndef _SL_IO_H_INCLUDED
#define _SL_IO_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"

#define SL_FILE_BUFFER_CAPACITY 4096

struct sl_file_buffer {
  const char* filename;
  FILE* restrict file;
  size_t capacity;
  struct sl_data data;
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

void* sl_file_iter_get_item(struct sl_iterator iter[const static 1]) {
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
  struct sl_iterator iter = {
      .get_item = sl_file_iter_get_item,
      .advance = sl_file_iter_advance,
      .is_done = sl_file_iter_is_done,
  };

  FILE* restrict file = fopen(filename, "rb");
  if (!file) {
    goto done;
  }

  struct sl_file_buffer* buffer = sl_alloc(1, sizeof(struct sl_file_buffer));
  *buffer = (struct sl_file_buffer){
      .file = file,
      .filename = filename,
      .capacity = SL_FILE_BUFFER_CAPACITY,
      .data = sl_data_create(SL_FILE_BUFFER_CAPACITY),
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
  sl_data_delete(&buffer->data);
  sl_free(iter->data);
  iter->data = nullptr;
}

#endif  // _SL_IO_H_INCLUDED
