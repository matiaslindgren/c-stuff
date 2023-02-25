#ifndef _STUFFLIB_TOKENIZER_H_INCLUDED
#define _STUFFLIB_TOKENIZER_H_INCLUDED
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"

typedef struct stufflib_tokenizer stufflib_tokenizer;
struct stufflib_tokenizer {
  stufflib_data data;
  stufflib_data delimiter;
  stufflib_data token;
};

stufflib_data stufflib_tokenizer_next_token(
    const stufflib_data data[const static 1],
    const stufflib_data delimiter[const static 1],
    const size_t data_begin) {
  stufflib_data tail = stufflib_data_slice(data, data_begin, data->size);
  stufflib_data token_end = stufflib_data_find(&tail, delimiter);
  if (!token_end.data) {
    return tail;
  }
  const size_t token_size = token_end.data - tail.data;
  if (!token_size) {
    return (stufflib_data){0};
  }
  return stufflib_data_view(token_size, tail.data);
}

stufflib_tokenizer stufflib_tokenizer_create(
    const stufflib_data data[const static 1],
    const stufflib_data delimiter[const static 1]) {
  stufflib_data data_view = stufflib_data_view(data->size, data->data);
  stufflib_data delimiter_view =
      stufflib_data_view(delimiter->size, delimiter->data);
  stufflib_data first_token =
      stufflib_tokenizer_next_token(&data_view, &delimiter_view, 0);
  return (stufflib_tokenizer){
      .data = data_view,
      .delimiter = delimiter_view,
      .token = first_token,
  };
}

void* stufflib_tokenizer_iter_get_item(stufflib_iterator iter[const static 1]) {
  stufflib_tokenizer* tok = iter->data;
  return &(tok->token);
}

void stufflib_tokenizer_iter_advance(stufflib_iterator iter[const static 1]) {
  stufflib_tokenizer* tok = iter->data;
  const size_t next_token_begin =
      iter->index + tok->token.size + tok->delimiter.size;
  tok->token = stufflib_tokenizer_next_token(&(tok->data),
                                             &(tok->delimiter),
                                             next_token_begin);
  iter->index = STUFFLIB_MIN(next_token_begin, tok->data.size);
  ++(iter->pos);
}

bool stufflib_tokenizer_iter_is_done(stufflib_iterator iter[const static 1]) {
  const stufflib_tokenizer* tok = iter->data;
  return iter->index == tok->data.size;
}

stufflib_iterator stufflib_tokenizer_iter(
    const stufflib_tokenizer tok[const static 1]) {
  return (stufflib_iterator){
      .data = (void*)tok,
      .get_item = stufflib_tokenizer_iter_get_item,
      .advance = stufflib_tokenizer_iter_advance,
      .is_done = stufflib_tokenizer_iter_is_done,
  };
}

#endif  // _STUFFLIB_TOKENIZER_H_INCLUDED
