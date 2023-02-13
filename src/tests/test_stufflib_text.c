#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_test.h"
#include "stufflib_text.h"

int test_size() {
  stufflib_text text = (stufflib_text){.str = calloc(1, 1), .length = 0};

  assert(text.str);
  assert(stufflib_text_size(&text) == 0);

  text.str = realloc(text.str, 6);
  assert(text.str);
  memcpy(text.str, "hello", 5);
  text.length = 5;
  assert(stufflib_text_size(&text) == 5);

  free(text.str);
  return 1;
}

int test_count() {
  stufflib_text text = (stufflib_text){.str = calloc(1, 1), .length = 0};

  assert(text.str);
  assert(!text.next);
  assert(stufflib_text_count(&text) == 1);

  text.next = &(stufflib_text){0};
  assert(text.str);
  assert(text.next);
  assert(!text.next->next);
  assert(stufflib_text_count(&text) == 2);

  free(text.str);
  return 1;
}

int test_append_str() {
  stufflib_text text = (stufflib_text){.str = calloc(1, 1), .length = 0};

  stufflib_text_append_str(&text, "hello", 5);
  assert(strcmp(text.str, "hello") == 0);
  assert(text.length == strlen("hello"));
  assert(!text.next);
  assert(stufflib_text_size(&text) == strlen("hello"));
  assert(stufflib_text_count(&text) == 1);

  stufflib_text_append_str(&text, " there", 1);
  assert(strcmp(text.str, "hello ") == 0);
  assert(text.length == strlen("hello "));
  assert(!text.next);
  assert(stufflib_text_size(&text) == strlen("hello "));
  assert(stufflib_text_count(&text) == 1);

  stufflib_text_append_str(&text, "there", 5);
  assert(strcmp(text.str, "hello there") == 0);
  assert(text.length == strlen("hello there"));
  assert(!text.next);
  assert(stufflib_text_size(&text) == strlen("hello there"));
  assert(stufflib_text_count(&text) == 1);

  free(text.str);
  return 1;
}

int test_split() {
  stufflib_text text = (stufflib_text){.str = "\n\n\n", .length = 3};

  stufflib_text* lines = stufflib_text_split(&text, "\n");
  assert(stufflib_text_size(lines) == 0);
  assert(stufflib_text_count(lines) == 4);
  stufflib_text* line = lines;
  for (size_t i = 0; i < 4; ++i) {
    assert(line);
    assert(line->str);
    assert(strcmp(line->str, "") == 0);
    line = line->next;
  }

  stufflib_text_destroy(lines);
  return 1;
}
int test_init_from_str() {
  stufflib_text* text = stufflib_text_from_str("hello");

  assert(text);
  assert(text->str);
  assert(text->length == strlen("hello"));
  assert(!text->next);

  stufflib_text_destroy(text);
  return 1;
}

int test_init_from_file() {
  const char* const path = "./src/tests/test_stufflib_text.c";
  stufflib_text* text = stufflib_text_from_file(path);
  assert(text);
  assert(text->str);
  assert(text->length == stufflib_io_file_size(path));
  assert(!text->next);
  stufflib_text_destroy(text);
  return 1;
}

int test_init_from_file_and_splitlines() {
  const char* const path = "./test-data/numbers.txt";
  stufflib_text* text = stufflib_text_from_file(path);
  assert(text);
  assert(text->str);
  assert(text->length == stufflib_io_file_size(path));

  stufflib_text* lines = stufflib_text_split(text, "\n");
  stufflib_text* line = lines;
  for (size_t i = 1; i <= 100; ++i) {
    char expected[100] = {0};
    snprintf(expected, 4, "%zu", i);
    assert(line);
    assert(line->str);
    assert(strcmp(line->str, expected) == 0);
    line = line->next;
  }

  stufflib_text_destroy(text);
  stufflib_text_destroy(lines);
  return 1;
}

STUFFLIB_TEST_MAIN(test_size,
                   test_count,
                   test_append_str,
                   test_split,
                   test_init_from_str,
                   test_init_from_file,
                   test_init_from_file_and_splitlines)
