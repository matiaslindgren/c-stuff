#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_str.h"
#include "stufflib_test.h"

int test_no_splits() {
  char** chunks = stufflib_str_split("abc", " ");
  assert(strcmp(*chunks, "abc") == 0);
  assert(*(chunks + 1) == 0);
  stufflib_str_split_destroy(chunks);
  return 1;
}

int test_1_split() {
  char** chunks = stufflib_str_split("abc", "b");
  assert(strcmp(*(chunks + 0), "a") == 0);
  assert(strcmp(*(chunks + 1), "c") == 0);
  assert(*(chunks + 2) == 0);
  stufflib_str_split_destroy(chunks);
  return 1;
}

int test_2_splits() {
  char** chunks = stufflib_str_split("1\n2\n3", "\n");
  assert(strcmp(*(chunks + 0), "1") == 0);
  assert(strcmp(*(chunks + 1), "2") == 0);
  assert(strcmp(*(chunks + 2), "3") == 0);
  assert(*(chunks + 3) == 0);
  stufflib_str_split_destroy(chunks);
  return 1;
}

STUFFLIB_TEST_MAIN(test_no_splits, test_1_split, test_2_splits)
