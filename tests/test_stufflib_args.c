#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/macros/macros.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_parse_positional) {
  (void)ctx;
  (void)verbose;
  char* const argv[] = {
      "bin/path",
      "arg1",
      "arg2",
      0,
  };
  const int argc      = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  SL_ASSERT_TRUE(sl_args_count_positional(&args) == 2);
  SL_ASSERT_TRUE(sl_args_count_optional(&args) == 0);

  SL_ASSERT_TRUE(sl_args_get_positional(&args, 0) != nullptr);
  SL_ASSERT_TRUE(strcmp(sl_args_get_positional(&args, 0), argv[1]) == 0);
  SL_ASSERT_TRUE(sl_args_get_positional(&args, 1) != nullptr);
  SL_ASSERT_TRUE(strcmp(sl_args_get_positional(&args, 1), argv[2]) == 0);
  SL_ASSERT_TRUE(sl_args_get_positional(&args, 2) == nullptr);

  return true;
}

SL_TEST(test_parse_one_flag) {
  (void)ctx;
  (void)verbose;
  char* const argv[] = {
      "bin/path",
      "-x",
      0,
  };
  const int argc      = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  SL_ASSERT_TRUE(sl_args_count_positional(&args) == 0);
  SL_ASSERT_TRUE(sl_args_get_positional(&args, 0) == nullptr);

  SL_ASSERT_TRUE(sl_args_count_optional(&args) == 1);
  SL_ASSERT_TRUE(sl_args_parse_flag(&args, "-x"));

  char* const should_not_match[] = {
      "-vv",
      "-wvv",
      "v",
      "- v",
      "- w",
      "--w",
      "--v",
      "--w-1",
      "--v-abc",
      "",
      "-w",
      "-v",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(should_not_match); ++i) {
    SL_ASSERT_TRUE(!sl_args_parse_flag(&args, should_not_match[i]));
  }

  return true;
}

SL_TEST(test_parse_positional_after_optional) {
  (void)ctx;
  (void)verbose;
  char* const argv[] = {
      "bin/path",
      "-v",
      "arg1",
      0,
  };
  const int argc      = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  SL_ASSERT_TRUE(sl_args_count_positional(&args) == 1);
  SL_ASSERT_TRUE(sl_args_count_optional(&args) == 1);

  SL_ASSERT_TRUE(sl_args_get_positional(&args, 0) != nullptr);
  SL_ASSERT_TRUE(strcmp(sl_args_get_positional(&args, 0), argv[2]) == 0);
  SL_ASSERT_TRUE(sl_args_get_positional(&args, 1) == nullptr);

  SL_ASSERT_TRUE(sl_args_find_optional(&args, argv[1]) != nullptr);
  SL_ASSERT_TRUE(strcmp(sl_args_find_optional(&args, argv[1]), argv[1]) == 0);

  SL_ASSERT_TRUE(sl_args_parse_flag(&args, "-v"));

  return true;
}

SL_TEST(test_parse_two_flags) {
  (void)ctx;
  (void)verbose;
  char* const argv[] = {
      "bin/path",
      "-w",
      "-v",
      0,
  };
  const int argc      = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  SL_ASSERT_TRUE(sl_args_count_positional(&args) == 0);
  SL_ASSERT_TRUE(sl_args_count_optional(&args) == 2);

  SL_ASSERT_TRUE(sl_args_get_positional(&args, 0) == nullptr);

  for (int i = 1; i < argc; ++i) {
    SL_ASSERT_TRUE(sl_args_find_optional(&args, argv[i]) != nullptr);
    SL_ASSERT_TRUE(strcmp(sl_args_find_optional(&args, argv[i]), argv[i]) == 0);
  }

  SL_ASSERT_TRUE(sl_args_parse_flag(&args, "-w"));
  SL_ASSERT_TRUE(sl_args_parse_flag(&args, "-v"));

  char* const should_not_match[] = {
      "-x",
      "-vv",
      "-wvv",
      "v",
      "- v",
      "- w",
      "--w",
      "--v",
      "--w-v",
      "--v-w",
      "",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(should_not_match); ++i) {
    SL_ASSERT_TRUE(!sl_args_parse_flag(&args, should_not_match[i]));
  }

  return true;
}

SL_TEST(test_parse_optional_ints) {
  (void)ctx;
  (void)verbose;
  char* const argv[] = {
      "bin/path",
      "-w=1",
      "-f",
      "--flag",
      "-v=10",
      "--num-stuff=100",
      0,
  };
  const int argc      = SL_ARRAY_LEN(argv) - 1;
  struct sl_args args = {.argc = argc, .argv = argv};

  SL_ASSERT_TRUE(sl_args_count_positional(&args) == 0);
  SL_ASSERT_TRUE(sl_args_count_optional(&args) == 5);

  SL_ASSERT_TRUE(sl_args_get_positional(&args, 0) == nullptr);

  for (int i = 1; i < argc; ++i) {
    SL_ASSERT_TRUE(sl_args_find_optional(&args, argv[i]) != nullptr);
    SL_ASSERT_TRUE(strcmp(sl_args_find_optional(&args, argv[i]), argv[i]) == 0);
  }

  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "-w", 10) == 1);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "-w", 8) == 1);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "-w", 16) == 1);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "-v", 10) == 10);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "-v", 8) == 8);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "-v", 16) == 16);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "--num-stuff", 10) == 100);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "--num-stuff", 8) == 64);
  SL_ASSERT_TRUE(sl_args_parse_ull(&args, "--num-stuff", 16) == 256);

  char* const should_not_match[] = {
      "-f",
      "--flag",
      "",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(should_not_match); ++i) {
    SL_ASSERT_TRUE(sl_args_parse_ull(&args, should_not_match[i], 10) == 0);
    SL_ASSERT_TRUE(sl_args_parse_ull(&args, should_not_match[i], 8) == 0);
    SL_ASSERT_TRUE(sl_args_parse_ull(&args, should_not_match[i], 16) == 0);
  }

  return true;
}

SL_TEST_MAIN()
