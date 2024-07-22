#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../io/io.h"
#include "../macros/macros.h"
#include "../record/record.h"
#include "../span/span.h"
#include "./reader.h"

bool sl_record_reader_open(struct sl_record_reader reader[const static 1]) {
  if (!reader->file || !reader->record) {
    SL_LOG_ERROR("incorrectly initialized record reader");
    return false;
  }
  char full_path[1024] = {0};
  if (!sl_file_format_path(SL_ARRAY_LEN(full_path),
                           full_path,
                           reader->record->path,
                           reader->record->name,
                           ".sl_record_data")) {
    SL_LOG_ERROR("failed formatting record data file path");
    return false;
  }
  if (!sl_file_open(reader->file, full_path, "rb")) {
    SL_LOG_ERROR("cannot open record data file '%s'", full_path);
    return false;
  }
  return true;
}

void sl_record_reader_close(struct sl_record_reader reader[const static 1]) {
  sl_file_close(reader->file);
}

long long sl_record_reader_ftell(
    struct sl_record_reader reader[const static 1]) {
  return reader->file && reader->file->file ? ftell(reader->file->file) : -1;
}

bool sl_record_reader_is_done(struct sl_record_reader reader[const static 1]) {
  if (!reader->file || feof(reader->file->file)) {
    return true;
  }
  const long long fpos = sl_record_reader_ftell(reader);
  if (fpos < 0) {
    SL_LOG_ERROR("failed retrieving file pos for sl_record reader");
    return true;
  }
  const size_t sl_record_disk_size =
      ((SL_STR_EQ(reader->record->layout, "sparse") ? sizeof(int64_t) : 0) +
       sl_record_item_size(reader->record)) *
      reader->record->size;
  return (size_t)fpos >= sl_record_disk_size;
}

bool sl_record_reader_read_sparse_data(
    struct sl_record_reader reader[const static 1],
    struct sl_span buffer[const static 1]) {
  sl_span_clear(buffer);

  // TODO cache in reader on open?
  const size_t item_size = sl_record_item_size(reader->record);
  const size_t buffer_length = buffer->size / item_size;

  if (reader->sparse_offset > buffer_length) {
    reader->index += buffer_length;
    reader->sparse_offset -= buffer_length;
    return true;
  }

  while (sl_file_can_read(reader->file) && !sl_record_reader_is_done(reader)) {
    if (!reader->has_sparse_offset) {
      int64_t offset = -1;
      if (1 != fread(&offset, sizeof(offset), 1, reader->file->file) ||
          offset < 0) {
        SL_LOG_ERROR(
            "failed reading sparse index offset from %s at index %zu with "
            "n_read %zu",
            reader->file->path,
            reader->index,
            reader->n_read);
        return false;
      }
      reader->sparse_offset = (size_t)(offset);
      reader->has_sparse_offset = true;
    }

    const size_t idx = reader->index;
    const size_t offset = reader->sparse_offset;

    const size_t curr_batch = idx / buffer_length;
    const size_t batch_offset = idx % buffer_length;

    const size_t next_batch = (idx + offset) / buffer_length;
    const size_t buf_idx = (idx + offset) % buffer_length;

    if (next_batch > curr_batch) {
      const size_t n_skip = buffer_length - batch_offset;
      reader->sparse_offset -= n_skip;
      reader->index += n_skip;
      return true;
    }

    if (1 != fread(buffer->data + buf_idx * item_size,
                   item_size,
                   1,
                   reader->file->file)) {
      SL_LOG_ERROR(
          "failed reading sparse data from %s at index %zu with n_read %zu",
          reader->file->path,
          reader->index,
          reader->n_read);
      return false;
    }

    reader->n_read += 1;
    reader->index += reader->sparse_offset;
    reader->sparse_offset = 0;
    reader->has_sparse_offset = false;
  }

  return true;
}

bool sl_record_reader_read_dense_data(
    struct sl_record_reader reader[const static 1],
    struct sl_span buffer[const static 1]) {
  const size_t item_size = sl_record_item_size(reader->record);
  const size_t buffer_length = buffer->size / item_size;

  const size_t n_read =
      fread(buffer->data, item_size, buffer_length, reader->file->file);
  reader->n_read += n_read;
  reader->index += n_read;

  if (buffer_length != n_read) {
    SL_LOG_ERROR("failed reading dense data from %s", reader->file->path);
    return false;
  }
  return true;
}

bool sl_record_reader_read(struct sl_record_reader reader[const static 1],
                           struct sl_span buffer[const static 1]) {
  const char* const layout = reader->record->layout;
  if (SL_STR_EQ(layout, "sparse")) {
    return sl_record_reader_read_sparse_data(reader, buffer);
  } else if (SL_STR_EQ(layout, "dense")) {
    return sl_record_reader_read_dense_data(reader, buffer);
  }
  SL_LOG_ERROR("unknown data layout %s", layout);
  return false;
}

bool sl_record_read_all(struct sl_record record[const static 1],
                        const size_t bufsize,
                        void* buffer) {
  bool ok = false;
  struct sl_file file = {0};
  struct sl_record_reader reader = {
      .file = &file,
      .record = record,
  };
  if (!sl_record_reader_open(&reader)) {
    SL_LOG_ERROR("failed opening data file, cannot read record");
    goto done;
  }
  struct sl_span data = sl_span_view(bufsize, buffer);
  if (!sl_record_reader_read(&reader, &data) ||
      !sl_record_reader_is_done(&reader)) {
    SL_LOG_ERROR("failed reading record data");
    goto done;
  }
  ok = true;
done:
  sl_record_reader_close(&reader);
  return ok;
}
