#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib/args/args.h"
#include "stufflib/iterator/iterator.h"
#include "stufflib/macros/macros.h"
#include "stufflib/span/span.h"
#include "stufflib/unicode/unicode.h"

static bool test_data(const bool) {
  struct sl_span data = {
      .size = 5,
      .data = (unsigned char[5]){0, 1, 2, 3, 4},
  };
  size_t i = 0;
  struct sl_iterator iter = sl_span_iter(&data);
  for (; !sl_span_iter_is_done(&iter); sl_span_iter_advance(&iter)) {
    assert(iter.index == i);
    const unsigned char* item = sl_span_iter_get(&iter);
    assert(item);
    assert(*item == data.data[i]);
    ++i;
  }
  assert(iter.index == data.size);
  return true;
}

void* sl_cstr_iter_get(struct sl_iterator iter[const static 1]) {
  char* str = iter->data;
  return str + iter->index;
}

void sl_cstr_iter_advance(struct sl_iterator iter[const static 1]) {
  ++(iter->index);
}

bool sl_cstr_iter_is_done(struct sl_iterator iter[const static 1]) {
  char* str = iter->data;
  return str[iter->index] == 0;
}

struct sl_iterator sl_cstr_iter(char str[const static 1]) {
  return (struct sl_iterator){.index = 0, .data = str};
}

static bool test_cstr(const bool) {
  char* strings[] = {
      "",
      " ",
      "hello there",
      "\n",
      "\r",
      "\t",
      "\v",
      "0",
  };
  for (size_t i_str = 0; i_str < SL_ARRAY_LEN(strings); ++i_str) {
    size_t i = 0;
    char* str = strings[i_str];
    struct sl_iterator iter = sl_cstr_iter(str);
    for (; !sl_cstr_iter_is_done(&iter); sl_cstr_iter_advance(&iter)) {
      assert(iter.index == i);
      const char* ch = sl_cstr_iter_get(&iter);
      assert(ch);
      assert(*ch == str[i]);
      ++i;
    }
    assert(iter.index == strlen(str));
  }
  return true;
}

SL_TEST_MAIN(test_data, test_cstr)
