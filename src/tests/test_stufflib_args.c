#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"

bool test_parse_positional(const bool) {
  char* const argv[] = {
      "bin/path",
      "arg1",
      "arg2",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  assert(sl_args_count_positional(&args) == 2);
  assert(sl_args_count_optional(&args) == 0);

  assert(sl_args_get_positional(&args, 0) != nullptr);
  assert(strcmp(sl_args_get_positional(&args, 0), argv[1]) == 0);
  assert(sl_args_get_positional(&args, 1) != nullptr);
  assert(strcmp(sl_args_get_positional(&args, 1), argv[2]) == 0);
  assert(sl_args_get_positional(&args, 2) == nullptr);

  return true;
}

bool test_parse_one_flag(const bool) {
  char* const argv[] = {
      "bin/path",
      "-x",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  assert(sl_args_count_positional(&args) == 0);
  assert(sl_args_get_positional(&args, 0) == nullptr);

  assert(sl_args_count_optional(&args) == 1);
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

  return true;
}

bool test_parse_positional_after_optional(const bool) {
  char* const argv[] = {
      "bin/path",
      "-v",
      "arg1",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  assert(sl_args_count_positional(&args) == 1);
  assert(sl_args_count_optional(&args) == 1);

  assert(sl_args_get_positional(&args, 0) != nullptr);
  assert(strcmp(sl_args_get_positional(&args, 0), argv[2]) == 0);
  assert(sl_args_get_positional(&args, 1) == nullptr);

  assert(sl_args_find_optional(&args, argv[1]) != nullptr);
  assert(strcmp(sl_args_find_optional(&args, argv[1]), argv[1]) == 0);

  assert(sl_args_parse_flag(&args, "-v"));

  return true;
}

bool test_parse_two_flags(const bool) {
  char* const argv[] = {
      "bin/path",
      "-w",
      "-v",
      0,
  };
  const int argc = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  assert(sl_args_count_positional(&args) == 0);
  assert(sl_args_count_optional(&args) == 2);

  assert(sl_args_get_positional(&args, 0) == nullptr);

  for (int i = 1; i < argc; ++i) {
    assert(sl_args_find_optional(&args, argv[i]) != nullptr);
    assert(strcmp(sl_args_find_optional(&args, argv[i]), argv[i]) == 0);
  }

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

  return true;
}

bool test_parse_optional_ints(const bool) {
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
  struct sl_args args = {.argc = argc, .argv = argv};

  assert(sl_args_count_positional(&args) == 0);
  assert(sl_args_count_optional(&args) == 5);

  assert(sl_args_get_positional(&args, 0) == nullptr);

  for (int i = 1; i < argc; ++i) {
    assert(sl_args_find_optional(&args, argv[i]) != nullptr);
    assert(strcmp(sl_args_find_optional(&args, argv[i]), argv[i]) == 0);
  }

  assert(sl_args_parse_ull(&args, "-w", 10) == 1);
  assert(sl_args_parse_ull(&args, "-w", 8) == 1);
  assert(sl_args_parse_ull(&args, "-w", 16) == 1);
  assert(sl_args_parse_ull(&args, "-v", 10) == 10);
  assert(sl_args_parse_ull(&args, "-v", 8) == 8);
  assert(sl_args_parse_ull(&args, "-v", 16) == 16);
  assert(sl_args_parse_ull(&args, "--num-stuff", 10) == 100);
  assert(sl_args_parse_ull(&args, "--num-stuff", 8) == 64);
  assert(sl_args_parse_ull(&args, "--num-stuff", 16) == 256);

  char* const should_not_match[] = {
      "-f",
      "--flag",
      "",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(should_not_match); ++i) {
    assert(sl_args_parse_ull(&args, should_not_match[i], 10) == 0);
    assert(sl_args_parse_ull(&args, should_not_match[i], 8) == 0);
    assert(sl_args_parse_ull(&args, should_not_match[i], 16) == 0);
  }

  return true;
}

SL_TEST_MAIN(test_parse_positional,
             test_parse_one_flag,
             test_parse_positional_after_optional,
             test_parse_two_flags,
             test_parse_optional_ints)
