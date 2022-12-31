#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_text.h"

int _test_init_empty() {
  struct stufflib_text* text = stufflib_text_init();
  assert(text);
  assert(text->next == 0);
  assert(text->lineno == 0);
  assert(text->data == 0);
  assert(text->length == 0);
  stufflib_text_destroy(text);
  return 0;
}

int _test_init_from_str() {
  struct stufflib_text* text = stufflib_text_from_str("hello there");
  assert(text);
  assert(text->next == 0);
  assert(text->lineno == 1);
  assert(text->data);
  assert(text->length == strlen("hello there"));
  stufflib_text_destroy(text);
  return 0;
}

const size_t test_split_lineno;
const size_t test_linecount;

int _test_init_from_file() {
  struct stufflib_text* text =
      stufflib_text_from_file("./src/test/test_text_processor.c");

  const size_t split_lineno = test_split_lineno;
  const size_t line_count = test_linecount;

  assert(text);
  assert(text->data);
  assert(text->lineno == 1);
  assert(text->length == strlen("#include <assert.h>"));
  assert(stufflib_text_line_count(text) == line_count);
  assert(text->next);
  assert(text->next->data);
  assert(text->next->lineno == 2);
  assert(text->next->length == strlen("#include <stdio.h>"));

  struct stufflib_text* lhs = text;
  struct stufflib_text* rhs = stufflib_text_split_after(lhs, split_lineno);
  assert(lhs);
  assert(lhs->data);
  assert(lhs->lineno == 1);
  assert(lhs->length == strlen("#include <assert.h>"));
  assert(stufflib_text_line_count(lhs) == split_lineno);
  assert(rhs);
  assert(rhs->next);
  assert(rhs->data);
  assert(rhs->lineno == 1);
  assert(rhs->length == strlen("int main(void) {"));
  assert(stufflib_text_line_count(rhs) == line_count - split_lineno);

  text = stufflib_text_concat(lhs, rhs);
  assert(text);
  assert(text->next);
  assert(text->data);
  assert(text->lineno == 1);
  assert(text->length == strlen("#include <assert.h>"));
  assert(stufflib_text_line_count(text) == line_count);

  stufflib_text_destroy(text);
  return 0;
}

const size_t test_split_lineno = __LINE__;
int main(void) {
  int (*tests[])(void) = {
      _test_init_empty,
      _test_init_from_str,
      _test_init_from_file,
  };
  for (size_t t = 0; t < 3; ++t) {
    if (tests[t]()) {
      fprintf(stderr, "test %zu failed\n", t + 1);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
const size_t test_linecount = __LINE__;
