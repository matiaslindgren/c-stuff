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
  for (; !stufflib_data_iter_end(&iter); stufflib_data_iter_advance(&iter)) {
    assert(iter.index == i);
    unsigned char value = 0;
    assert(stufflib_data_iter_get(&iter, &value));
    assert(value == data.data[i]);
    ++i;
  }
  assert(iter.index == data.size);
  return true;
}

void* stufflib_cstr_iter_get(stufflib_iterator iter[const static 1], void* ch) {
  ((char*)ch)[0] = ((char*)iter->begin)[iter->index];
  return ch;
}

void stufflib_cstr_iter_advance(stufflib_iterator iter[const static 1]) {
  ++(iter->index);
}

bool stufflib_cstr_iter_end(stufflib_iterator iter[const static 1]) {
  return ((char*)iter->begin)[iter->index] == 0;
}

stufflib_iterator stufflib_cstr_iter(char str[const static 1]) {
  return (stufflib_iterator){
      .index = 0,
      .begin = (void*)str,
      .get = stufflib_cstr_iter_get,
      .advance = stufflib_cstr_iter_advance,
      .end = stufflib_cstr_iter_end,
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
    for (; !stufflib_cstr_iter_end(&iter); stufflib_cstr_iter_advance(&iter)) {
      assert(iter.index == i);
      char ch = 0;
      assert(stufflib_cstr_iter_get(&iter, &ch));
      assert(ch == str[i]);
      ++i;
    }
    assert(iter.index == strlen(str));
  }
  return true;
}

STUFFLIB_TEST_MAIN(test_data, test_cstr)
