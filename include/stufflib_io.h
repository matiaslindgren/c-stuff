#ifndef _STUFFLIB_IO_H_INCLUDED
#define _STUFFLIB_IO_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"

#define STUFFLIB_FILE_BUFFER_CAPACITY 4096

typedef struct stufflib_file_buffer stufflib_file_buffer;
struct stufflib_file_buffer {
  const char* filename;
  FILE* restrict file;
  size_t capacity;
  stufflib_data data;
};

void stufflib_file_iter_read_data(stufflib_file_buffer buffer[const static 1]) {
  buffer->data.size = fread(buffer->data.data,
                            sizeof(unsigned char),
                            buffer->capacity,
                            buffer->file);
  if (ferror(buffer->file)) {
    STUFFLIB_LOG_ERROR("failed reading %zu bytes from %s",
                       buffer->capacity,
                       buffer->filename);
  }
}

void* stufflib_file_iter_get_item(stufflib_iterator iter[const static 1]) {
  stufflib_file_buffer* buffer = iter->data;
  return &(buffer->data);
}

void stufflib_file_iter_advance(stufflib_iterator iter[const static 1]) {
  stufflib_file_buffer* buffer = iter->data;
  stufflib_file_iter_read_data(buffer);
  iter->index += buffer->data.size;
  iter->pos += 1;
}

bool stufflib_file_iter_is_done(stufflib_iterator iter[const static 1]) {
  stufflib_file_buffer* buffer = iter->data;
  return !iter->data || ferror(buffer->file) != 0 || buffer->data.size == 0;
}

stufflib_iterator stufflib_file_iter_open(const char filename[const static 1]) {
  stufflib_iterator iter = (stufflib_iterator){
      .get_item = stufflib_file_iter_get_item,
      .advance = stufflib_file_iter_advance,
      .is_done = stufflib_file_iter_is_done,
  };

  FILE* restrict file = fopen(filename, "rb");
  if (!file) {
    goto done;
  }

  stufflib_file_buffer* buffer =
      stufflib_alloc(1, sizeof(stufflib_file_buffer));
  *buffer = (stufflib_file_buffer){
      .file = file,
      .filename = filename,
      .capacity = STUFFLIB_FILE_BUFFER_CAPACITY,
      .data = stufflib_data_create(STUFFLIB_FILE_BUFFER_CAPACITY),
  };
  stufflib_file_iter_read_data(buffer);
  iter.data = buffer;

done:
  return iter;
}

void stufflib_file_iter_close(stufflib_iterator iter[const static 1]) {
  if (!iter->data) {
    return;
  }
  stufflib_file_buffer* buffer = iter->data;
  fclose(buffer->file);
  stufflib_data_delete(&buffer->data);
  stufflib_free(iter->data);
  iter->data = nullptr;
}

#endif  // _STUFFLIB_IO_H_INCLUDED
