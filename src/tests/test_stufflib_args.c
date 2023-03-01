#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"

bool test_parse_positional(const bool verbose) {
  char* const argv[] = {
      "bin/path",
      "arg1",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = sl_args_from_argv(argc, argv);

  assert(strcmp(args.program, "bin/path") == 0);
  assert(args.optional[0] == 0);

  assert(args.required[0] != 0);
  assert(args.required[1] == 0);
  assert(strcmp(*args.required[0], "arg1") == 0);

  assert(strcmp(sl_args_get_positional(&args, 0), "arg1") == 0);
  assert(sl_args_count_positional(&args) == 1);

  sl_args_destroy(&args);
  return true;
}

bool test_parse_one_flag(const bool verbose) {
  char* const argv[] = {
      "bin/path",
      "-x",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = sl_args_from_argv(argc, argv);

  assert(strcmp(args.program, "bin/path") == 0);
  assert(args.optional[0] != 0);
  assert(args.optional[1] == 0);
  assert(args.required[0] == 0);

  assert(sl_args_parse_flag(&args, "-x"));

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
  for (size_t i = 0; i < SL_ARRAY_LEN(should_not_match); ++i) {
    assert(!sl_args_parse_flag(&args, should_not_match[i]));
  }

  assert(sl_args_count_positional(&args) == 0);

  sl_args_destroy(&args);
  return true;
}

bool test_parse_positional_after_optional(const bool verbose) {
  char* const argv[] = {
      "bin/path",
      "-v",
      "arg1",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = sl_args_from_argv(argc, argv);

  assert(strcmp(args.program, "bin/path") == 0);

  assert(args.optional[0] != 0);
  assert(args.optional[1] == 0);
  assert(strcmp(*args.optional[0], "-v") == 0);

  assert(args.required[0] != 0);
  assert(args.required[1] == 0);
  assert(strcmp(*args.required[0], "arg1") == 0);

  assert(sl_args_parse_flag(&args, "-v"));
  assert(strcmp(sl_args_get_positional(&args, 0), "arg1") == 0);
  assert(sl_args_count_positional(&args) == 1);

  sl_args_destroy(&args);
  return true;
}

bool test_parse_two_flags(const bool verbose) {
  char* const argv[] = {
      "bin/path",
      "-w",
      "-v",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = sl_args_from_argv(argc, argv);

  assert(strcmp(args.program, "bin/path") == 0);
  assert(args.required[0] == 0);

  assert(args.optional[0] != 0);
  assert(args.optional[1] != 0);
  assert(args.optional[2] == 0);
  assert(strcmp(*args.optional[0], "-w") == 0);
  assert(strcmp(*args.optional[1], "-v") == 0);

  assert(sl_args_parse_flag(&args, "-w"));
  assert(sl_args_parse_flag(&args, "-v"));

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
  for (size_t i = 0; i < SL_ARRAY_LEN(should_not_match); ++i) {
    assert(!sl_args_parse_flag(&args, should_not_match[i]));
  }

  assert(sl_args_count_positional(&args) == 0);

  sl_args_destroy(&args);
  return true;
}

bool test_parse_uint(const bool verbose) {
  char* const argv[] = {
      "bin/path",
      "-w=1",
      "-f",
      "--flag",
      "-v=10",
      "--num-stuff=100",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = sl_args_from_argv(argc, argv);

  assert(strcmp(args.program, "bin/path") == 0);
  assert(args.required[0] == 0);

  for (size_t i = 0; i < argc - 1; ++i) {
    assert(args.optional[i] != 0);
  }
  assert(strcmp(*args.optional[0], "-w=1") == 0);
  assert(strcmp(*args.optional[1], "-f") == 0);
  assert(strcmp(*args.optional[2], "--flag") == 0);
  assert(strcmp(*args.optional[3], "-v=10") == 0);
  assert(strcmp(*args.optional[4], "--num-stuff=100") == 0);

  assert(sl_args_parse_uint(&args, "-w", 10) == 1);
  assert(sl_args_parse_uint(&args, "-w", 8) == 1);
  assert(sl_args_parse_uint(&args, "-w", 16) == 1);
  assert(sl_args_parse_uint(&args, "-v", 10) == 10);
  assert(sl_args_parse_uint(&args, "-v", 8) == 8);
  assert(sl_args_parse_uint(&args, "-v", 16) == 16);
  assert(sl_args_parse_uint(&args, "--num-stuff", 10) == 100);
  assert(sl_args_parse_uint(&args, "--num-stuff", 8) == 64);
  assert(sl_args_parse_uint(&args, "--num-stuff", 16) == 256);

  char* const should_not_match[] = {
      "-f",
      "--flag",
      "",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(should_not_match); ++i) {
    assert(sl_args_parse_uint(&args, should_not_match[i], 10) == 0);
  }

  assert(sl_args_count_positional(&args) == 0);

  sl_args_destroy(&args);
  return true;
}

SL_TEST_MAIN(test_parse_positional,
             test_parse_one_flag,
             test_parse_positional_after_optional,
             test_parse_two_flags,
             test_parse_uint)
