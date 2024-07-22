#ifndef SL_TOKENIZER_H_INCLUDED
#define SL_TOKENIZER_H_INCLUDED
#include "../iterator/iterator.h"
#include "../span/span.h"

struct sl_tokenizer {
  struct sl_span data;
  struct sl_span delimiter;
  struct sl_span token;
};

struct sl_span sl_tokenizer_next_token(struct sl_span data[const static 1],
                                       struct sl_span delimiter[const static 1],
                                       size_t data_begin);

struct sl_tokenizer sl_tokenizer_create(
    struct sl_span data[const static 1],
    struct sl_span delimiter[const static 1]);

void* sl_tokenizer_iter_get(struct sl_iterator iter[const static 1]);

void sl_tokenizer_iter_advance(struct sl_iterator iter[const static 1]);

bool sl_tokenizer_iter_is_done(struct sl_iterator iter[const static 1]);

struct sl_iterator sl_tokenizer_iter(struct sl_tokenizer tok[const static 1]);

#endif  // SL_TOKENIZER_H_INCLUDED
