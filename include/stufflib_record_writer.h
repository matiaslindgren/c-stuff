#ifndef STUFFLIB_RECORD_WRITER_H_INCLUDED
#define STUFFLIB_RECORD_WRITER_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stufflib_io.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"
#include "stufflib_span.h"

struct sl_record_writer {
  struct sl_file* file;
  struct sl_record* record;
  size_t n_written;
  size_t sparse_offset;
};

bool sl_record_writer_open(struct sl_record_writer writer[const static 1]) {
  if (!writer->file || !writer->record) {
    SL_LOG_ERROR("incorrectly initialized record writer");
    return false;
  }
  char full_path[1024] = {0};
  if (!sl_file_format_path(SL_ARRAY_LEN(full_path),
                           full_path,
                           writer->record->path,
                           writer->record->name,
                           ".sl_record_data")) {
    SL_LOG_ERROR("failed formatting record data file path");
    return false;
  }
  if (!sl_file_open(writer->file, full_path, "wb")) {
    SL_LOG_ERROR("cannot open record data file '%s'", full_path);
    return false;
  }
  return true;
}

void sl_record_writer_close(struct sl_record_writer writer[const static 1]) {
  sl_file_close(writer->file);
}

bool sl_record_writer_write(struct sl_record_writer writer[const static 1],
                            struct sl_span buffer[const static 1]) {
  const char* const layout = writer->record->layout;
  const size_t item_size = sl_record_item_size(writer->record);
  const size_t buffer_length = buffer->size / item_size;

  if (SL_STR_EQ(layout, "sparse")) {
    int64_t offset = (int64_t)writer->sparse_offset;
    for (size_t buf_idx = 0; buf_idx < buffer_length; ++buf_idx) {
      unsigned char* value = buffer->data + buf_idx * item_size;
      if (!sl_misc_is_zero(item_size, value)) {
        if (1 != fwrite(&offset, sizeof(offset), 1, writer->file->file) ||
            ferror(writer->file->file) != 0) {
          SL_LOG_ERROR("failed appending offset to %s", writer->file->path);
          return false;
        }
        if (1 != fwrite(value, item_size, 1, writer->file->file) ||
            ferror(writer->file->file) != 0) {
          SL_LOG_ERROR("failed appending data to %s", writer->file->path);
          return false;
        }
        writer->n_written += 1;
        offset = 0;
      }
      offset += 1;
    }
    writer->sparse_offset = (size_t)offset;
    return true;
  }

  if (SL_STR_EQ(layout, "dense")) {
    const size_t n_written =
        fwrite(buffer->data, item_size, buffer_length, writer->file->file);
    writer->n_written += n_written;
    if (ferror(writer->file->file) != 0 || buffer_length != n_written) {
      SL_LOG_ERROR("failed writing dense data to %s", writer->file->path);
      return false;
    }
    return true;
  }

  SL_LOG_ERROR("unknown data layout %s", layout);
  return false;
}

bool sl_record_write_all(struct sl_record record[const static 1],
                         const size_t bufsize,
                         void* buffer) {
  bool ok = false;
  struct sl_file file = {0};
  struct sl_record_writer writer = {
      .file = &file,
      .record = record,
  };
  if (!sl_record_writer_open(&writer)) {
    SL_LOG_ERROR("failed opening data file, cannot write record");
    goto done;
  }
  struct sl_span data = sl_span_view(bufsize, buffer);
  if (!sl_record_writer_write(&writer, &data)) {
    SL_LOG_ERROR("failed writing record data");
    goto done;
  }
  ok = true;
done:
  sl_record_writer_close(&writer);
  return ok;
}

#endif  // STUFFLIB_RECORD_WRITER_H_INCLUDED
