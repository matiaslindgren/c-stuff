#ifndef SL_TESTING_H_INCLUDED
#define SL_TESTING_H_INCLUDED

#include <stufflib/math/math.h>

#define SL_FORMAT(x)              \
  _Generic(                       \
      (x),                        \
      bool: "%d",                 \
      char: "%c",                 \
      signed char: "%hhd",        \
      unsigned char: "%hhu",      \
      short: "%hd",               \
      unsigned short: "%hu",      \
      int: "%d",                  \
      unsigned int: "%u",         \
      long: "%ld",                \
      unsigned long: "%lu",       \
      long long: "%lld",          \
      unsigned long long: "%llu", \
      float: "%g",                \
      double: "%g",               \
      void*: "%p",                \
      const void*: "%p"           \
  )

#define SL_ASSERT_BINOP(binop, lhs_expr, rhs_expr, ...)                             \
  do {                                                                              \
    const typeof(lhs_expr) lhs = (lhs_expr);                                        \
    const typeof(lhs_expr) rhs = (typeof(lhs_expr))(rhs_expr);                      \
    if (!(lhs binop rhs)) {                                                         \
      fprintf(                                                                      \
          stderr,                                                                   \
          "assertion failed: (" #lhs_expr ") " #binop " (" #rhs_expr                \
          ")"                                                                       \
          "\n  lhs: " #lhs_expr " = "                                               \
      );                                                                            \
      fprintf(stderr, SL_FORMAT(lhs), lhs);                                         \
      fprintf(stderr, "\n  rhs: " #rhs_expr " = ");                                 \
      fprintf(stderr, SL_FORMAT(rhs), rhs);                                         \
      fprintf(                                                                      \
          stderr,                                                                   \
          "\n  function: %s\n  file: %s\n  line: %d\n  msg: \"" __VA_ARGS__ "\"\n", \
          __func__,                                                                 \
          __FILE__,                                                                 \
          __LINE__                                                                  \
      );                                                                            \
      return false;                                                                 \
    }                                                                               \
  } while (false)

#define SL_ASSERT_STRCMP(expect, lhs, rhs, ...)                                       \
  do {                                                                                \
    const char* const sl_assert_lhs = (const char* const)(lhs);                       \
    const char* const sl_assert_rhs = (const char* const)(rhs);                       \
    if (!(strcmp(sl_assert_lhs, sl_assert_rhs) == expect)) {                          \
      fprintf(                                                                        \
          stderr,                                                                     \
          "assertion failed: strcmp(" #lhs ", " #rhs ") == " #expect "\n  lhs: " #lhs \
          " = \"%s\"\n  rhs: " #rhs                                                   \
          " = \"%s\""                                                                 \
          "\n  function: %s\n  file: %s\n  line: %d\n  msg: \"" __VA_ARGS__ "\"\n",   \
          sl_assert_lhs,                                                              \
          sl_assert_rhs,                                                              \
          __func__,                                                                   \
          __FILE__,                                                                   \
          __LINE__                                                                    \
      );                                                                              \
      return false;                                                                   \
    }                                                                                 \
  } while (false)

#define SL_ASSERT_DOUBLE_ALMOST(lhs, rhs, tolerance, ...)                           \
  do {                                                                              \
    double sl_assert_lhs = (double)(lhs);                                           \
    double sl_assert_rhs = (double)(rhs);                                           \
    if (!(sl_math_double_almost(sl_assert_lhs, sl_assert_rhs, tolerance))) {        \
      fprintf(                                                                      \
          stderr,                                                                   \
          "assertion failed: sl_math_double_almost(" #lhs ", " #rhs ", " #tolerance \
          ")"                                                                       \
          "\n  lhs: " #lhs " = \"%g\"\n  rhs: " #rhs                                \
          " = \"%g\""                                                               \
          "\n  function: %s\n  file: %s\n  line: %d\n  msg: \"" __VA_ARGS__ "\"\n", \
          sl_assert_lhs,                                                            \
          sl_assert_rhs,                                                            \
          __func__,                                                                 \
          __FILE__,                                                                 \
          __LINE__                                                                  \
      );                                                                            \
      return false;                                                                 \
    }                                                                               \
  } while (false)

#define SL_ASSERT_TRUE(expr, ...) SL_ASSERT_BINOP(==, (bool)(expr), true, __VA_ARGS__)
#define SL_ASSERT_FALSE(expr, ...) SL_ASSERT_BINOP(==, (bool)(expr), false, __VA_ARGS__)
#define SL_ASSERT_EQ_CHAR(lhs, rhs, ...) SL_ASSERT_BINOP(==, (char)(lhs), (char)(rhs), __VA_ARGS__)
#define SL_ASSERT_EQ_LL(lhs, rhs, ...) \
  SL_ASSERT_BINOP(==, (long long)(lhs), (long long)(rhs), __VA_ARGS__)
#define SL_ASSERT_EQ_PTR(lhs, rhs, ...) SL_ASSERT_BINOP(==, (void*)(lhs), (void*)(rhs), __VA_ARGS__)
#define SL_ASSERT_EQ_STR(lhs, rhs, ...) SL_ASSERT_STRCMP(0, lhs, rhs, __VA_ARGS__)
#define SL_ASSERT_EQ_DOUBLE(...) SL_ASSERT_DOUBLE_ALMOST(__VA_ARGS__)

#define SL_TEST_MAIN(...)                                                                       \
  int main(int argc, char* const argv[argc + 1]) {                                              \
    struct sl_args args                                      = {.argc = argc, .argv = argv};    \
    const bool verbose                                       = sl_args_parse_flag(&args, "-v"); \
    bool (*tests[])(struct sl_context[static 1], const bool) = {__VA_ARGS__};                   \
    char test_names_str[]                                    = #__VA_ARGS__;                    \
    bool ok                                                  = true;                            \
    const char* test_name                                    = "";                              \
    struct sl_context ctx                                    = {0};                             \
    for (size_t t = 0; ok && t < SL_ARRAY_LEN(tests); ++t) {                                    \
      sl_error_clear(&ctx.errors);                                                              \
      test_name = strtok(t ? 0 : test_names_str, ", ");                                         \
      assert(test_name);                                                                        \
      if (verbose) {                                                                            \
        printf("%s\n", test_name);                                                              \
      }                                                                                         \
      ok = tests[t](&ctx, verbose);                                                             \
      if (!ok) {                                                                                \
        SL_LOG_ERROR("test %s (%zu) failed", test_name, t);                                     \
      }                                                                                         \
    }                                                                                           \
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;                                                    \
  }

#endif  // SL_TESTING_H_INCLUDED
