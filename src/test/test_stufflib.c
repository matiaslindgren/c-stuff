#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_rand.h"

int test_stufflib_argv_parse_flag() {
  {
    int argc = 3;
    char* const argv[] = {
        "bin/path",
        "-w",
        "-v",
        0,
    };
    if (!stufflib_argv_parse_flag(argc, argv, "-v")) {
      return 0;
    }
    if (!stufflib_argv_parse_flag(argc, argv, "-w")) {
      return 0;
    }
    char* const should_not_match[] = {
        "-x",
        "-vv",
        "-wvv",
        "v",
        "- v",
        "- w",
        "--w",
        "--v",
        "",
    };
    for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(should_not_match); ++i) {
      if (stufflib_argv_parse_flag(argc, argv, should_not_match[i])) {
        return 0;
      }
    }
  }
  {
    int argc = 2;
    char* const argv[] = {
        "bin/path",
        "-x",
        0,
    };
    if (!stufflib_argv_parse_flag(argc, argv, "-x")) {
      return 0;
    }
    char* const should_not_match[] = {
        "-vv",
        "-wvv",
        "v",
        "- v",
        "- w",
        "--w",
        "--v",
        "",
        "-w",
        "-v",
    };
    for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(should_not_match); ++i) {
      if (stufflib_argv_parse_flag(argc, argv, should_not_match[i])) {
        return 0;
      }
    }
  }
  return 1;
}

int test_stufflib_argv_parse_uint() {
  {
    int argc = 8;
    char* const argv[] = {
        "bin/path",
        "-w=1",
        "-w 2",
        "-f",
        "--flag",
        "-v=10",
        "--num-stuff=100",
        "--num-stuff 101",
        0,
    };
    if (stufflib_argv_parse_uint(argc, argv, "-w", 0) != 1) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "-v", 0) != 10) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "--num-stuff", 0) != 100) {
      return 0;
    }
    char* const should_not_match[] = {
        "-f",
        "--flag",
        "",
    };
    for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(should_not_match); ++i) {
      if (stufflib_argv_parse_uint(argc, argv, should_not_match[i], 0xabcdef) !=
          0xabcdef) {
        return 0;
      }
    }
  }
  return 1;
}

int test_stufflib_rand_fill() {
  const size_t n = 1000;
  double x[n];
  stufflib_rand_fill_double(n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return 0;
    }
  }
  stufflib_rand_fill_double(n, x, 10);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 10) {
      return 0;
    }
  }
  return 1;
}

int test_stufflib_rand_set_zero() {
  const size_t n = 1000;
  double x[n];
  for (size_t i = 0; i < n; ++i) {
    x[i] = i + 1;
  }
  stufflib_rand_set_zero_double(n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) < 1) {
      return 0;
    }
  }
  stufflib_rand_set_zero_double(n, x, 1);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return 0;
    }
  }
  return 1;
}

int test_stufflib_math_double_almost() {
  if (!stufflib_math_double_almost(1, 1.5, 1)) {
    return 0;
  }
  if (stufflib_math_double_almost(1, 1.5, 0.1)) {
    return 0;
  }
  return 1;
}

typedef int test_function(void);

int main(void) {
  test_function* tests[] = {
      test_stufflib_argv_parse_flag,
      test_stufflib_argv_parse_uint,
      test_stufflib_rand_fill,
      test_stufflib_rand_set_zero,
      test_stufflib_math_double_almost,
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(tests); ++i) {
    if (!tests[i]()) {
      fprintf(stderr, "test %zu failed\n", i);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
