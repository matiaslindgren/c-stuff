#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/iterator/iterator.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>
#include <stufflib/testing/testing.h>
#include <stufflib/unicode/unicode.h>

SL_TEST(test_data) {
  (void)ctx;
  (void)verbose;
  struct sl_span data = {
      .size = 5,
      .data = (unsigned char[5]){0, 1, 2, 3, 4},
  };
  size_t i                = 0;
  struct sl_iterator iter = sl_span_iter(&data);
  for (; !sl_span_iter_is_done(&iter); sl_span_iter_advance(&iter)) {
    SL_ASSERT_TRUE(iter.index == i);
    const unsigned char* item = sl_span_iter_get(&iter);
    SL_ASSERT_TRUE(item);
    SL_ASSERT_TRUE(*item == data.data[i]);
    ++i;
  }
  SL_ASSERT_TRUE(iter.index == data.size);
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

SL_TEST(test_cstr) {
  (void)ctx;
  (void)verbose;
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
    size_t i                = 0;
    char* str               = strings[i_str];
    struct sl_iterator iter = sl_cstr_iter(str);
    for (; !sl_cstr_iter_is_done(&iter); sl_cstr_iter_advance(&iter)) {
      SL_ASSERT_TRUE(iter.index == i);
      const char* ch = sl_cstr_iter_get(&iter);
      SL_ASSERT_TRUE(ch);
      SL_ASSERT_TRUE(*ch == str[i]);
      ++i;
    }
    SL_ASSERT_TRUE(iter.index == strlen(str));
  }
  return true;
}

SL_TEST_MAIN()
