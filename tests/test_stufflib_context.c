#include <assert.h>
#include <string.h>
#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/macros/macros.h>

static bool test_fresh_ctx_has_no_errors(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  assert(!sl_error_occurred(&c.errors));
  assert(sl_error_depth(&c.errors) == 0);

  return true;
}

static bool test_sl_error_macro_no_format_args(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "something failed");

  assert(sl_error_occurred(&c.errors));
  assert(sl_error_depth(&c.errors) == 1);

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  assert(top != nullptr);
  assert(strcmp(top->msg, "something failed") == 0);

  return true;
}

static bool test_sl_error_macro_formats_message(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "value is %d and str is %s", 42, "hello");

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  assert(top != nullptr);
  assert(strcmp(top->msg, "value is 42 and str is hello") == 0);

  return true;
}

static bool test_sl_error_macro_captures_file(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "check file");

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  assert(top != nullptr);
  assert(strstr(top->file, "test_stufflib_context.c") != nullptr);

  return true;
}

static bool test_sl_error_macro_captures_line(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  const int line_before = __LINE__;
  SL_ERROR(&c, "check line");

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  assert(top != nullptr);
  assert(top->line == line_before + 1);

  return true;
}

static bool test_sl_error_macro_accumulates(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "first");
  SL_ERROR(&c, "second");
  SL_ERROR(&c, "third");

  assert(sl_error_depth(&c.errors) == 3);

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  assert(top != nullptr);
  assert(strcmp(top->msg, "third") == 0);

  return true;
}

static bool test_sl_error_macro_msg_truncation(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  // Format a message that would exceed SL_ERROR_MSG_LEN before truncation
  char long_str[SL_ERROR_MSG_LEN + 64];
  memset(long_str, 'z', sizeof(long_str) - 1);
  long_str[sizeof(long_str) - 1] = '\0';

  SL_ERROR(&c, "%s", long_str);

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  assert(top != nullptr);
  assert(strlen(top->msg) == SL_ERROR_MSG_LEN - 1);

  return true;
}

SL_TEST_MAIN(
    test_fresh_ctx_has_no_errors,
    test_sl_error_macro_no_format_args,
    test_sl_error_macro_formats_message,
    test_sl_error_macro_captures_file,
    test_sl_error_macro_captures_line,
    test_sl_error_macro_accumulates,
    test_sl_error_macro_msg_truncation
)
