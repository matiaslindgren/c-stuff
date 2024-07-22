#ifndef SL_DATA_H_INCLUDED
#define SL_DATA_H_INCLUDED
// TODO pull iterator impl from core layer
#include "../iterator/iterator.h"

struct sl_span {
  bool owned;
  size_t size;
  unsigned char* data;
};

struct sl_span sl_span_view(size_t size, unsigned char data[size]);

struct sl_span sl_span_create(size_t size);

void sl_span_destroy(struct sl_span data[static 1]);

 void sl_span_clear(struct sl_span data[const static 1]);

struct sl_span sl_span_copy(struct sl_span src[const static 1]);

struct sl_span sl_span_from_str(const char str[const static 1]);

bool sl_span_is_hexadecimal_str(struct sl_span src[const static 1]);

struct sl_span sl_span_parse_hex(struct sl_span src[const static 1]);

struct sl_span sl_span_concat(struct sl_span data1[const static 1],
                              struct sl_span data2[const static 1]);

void sl_span_extend(struct sl_span dst[static 1],
                    struct sl_span src[const static 1]);

struct sl_span sl_span_slice(struct sl_span data[const static 1],
                             size_t begin,
                             size_t end_or_max);

struct sl_span sl_span_find(struct sl_span data[const static 1],
                            struct sl_span pattern[const static 1]);

int sl_span_compare(struct sl_span lhs[const static 1],
                    struct sl_span rhs[const static 1]);

void* sl_span_iter_get(struct sl_iterator iter[const static 1]);

void sl_span_iter_advance(struct sl_iterator iter[const static 1]);

bool sl_span_iter_is_done(struct sl_iterator iter[const static 1]);

struct sl_iterator sl_span_iter(struct sl_span data[const static 1]);

#endif  // SL_DATA_H_INCLUDED
