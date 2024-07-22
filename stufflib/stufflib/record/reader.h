#ifndef STUFFLIB_RECORD_READER_H_INCLUDED
#define STUFFLIB_RECORD_READER_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../io/io.h"
#include "../record/record.h"
#include "../span/span.h"

struct sl_record_reader {
  struct sl_file* file;
  struct sl_record* record;
  size_t n_read;
  size_t index;
  size_t sparse_offset;
  bool has_sparse_offset;
};

bool sl_record_reader_open(struct sl_record_reader reader[const static 1]);

void sl_record_reader_close(struct sl_record_reader reader[const static 1]);

long long sl_record_reader_ftell(
    struct sl_record_reader reader[const static 1]);

bool sl_record_reader_is_done(struct sl_record_reader reader[const static 1]);

bool sl_record_reader_read_sparse_data(
    struct sl_record_reader reader[const static 1],
    struct sl_span buffer[const static 1]);

bool sl_record_reader_read_dense_data(
    struct sl_record_reader reader[const static 1],
    struct sl_span buffer[const static 1]);

bool sl_record_reader_read(struct sl_record_reader reader[const static 1],
                           struct sl_span buffer[const static 1]);

bool sl_record_read_all(struct sl_record record[const static 1],
                        const size_t bufsize,
                        void* buffer);

#endif  // STUFFLIB_RECORD_READER_H_INCLUDED
