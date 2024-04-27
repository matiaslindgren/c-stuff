#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"
#include "stufflib_span.h"
#include "stufflib_tokenizer.h"

bool test_tokenize_empty(const bool verbose) {
  struct sl_span empty1 = {0};
  struct sl_span empty2 = {0};
  unsigned char x[] = {1, 2, 3};
  const size_t n = SL_ARRAY_LEN(x);
  struct sl_span data = sl_span_view(n, x);

  struct sl_span token1 = sl_tokenizer_next_token(&empty1, &empty2, 0);
  assert(!token1.size);
  assert(!token1.data);

  struct sl_span token2 = sl_tokenizer_next_token(&data, &empty1, 0);
  assert(token2.size == 3);
  assert(token2.data == x);

  struct sl_span token3 = sl_tokenizer_next_token(&empty1, &data, 0);
  assert(!token3.size);
  assert(!token3.data);

  return true;
}

bool test_tokenize_one(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 1, 2, 3};
  unsigned char x2[] = {2};
  const size_t n1 = SL_ARRAY_LEN(x1);
  const size_t n2 = SL_ARRAY_LEN(x2);
  struct sl_span data1 = sl_span_view(n1, x1);
  struct sl_span data2 = sl_span_view(n2, x2);

  struct sl_span token1 = sl_tokenizer_next_token(&data1, &data2, 0);
  assert(token1.size == 1);
  assert(token1.data == x1);

  struct sl_span token2 = sl_tokenizer_next_token(&data1, &data2, 1);
  assert(!token2.size);
  assert(!token2.data);

  struct sl_span token3 = sl_tokenizer_next_token(&data1, &data2, 2);
  assert(token3.size == 2);
  assert(token3.data == x1 + 2);
  assert(token3.data + 1 == x1 + 3);

  return true;
}

bool test_tokenize_many(const bool verbose) {
  unsigned char x1[] = {1, 2, 3, 1, 2, 3};
  unsigned char x2[] = {2, 3, 1};
  const size_t n1 = SL_ARRAY_LEN(x1);
  const size_t n2 = SL_ARRAY_LEN(x2);
  struct sl_span data1 = sl_span_view(n1, x1);
  struct sl_span data2 = sl_span_view(n2, x2);

  struct sl_span token1 = sl_tokenizer_next_token(&data1, &data2, 0);
  assert(token1.size == 1);
  assert(token1.data == x1);

  struct sl_span token2 = sl_tokenizer_next_token(&data1, &data2, 1);
  assert(!token2.size);
  assert(!token2.data);

  struct sl_span token3 = sl_tokenizer_next_token(&data1, &data2, 2);
  assert(token3.size == 4);
  for (size_t i = 0; i < 4; ++i) {
    assert(token3.data + i == x1 + 2 + i);
  }

  return true;
}

bool test_tokenize_delimiters(const bool verbose) {
  unsigned char x1[] = {0, 0, 0};
  unsigned char x2[] = {0};
  const size_t n1 = SL_ARRAY_LEN(x1);
  const size_t n2 = SL_ARRAY_LEN(x2);
  struct sl_span data1 = sl_span_view(n1, x1);
  struct sl_span data2 = sl_span_view(n2, x2);

  for (size_t begin = 0; begin < 3; ++begin) {
    struct sl_span token = sl_tokenizer_next_token(&data1, &data2, begin);
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
  struct sl_span data = sl_span_view(SL_ARRAY_LEN(data_str), data_str);
  struct sl_span delimiter =
      sl_span_view(SL_ARRAY_LEN(delimiter_str), delimiter_str);

  struct sl_tokenizer tok = sl_tokenizer_create(&data, &delimiter);
  struct sl_iterator iter = sl_tokenizer_iter(&tok);

  assert(!sl_tokenizer_iter_is_done(&iter));
  {
    struct sl_span* token = sl_tokenizer_iter_get(&iter);
    assert(token);
    assert(token->size == 3);
    assert(token->data == data_str);
  }
  sl_tokenizer_iter_advance(&iter);

  assert(!sl_tokenizer_iter_is_done(&iter));
  {
    struct sl_span* token = sl_tokenizer_iter_get(&iter);
    assert(token);
    assert(token->size == 2);
    assert(token->data == data_str + 5);
  }
  sl_tokenizer_iter_advance(&iter);

  assert(!sl_tokenizer_iter_is_done(&iter));
  {
    struct sl_span* token = sl_tokenizer_iter_get(&iter);
    assert(token);
    assert(token->size == 4);
    assert(token->data == data_str + 9);
  }
  sl_tokenizer_iter_advance(&iter);

  assert(!sl_tokenizer_iter_is_done(&iter));
  {
    struct sl_span* token = sl_tokenizer_iter_get(&iter);
    assert(token);
    assert(token->size == 1);
    assert(token->data == data_str + 15);
  }
  sl_tokenizer_iter_advance(&iter);

  assert(!sl_tokenizer_iter_is_done(&iter));
  {
    struct sl_span* token = sl_tokenizer_iter_get(&iter);
    assert(token);
    assert(!token->size);
    assert(!token->data);
  }
  sl_tokenizer_iter_advance(&iter);

  assert(sl_tokenizer_iter_is_done(&iter));

  return true;
}

SL_TEST_MAIN(test_tokenize_empty,
             test_tokenize_one,
             test_tokenize_many,
             test_tokenize_delimiters,
             test_tokenize_iter)
