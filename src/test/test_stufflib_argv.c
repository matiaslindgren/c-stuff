#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_test.h"

int test_stufflib_argv_parse_flag(const int verbose) {
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

int test_stufflib_argv_parse_uint(const int verbose) {
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
    if (stufflib_argv_parse_uint(argc, argv, "-w", 10) != 1) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "-w", 8) != 1) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "-w", 16) != 1) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "-v", 10) != 10) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "-v", 8) != 8) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "-v", 16) != 16) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "--num-stuff", 10) != 100) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "--num-stuff", 8) != 64) {
      return 0;
    }
    if (stufflib_argv_parse_uint(argc, argv, "--num-stuff", 16) != 256) {
      return 0;
    }
    char* const should_not_match[] = {
        "-f",
        "--flag",
        "",
    };
    for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(should_not_match); ++i) {
      if (stufflib_argv_parse_uint(argc, argv, should_not_match[i], 10)) {
        return 0;
      }
    }
  }
  return 1;
}

STUFFLIB_TEST_MAIN(test_stufflib_argv_parse_flag,
                   test_stufflib_argv_parse_uint);
