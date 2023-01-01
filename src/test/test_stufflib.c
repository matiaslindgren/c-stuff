#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_math.h"
#include "stufflib_rand.h"

int test_stufflib_argv_parse_flag() {
  {
    int argc = 3;
    char* const argv[4] = {
        "bin/path",
        "-w",
        "-v",
        0,
    };
    if (!stufflib_argv_parse_flag(argc, argv, "-v")) {
      return 1;
    }
    if (!stufflib_argv_parse_flag(argc, argv, "-w")) {
      return 1;
    }
    const char* should_not_match[] = {
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
    for (size_t i = 0;
         i < sizeof(should_not_match) / sizeof(should_not_match[0]);
         ++i) {
      if (stufflib_argv_parse_flag(argc, argv, should_not_match[i])) {
        return 1;
      }
    }
  }
  {
    int argc = 2;
    char* const argv[3] = {
        "bin/path",
        "-x",
        0,
    };
    if (!stufflib_argv_parse_flag(argc, argv, "-x")) {
      return 1;
    }
    const char* should_not_match[] = {
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
    for (size_t i = 0;
         i < sizeof(should_not_match) / sizeof(should_not_match[0]);
         ++i) {
      if (stufflib_argv_parse_flag(argc, argv, should_not_match[i])) {
        return 1;
      }
    }
  }
  return 0;
}

int test_stufflib_rand_fill() {
  const size_t n = 1000;
  double x[n];
  stufflib_rand_fill_double(n, x, 0);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return 1;
    }
  }
  stufflib_rand_fill_double(n, x, 10);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 10) {
      return 1;
    }
  }
  return 0;
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
      return 1;
    }
  }
  stufflib_rand_set_zero_double(n, x, 1);
  for (size_t i = 0; i < n; ++i) {
    if (fabs(x[i]) > 0) {
      return 1;
    }
  }
  return 0;
}

int test_stufflib_math_double_almost() {
  if (!stufflib_math_double_almost(1, 1.5, 1)) {
    return 1;
  }
  if (stufflib_math_double_almost(1, 1.5, 0.1)) {
    return 1;
  }
  return 0;
}

typedef int test_function(void);

int main(void) {
  test_function* tests[] = {
      test_stufflib_argv_parse_flag,
      test_stufflib_rand_fill,
      test_stufflib_rand_set_zero,
      test_stufflib_math_double_almost,
  };
  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    if (tests[i]()) {
      fprintf(stderr, "test %zu failed\n", i);
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
