#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_io.h"
#include "stufflib_test.h"

bool test_file_size(const bool verbose) {
  assert(stufflib_io_file_size("./test-data/txt/empty") == 0);
  assert(stufflib_io_file_size("./test-data/txt/one.txt") == 1);
  assert(stufflib_io_file_size("./test-data/txt/hello.txt") == 11);
  assert(stufflib_io_file_size("./test-data/png/asan.png") == 24733);
  return true;
}

bool test_file_slurp(const bool verbose) {
  {
    char* const str = stufflib_io_slurp_file("./test-data/txt/empty");
    assert(strcmp(str, "") == 0);
    free(str);
  }
  {
    char* const str = stufflib_io_slurp_file("./test-data/txt/one.txt");
    assert(strcmp(str, "1") == 0);
    free(str);
  }
  {
    char* const str = stufflib_io_slurp_file("./test-data/txt/hello.txt");
    assert(strcmp(str, "hello\nthere") == 0);
    free(str);
  }
  return true;
}

STUFFLIB_TEST_MAIN(test_file_size, test_file_slurp)
