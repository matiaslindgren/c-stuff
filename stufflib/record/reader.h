#ifndef STUFFLIB_RECORD_READER_H_INCLUDED
#define STUFFLIB_RECORD_READER_H_INCLUDED

#include <stddef.h>
#include <stufflib/context/context.h>
#include <stufflib/io/io.h>
#include <stufflib/record/record.h>
#include <stufflib/span/span.h>

struct sl_record_reader {
  struct sl_file* file;
  struct sl_record* record;
  size_t n_read;
  size_t index;
  size_t sparse_offset;
  bool has_sparse_offset;
};

bool sl_record_reader_open(
    struct sl_context ctx[static 1],
    struct sl_record_reader reader[const static 1]
);
void sl_record_reader_close(struct sl_record_reader reader[const static 1]);
long long sl_record_reader_ftell(struct sl_record_reader reader[const static 1]);
bool sl_record_reader_is_done(
    struct sl_context ctx[static 1],
    struct sl_record_reader reader[const static 1]
);
bool sl_record_reader_read_sparse_data(
    struct sl_context ctx[static 1],
    struct sl_record_reader reader[const static 1],
    struct sl_span buffer[const static 1]
);
bool sl_record_reader_read_dense_data(
    struct sl_context ctx[static 1],
    struct sl_record_reader reader[const static 1],
    struct sl_span buffer[const static 1]
);
bool sl_record_reader_read(
    struct sl_context ctx[static 1],
    struct sl_record_reader reader[const static 1],
    struct sl_span buffer[const static 1]
);
bool sl_record_read_all(
    struct sl_context ctx[static 1],
    struct sl_record record[const static 1],
    const size_t bufsize,
    void* buffer
);

#endif  // STUFFLIB_RECORD_READER_H_INCLUDED
