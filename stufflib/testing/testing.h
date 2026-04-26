#ifndef SL_TESTING_H_INCLUDED
#define SL_TESTING_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#include <stufflib/context/context.h>
#include <stufflib/macros/macros.h>
#include <stufflib/math/math.h>

#define SL_ANSI_RED         "\033[31m"
#define SL_ANSI_GREEN       "\033[32m"
#define SL_ANSI_RESET       "\033[0m"
#define SL_ANSI_COLOR(c, s) (sl_terminal_use_colors() ? (c s SL_ANSI_RESET) : (s))

bool sl_terminal_use_colors(void);

typedef bool sl_testing_function(struct sl_context* const);

struct sl_testing_test {
  struct sl_testing_test* next;
  const char* name;
  const char* file;
  sl_testing_function* func;
};

static struct sl_testing_test* sl_testing_tests = nullptr;

static inline void sl_testing_destroy_tests(void) {
  for (struct sl_testing_test* test = sl_testing_tests; test != nullptr;) {
    struct sl_testing_test* next_test = test->next;
    free(test);
    test = next_test;
  }
  sl_testing_tests = nullptr;
}

static inline void sl_testing_register_test_instance(
    const char name[const static 1],
    const char file[const static 1],
    sl_testing_function* test_fn
) {
  struct sl_testing_test* test = calloc(1, sizeof(struct sl_testing_test));
  if (!test) {
    fprintf(
        stderr,
        "[%s] FATAL: failed allocating sl_testing_test node for %s:%s\n",
        __func__,
        file,
        name
    );
    sl_testing_destroy_tests();
    abort();
  }
  *test = (struct sl_testing_test){
      .next = sl_testing_tests,
      .file = file,
      .name = name,
      .func = test_fn,
  };
  sl_testing_tests = test;
}

static inline int sl_testing_run(struct sl_context ctx[static const 1]) {
  int n_failures = 0;
  for (struct sl_testing_test* test = sl_testing_tests; test != nullptr; test = test->next) {
    fprintf(stderr, "%s:%s: ", test->file, test->name);
    bool passed = test->func(ctx);
    if (sl_context_error_occurred(ctx)) {
      fprintf(stderr, SL_ANSI_COLOR(SL_ANSI_RED, " ERROR\n"));
      sl_context_unwind_errors(ctx, stderr);
      ++n_failures;
    } else if (passed) {
      fprintf(stderr, SL_ANSI_COLOR(SL_ANSI_GREEN, " PASSED\n"));
    } else {
      fprintf(stderr, SL_ANSI_COLOR(SL_ANSI_RED, " FAILED\n"));
      ++n_failures;
    }
  }
  return n_failures;
}

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
      const void*: "%p",          \
      unsigned char*: "%p",       \
      const unsigned char*: "%p", \
      char*: "%p",                \
      const char*: "%p"           \
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

#define SL_ASSERT_STRNCMP(expect, lhs, rhs, len, ...)                               \
  do {                                                                              \
    const char* const sl_strncmp_lhs = (const char* const)(lhs);                    \
    const char* const sl_strncmp_rhs = (const char* const)(rhs);                    \
    const size_t sl_strncmp_len      = (size_t)(len);                               \
    if (!(strncmp(sl_strncmp_lhs, sl_strncmp_rhs, sl_strncmp_len) == expect)) {     \
      fprintf(                                                                      \
          stderr,                                                                   \
          "assertion failed: strncmp(" #lhs ", " #rhs ", " #len                     \
          ")"                                                                       \
          " == " #expect "\n  lhs: " #lhs " = \"%s\"\n  rhs: " #rhs                 \
          " = \"%s\""                                                               \
          "\n  function: %s\n  file: %s\n  line: %d\n  msg: \"" __VA_ARGS__ "\"\n", \
          sl_strncmp_lhs,                                                           \
          sl_strncmp_rhs,                                                           \
          __func__,                                                                 \
          __FILE__,                                                                 \
          __LINE__                                                                  \
      );                                                                            \
      return false;                                                                 \
    }                                                                               \
  } while (false)

#define SL_ASSERT_EQ_STR(lhs, rhs, ...)                      \
  do {                                                       \
    const char* const sl_eq_str_lhs = (lhs);                 \
    const char* const sl_eq_str_rhs = (rhs);                 \
    const size_t sl_eq_str_lhslen   = strlen(sl_eq_str_lhs); \
    const size_t sl_eq_str_rhslen   = strlen(sl_eq_str_rhs); \
    SL_ASSERT_STRNCMP(                                       \
        0,                                                   \
        sl_eq_str_lhs,                                       \
        sl_eq_str_rhs,                                       \
        SL_MAX(sl_eq_str_lhslen, sl_eq_str_rhslen),          \
        __VA_ARGS__                                          \
    );                                                       \
  } while (false)

#define SL_ASSERT_STR_STARTS_WITH(str, prefix, ...)                                          \
  do {                                                                                       \
    const char* const sl_assert_str    = (str);                                              \
    const char* const sl_assert_prefix = (prefix);                                           \
    const size_t sl_assert_prefixlen   = strlen(sl_assert_prefix);                           \
    SL_ASSERT_STRNCMP(0, sl_assert_str, sl_assert_prefix, sl_assert_prefixlen, __VA_ARGS__); \
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

#define SL_ASSERT_TRUE(expr, ...)        SL_ASSERT_BINOP(==, (bool)(expr), true, __VA_ARGS__)
#define SL_ASSERT_FALSE(expr, ...)       SL_ASSERT_BINOP(==, (bool)(expr), false, __VA_ARGS__)
#define SL_ASSERT_EQ_CHAR(lhs, rhs, ...) SL_ASSERT_BINOP(==, (char)(lhs), (char)(rhs), __VA_ARGS__)
#define SL_ASSERT_EQ_LL(lhs, rhs, ...) \
  SL_ASSERT_BINOP(==, (long long)(lhs), (long long)(rhs), __VA_ARGS__)
#define SL_ASSERT_EQ_PTR(lhs, rhs, ...) SL_ASSERT_BINOP(==, (lhs), (rhs), __VA_ARGS__)
#define SL_ASSERT_EQ_DOUBLE(...)        SL_ASSERT_DOUBLE_ALMOST(__VA_ARGS__)

#define SL_ASSERT_ERROR_OCCURRED(ctx, prefix, ...)                                   \
  do {                                                                               \
    struct sl_context* sl_assert_err_ctx = (ctx);                                    \
    struct sl_error_msg sl_assert_err_msg;                                           \
    SL_ASSERT_EQ_LL(sl_error_depth(&(sl_assert_err_ctx->errors)), 1 __VA_OPT__(, ) __VA_ARGS__); \
    sl_error_pop(&(sl_assert_err_ctx->errors), &sl_assert_err_msg);                              \
    SL_ASSERT_STR_STARTS_WITH(sl_assert_err_msg.msg, (prefix) __VA_OPT__(, ) __VA_ARGS__);       \
  } while (false)

#define SL_TEST(name)                                                         \
  static bool name(struct sl_context ctx[const static 1]);                    \
  __attribute__((constructor)) static void sl_testing_register_##name(void) { \
    sl_testing_register_test_instance(#name, __FILE__, name);                 \
  }                                                                           \
  static bool name(struct sl_context ctx[const static 1])

#define SL_TEST_MAIN()                             \
  int main(int argc, char* const argv[argc + 1]) { \
    (void)argc;                                    \
    (void)argv;                                    \
    struct sl_context ctx = {0};                   \
    bool pass             = true;                  \
    int n_failures        = sl_testing_run(&ctx);  \
    if (n_failures > 0) {                          \
      pass = false;                                \
    }                                              \
    if (sl_context_error_occurred(&ctx)) {         \
      pass = false;                                \
    }                                              \
    if (!sl_context_unwind_errors(&ctx, stderr)) { \
      pass = false;                                \
    }                                              \
    return pass ? EXIT_SUCCESS : EXIT_FAILURE;     \
  }

#endif  // SL_TESTING_H_INCLUDED
