#include <assert.h>
#include <string.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"
#include "stufflib_test.h"
#include "stufflib_unicode.h"

bool test_data(const bool verbose) {
  stufflib_data data = {
      .size = 5,
      .data = (unsigned char[5]){0, 1, 2, 3, 4},
  };
  size_t i = 0;
  stufflib_iterator iter = stufflib_data_iter(&data);
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

void* stufflib_cstr_iter_get_item(stufflib_iterator iter[const static 1]) {
  return ((char*)iter->data) + iter->index;
}

void stufflib_cstr_iter_advance(stufflib_iterator iter[const static 1]) {
  ++(iter->index);
}

bool stufflib_cstr_iter_is_done(stufflib_iterator iter[const static 1]) {
  return ((char*)iter->data)[iter->index] == 0;
}

stufflib_iterator stufflib_cstr_iter(char str[const static 1]) {
  return (stufflib_iterator){
      .index = 0,
      .data = (void*)str,
      .get_item = stufflib_cstr_iter_get_item,
      .advance = stufflib_cstr_iter_advance,
      .is_done = stufflib_cstr_iter_is_done,
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
  for (size_t i_str = 0; i_str < STUFFLIB_ARRAY_LEN(strings); ++i_str) {
    size_t i = 0;
    char* str = strings[i_str];
    stufflib_iterator iter = stufflib_cstr_iter(str);
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

STUFFLIB_TEST_MAIN(test_data, test_cstr)
