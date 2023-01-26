#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_io.h"
#include "stufflib_macros.h"
#include "stufflib_text.h"

const size_t test_main_lineno;
const size_t test_line_count;

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
  const char* const path = "./src/test/test_stufflib_text.c";
  stufflib_text* text = stufflib_text_from_file(path);
  assert(text);
  assert(text->str);
  assert(text->length == stufflib_io_file_size(path));
  assert(!text->next);
  stufflib_text_destroy(text);
  return 1;
}

int test_init_from_file_and_splitlines() {
  const char* const path = "./src/test/test_stufflib_text.c";
  stufflib_text* text = stufflib_text_from_file(path);
  assert(text);
  assert(text->str);
  assert(text->length == stufflib_io_file_size(path));

  stufflib_text* lines = stufflib_text_split(text, "\n");
  const char* expected[] = {
      "#include <assert.h>",
      "#include <stdio.h>",
      "#include <stdlib.h>",
      "",
      "#include \"stufflib_argv.h\"",
      "#include \"stufflib_io.h\"",
  };
  stufflib_text* line = lines;
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(expected); ++i) {
    assert(line);
    assert(line->str);
    assert(strcmp(line->str, expected[i]) == 0);
    line = line->next;
  }

  stufflib_text_destroy(text);
  stufflib_text_destroy(lines);
  return 1;
}

const size_t test_main_lineno = __LINE__ + 1;

int main(int argc, char* const argv[argc + 1]) {
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");
  int (*tests[])(int) = {
      test_size,
      test_count,
      test_append_str,
      test_split,
      test_init_from_str,
      test_init_from_file,
      test_init_from_file_and_splitlines,
  };
  const char* test_names[] = {
      "test_size",
      "test_count",
      "test_append_str",
      "test_split",
      "test_init_from_str",
      "test_init_from_file",
      "test_init_from_file_and_splitlines",
  };
  for (size_t t = 0; t < STUFFLIB_ARRAY_LEN(tests); ++t) {
    if (verbose) {
      printf("\n%s\n", test_names[t]);
    }
    if (!tests[t](verbose)) {
      STUFFLIB_PRINT_ERROR("test %s (%zu) failed", test_names[t], t);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
const size_t test_line_count = __LINE__;
