#ifndef SL_TOKENIZER_H_INCLUDED
#define SL_TOKENIZER_H_INCLUDED
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"

struct sl_tokenizer {
  struct sl_data data;
  struct sl_data delimiter;
  struct sl_data token;
};

struct sl_data sl_tokenizer_next_token(
    const struct sl_data data[const static 1],
    const struct sl_data delimiter[const static 1],
    const size_t data_begin) {
  struct sl_data tail = sl_data_slice(data, data_begin, data->size);
  struct sl_data token_end = sl_data_find(&tail, delimiter);
  if (!token_end.data) {
    return tail;
  }
  const size_t token_size = token_end.data - tail.data;
  if (!token_size) {
    return (struct sl_data){0};
  }
  return sl_data_view(token_size, tail.data);
}

struct sl_tokenizer sl_tokenizer_create(
    const struct sl_data data[const static 1],
    const struct sl_data delimiter[const static 1]) {
  struct sl_data data_view = sl_data_view(data->size, data->data);
  struct sl_data delimiter_view =
      sl_data_view(delimiter->size, delimiter->data);
  struct sl_data first_token =
      sl_tokenizer_next_token(&data_view, &delimiter_view, 0);
  return (struct sl_tokenizer){
      .data = data_view,
      .delimiter = delimiter_view,
      .token = first_token,
  };
}

void* sl_tokenizer_iter_get_item(struct sl_iterator iter[const static 1]) {
  struct sl_tokenizer* tok = iter->data;
  return &(tok->token);
}

void sl_tokenizer_iter_advance(struct sl_iterator iter[const static 1]) {
  struct sl_tokenizer* tok = iter->data;
  const size_t next_token_begin =
      iter->index + tok->token.size + tok->delimiter.size;
  tok->token = sl_tokenizer_next_token(&(tok->data),
                                       &(tok->delimiter),
                                       next_token_begin);
  iter->index = SL_MIN(next_token_begin, tok->data.size);
  ++(iter->pos);
}

bool sl_tokenizer_iter_is_done(struct sl_iterator iter[const static 1]) {
  const struct sl_tokenizer* tok = iter->data;
  return iter->index == tok->data.size;
}

struct sl_iterator sl_tokenizer_iter(
    const struct sl_tokenizer tok[const static 1]) {
  return (struct sl_iterator){
      .data = (void*)tok,
      .get_item = sl_tokenizer_iter_get_item,
      .advance = sl_tokenizer_iter_advance,
      .is_done = sl_tokenizer_iter_is_done,
  };
}

#endif  // SL_TOKENIZER_H_INCLUDED
