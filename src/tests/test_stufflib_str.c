#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_str.h"
#include "stufflib_test.h"

bool test_no_splits(const bool verbose) {
  char** chunks = stufflib_str_split("abc", " ");
  assert(strcmp(chunks[0], "abc") == 0);
  assert(chunks[1] == 0);
  stufflib_str_chunks_destroy(chunks);
  return true;
}

bool test_1_split(const bool verbose) {
  char** chunks = stufflib_str_split("abc", "b");
  assert(strcmp(chunks[0], "a") == 0);
  assert(strcmp(chunks[1], "c") == 0);
  assert(chunks[2] == 0);
  stufflib_str_chunks_destroy(chunks);
  return true;
}

bool test_2_splits(const bool verbose) {
  char** chunks = stufflib_str_split("1\n2\n3", "\n");
  assert(strcmp(chunks[0], "1") == 0);
  assert(strcmp(chunks[1], "2") == 0);
  assert(strcmp(chunks[2], "3") == 0);
  assert(chunks[3] == 0);
  stufflib_str_chunks_destroy(chunks);
  return true;
}

bool test_slice(const bool verbose) {
  char* chunks[] = {
      "one",
      "two",
      "three",
      "",
      "four",
      "five",
  };

  {
    char** slice = stufflib_str_slice_chunks(chunks, 0, 0);
    assert(slice[0] == 0);
    stufflib_str_chunks_destroy(slice);
  }

  {
    char** slice = stufflib_str_slice_chunks(chunks, 0, 1);
    assert(strcmp(slice[0], "one") == 0);
    assert(slice[1] == 0);
    stufflib_str_chunks_destroy(slice);
  }

  {
    char** slice = stufflib_str_slice_chunks(chunks, 0, 2);
    assert(strcmp(slice[0], "one") == 0);
    assert(strcmp(slice[1], "two") == 0);
    assert(slice[2] == 0);
    stufflib_str_chunks_destroy(slice);
  }

  {
    char** slice = stufflib_str_slice_chunks(chunks, 1, 2);
    assert(strcmp(slice[0], "two") == 0);
    assert(slice[1] == 0);
    stufflib_str_chunks_destroy(slice);
  }

  {
    char** slice = stufflib_str_slice_chunks(chunks, 3, 5);
    assert(strcmp(slice[0], "") == 0);
    assert(strcmp(slice[1], "four") == 0);
    assert(slice[2] == 0);
    stufflib_str_chunks_destroy(slice);
  }

  {
    const size_t n = STUFFLIB_ARRAY_LEN(chunks);
    char** slice = stufflib_str_slice_chunks(chunks, 0, n);
    for (size_t i = 0; i < n; ++i) {
      assert(strcmp(slice[i], chunks[i]) == 0);
    }
    assert(slice[n] == 0);
    stufflib_str_chunks_destroy(slice);
  }

  return true;
}

bool test_split_set(const bool verbose) {
  char** chunks = stufflib_str_split_any("abcdeefgghhiiikkij",
                                         4,
                                         (const char*[]){"c", "e", "f", "i"});
  const char* expected[] = {"ab", "d", "", "", "gghh", "", "", "kk", "j"};
  const size_t num_chunks = STUFFLIB_ARRAY_LEN(expected);
  for (size_t i = 0; i < num_chunks; ++i) {
    assert(chunks[i]);
    assert(strcmp(chunks[i], expected[i]) == 0);
  }
  assert(chunks[num_chunks] == 0);
  stufflib_str_chunks_destroy(chunks);
  return true;
}

bool test_split_whitespace(const bool verbose) {
  char** chunks =
      stufflib_str_split_whitespace("ab c\td  e \t\t f\ng\n\nhi\n\nj k \tl");
  const char* expected[] = {
      "ab",
      "c",
      "d",
      "",
      "e",
      "",
      "",
      "",
      "f",
      "g",
      "",
      "hi",
      "",
      "j",
      "k",
      "",
      "l",
  };
  const size_t num_chunks = STUFFLIB_ARRAY_LEN(expected);
  for (size_t i = 0; i < num_chunks; ++i) {
    assert(chunks[i]);
    assert(strcmp(chunks[i], expected[i]) == 0);
  }
  assert(chunks[num_chunks] == 0);
  stufflib_str_chunks_destroy(chunks);
  return true;
}

STUFFLIB_TEST_MAIN(test_no_splits,
                   test_1_split,
                   test_2_splits,
                   test_slice,
                   test_split_set,
                   test_split_whitespace)
