#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_macros.h"
#include "stufflib_tokenizer.h"

bool test_tokenize_empty(const bool verbose) {
  stufflib_data empty1 = (stufflib_data){0};
  stufflib_data empty2 = (stufflib_data){0};
  unsigned char x[] = {1, 2, 3};
  const size_t n = STUFFLIB_ARRAY_LEN(x);
  stufflib_data data = stufflib_data_view(n, x);

  stufflib_data token1 = stufflib_tokenizer_next_token(&empty1, &empty2, 0);
  assert(!token1.size);
  assert(!token1.data);

  stufflib_data token2 = stufflib_tokenizer_next_token(&data, &empty1, 0);
  assert(token2.size == 3);
  assert(token2.data == x);

  stufflib_data token3 = stufflib_tokenizer_next_token(&empty1, &data, 0);
  assert(!token3.size);
  assert(!token3.data);

  return true;
}

bool test_tokenize_one(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 1, 2, 3};
  unsigned char x2[] = {2};
  const size_t n1 = STUFFLIB_ARRAY_LEN(x1);
  const size_t n2 = STUFFLIB_ARRAY_LEN(x2);
  stufflib_data data1 = stufflib_data_view(n1, x1);
  stufflib_data data2 = stufflib_data_view(n2, x2);

  stufflib_data token1 = stufflib_tokenizer_next_token(&data1, &data2, 0);
  assert(token1.size == 1);
  assert(token1.data == x1);

  stufflib_data token2 = stufflib_tokenizer_next_token(&data1, &data2, 1);
  assert(!token2.size);
  assert(!token2.data);

  stufflib_data token3 = stufflib_tokenizer_next_token(&data1, &data2, 2);
  assert(token3.size == 2);
  assert(token3.data == x1 + 2);
  assert(token3.data + 1 == x1 + 3);

  return true;
}

bool test_tokenize_many(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 1, 2, 3};
  unsigned char x2[] = {2, 3, 1};
  const size_t n1 = STUFFLIB_ARRAY_LEN(x1);
  const size_t n2 = STUFFLIB_ARRAY_LEN(x2);
  stufflib_data data1 = stufflib_data_view(n1, x1);
  stufflib_data data2 = stufflib_data_view(n2, x2);

  stufflib_data token1 = stufflib_tokenizer_next_token(&data1, &data2, 0);
  assert(token1.size == 1);
  assert(token1.data == x1);

  stufflib_data token2 = stufflib_tokenizer_next_token(&data1, &data2, 1);
  assert(!token2.size);
  assert(!token2.data);

  stufflib_data token3 = stufflib_tokenizer_next_token(&data1, &data2, 2);
  assert(token3.size == 4);
  for (size_t i = 0; i < 4; ++i) {
    assert(token3.data + i == x1 + 2 + i);
  }

  return true;
}

bool test_tokenize_delimiters(const bool verbose) {
  unsigned char x1[] = {0, 0, 0};
  unsigned char x2[] = {0};
  const size_t n1 = STUFFLIB_ARRAY_LEN(x1);
  const size_t n2 = STUFFLIB_ARRAY_LEN(x2);
  stufflib_data data1 = stufflib_data_view(n1, x1);
  stufflib_data data2 = stufflib_data_view(n2, x2);

  for (size_t begin = 0; begin < 3; ++begin) {
    stufflib_data token = stufflib_tokenizer_next_token(&data1, &data2, begin);
    assert(!token.size);
    assert(!token.data);
  }

  return true;
}

bool test_tokenize_iter(const bool verbose) {
  unsigned char data_str[] = {
      2, 3, 4, 0, 1, 5, 6, 0, 1, 8, 9, 10, 11, 0, 1, 12, 0, 1, 0, 1,
  };
  unsigned char delimiter_str[] = {0, 1};
  stufflib_data data =
      stufflib_data_view(STUFFLIB_ARRAY_LEN(data_str), data_str);
  stufflib_data delimiter =
      stufflib_data_view(STUFFLIB_ARRAY_LEN(delimiter_str), delimiter_str);

  stufflib_tokenizer tok = stufflib_tokenizer_create(&data, &delimiter);
  stufflib_iterator iter = stufflib_tokenizer_iter(&tok);

  assert(!iter.is_done(&iter));
  {
    stufflib_data* token = iter.get_item(&iter);
    assert(token);
    assert(token->size == 3);
    assert(token->data == data_str);
  }
  iter.advance(&iter);

  assert(!iter.is_done(&iter));
  {
    stufflib_data* token = iter.get_item(&iter);
    assert(token);
    assert(token->size == 2);
    assert(token->data == data_str + 5);
  }
  iter.advance(&iter);

  assert(!iter.is_done(&iter));
  {
    stufflib_data* token = iter.get_item(&iter);
    assert(token);
    assert(token->size == 4);
    assert(token->data == data_str + 9);
  }
  iter.advance(&iter);

  assert(!iter.is_done(&iter));
  {
    stufflib_data* token = iter.get_item(&iter);
    assert(token);
    assert(token->size == 1);
    assert(token->data == data_str + 15);
  }
  iter.advance(&iter);

  assert(!iter.is_done(&iter));
  {
    stufflib_data* token = iter.get_item(&iter);
    assert(token);
    assert(!token->size);
    assert(!token->data);
  }
  iter.advance(&iter);

  assert(iter.is_done(&iter));

  return true;
}

STUFFLIB_TEST_MAIN(test_tokenize_empty,
                   test_tokenize_one,
                   test_tokenize_many,
                   test_tokenize_delimiters,
                   test_tokenize_iter)
