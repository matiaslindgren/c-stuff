#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_unicode.h"

bool test_data(const bool verbose) {
  struct sl_data data = {
      .size = 5,
      .data = (unsigned char[5]){0, 1, 2, 3, 4},
  };
  size_t i = 0;
  struct sl_iterator iter = sl_data_iter(&data);
  for (; !iter.is_done(&iter); iter.advance(&iter)) {
    assert(iter.index == i);
    const unsigned char* item = iter.get_item(&iter);
    assert(item);
    assert(*item == data.data[i]);
    ++i;
  }
  assert(iter.index == data.size);
  return true;
}

void* sl_cstr_iter_get_item(struct sl_iterator iter[const static 1]) {
  return ((char*)iter->data) + iter->index;
}

void sl_cstr_iter_advance(struct sl_iterator iter[const static 1]) {
  ++(iter->index);
}

bool sl_cstr_iter_is_done(struct sl_iterator iter[const static 1]) {
  return ((char*)iter->data)[iter->index] == 0;
}

struct sl_iterator sl_cstr_iter(char str[const static 1]) {
  return (struct sl_iterator){
      .index = 0,
      .data = (void*)str,
      .get_item = sl_cstr_iter_get_item,
      .advance = sl_cstr_iter_advance,
      .is_done = sl_cstr_iter_is_done,
  };
}

bool test_cstr(const bool verbose) {
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
    for (; !iter.is_done(&iter); iter.advance(&iter)) {
      assert(iter.index == i);
      const char* ch = iter.get_item(&iter);
      assert(ch);
      assert(*ch == str[i]);
      ++i;
    }
    assert(iter.index == strlen(str));
  }
  return true;
}

SL_TEST_MAIN(test_data, test_cstr)
