#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_io.h"
#include "stufflib_test.h"

int test_file_size() {
  assert(stufflib_io_file_size("./test-data/empty") == 0);
  assert(stufflib_io_file_size("./test-data/one.txt") == 1);
  assert(stufflib_io_file_size("./test-data/hello.txt") == 11);
  assert(stufflib_io_file_size("./test-data/asan.png") == 24733);
  return 1;
}

int test_file_slurp() {
  {
    char* const str = stufflib_io_slurp_file("./test-data/empty");
    assert(strcmp(str, "") == 0);
    free(str);
  }
  {
    char* const str = stufflib_io_slurp_file("./test-data/one.txt");
    assert(strcmp(str, "1") == 0);
    free(str);
  }
  {
    char* const str = stufflib_io_slurp_file("./test-data/hello.txt");
    assert(strcmp(str, "hello\nthere") == 0);
    free(str);
  }
  return 1;
}

STUFFLIB_TEST_MAIN(test_file_size, test_file_slurp);
