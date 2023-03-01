#ifndef _SL_TOKENIZER_H_INCLUDED
#define _SL_TOKENIZER_H_INCLUDED
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"

typedef struct sl_tokenizer sl_tokenizer;
struct sl_tokenizer {
  sl_data data;
  sl_data delimiter;
  sl_data token;
};

sl_data sl_tokenizer_next_token(const sl_data data[const static 1],
                                const sl_data delimiter[const static 1],
                                const size_t data_begin) {
  sl_data tail = sl_data_slice(data, data_begin, data->size);
  sl_data token_end = sl_data_find(&tail, delimiter);
  if (!token_end.data) {
    return tail;
  }
  const size_t token_size = token_end.data - tail.data;
  if (!token_size) {
    return (sl_data){0};
  }
  return sl_data_view(token_size, tail.data);
}

sl_tokenizer sl_tokenizer_create(const sl_data data[const static 1],
                                 const sl_data delimiter[const static 1]) {
  sl_data data_view = sl_data_view(data->size, data->data);
  sl_data delimiter_view = sl_data_view(delimiter->size, delimiter->data);
  sl_data first_token = sl_tokenizer_next_token(&data_view, &delimiter_view, 0);
  return (sl_tokenizer){
      .data = data_view,
      .delimiter = delimiter_view,
      .token = first_token,
  };
}

void* sl_tokenizer_iter_get_item(sl_iterator iter[const static 1]) {
  sl_tokenizer* tok = iter->data;
  return &(tok->token);
}

void sl_tokenizer_iter_advance(sl_iterator iter[const static 1]) {
  sl_tokenizer* tok = iter->data;
  const size_t next_token_begin =
      iter->index + tok->token.size + tok->delimiter.size;
  tok->token = sl_tokenizer_next_token(&(tok->data),
                                       &(tok->delimiter),
                                       next_token_begin);
  iter->index = SL_MIN(next_token_begin, tok->data.size);
  ++(iter->pos);
}

bool sl_tokenizer_iter_is_done(sl_iterator iter[const static 1]) {
  const sl_tokenizer* tok = iter->data;
  return iter->index == tok->data.size;
}

sl_iterator sl_tokenizer_iter(const sl_tokenizer tok[const static 1]) {
  return (sl_iterator){
      .data = (void*)tok,
      .get_item = sl_tokenizer_iter_get_item,
      .advance = sl_tokenizer_iter_advance,
      .is_done = sl_tokenizer_iter_is_done,
  };
}

#endif  // _SL_TOKENIZER_H_INCLUDED
