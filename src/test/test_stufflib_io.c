#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_io.h"
#include "stufflib_macros.h"

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

typedef int test_function(void);

int main(int argc, char* const argv[argc + 1]) {
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  test_function* tests[] = {
      test_file_size,
      test_file_slurp,
  };
  const char* test_names[] = {
      "test_file_size",
      "test_file_slurp",
  };

  for (size_t t = 0; t < STUFFLIB_ARRAY_LEN(tests); ++t) {
    if (verbose) {
      printf("\n%s\n", test_names[t]);
    }
    if (!tests[t]()) {
      STUFFLIB_PRINT_ERROR("test %s (%zu) failed", test_names[t], t);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
