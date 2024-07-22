#ifndef STUFFLIB_RECORD_WRITER_H_INCLUDED
#define STUFFLIB_RECORD_WRITER_H_INCLUDED

#include "../io/io.h"
#include "../record/record.h"
#include "../span/span.h"

struct sl_record_writer {
  struct sl_file* file;
  struct sl_record* record;
  size_t n_written;
  size_t sparse_offset;
};

bool sl_record_writer_open(struct sl_record_writer writer[const static 1]);

void sl_record_writer_close(struct sl_record_writer writer[const static 1]);

bool sl_record_writer_write(struct sl_record_writer writer[const static 1],
                            struct sl_span buffer[const static 1]);

bool sl_record_write_all(struct sl_record record[const static 1],
                         const size_t bufsize,
                         void* buffer);

#endif  // STUFFLIB_RECORD_WRITER_H_INCLUDED
