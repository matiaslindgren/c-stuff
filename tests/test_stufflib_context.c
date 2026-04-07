#include <assert.h>
#include <stdio.h>
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

  assert(strlen(long_str) > SL_ERROR_MSG_LEN - 1);

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  assert(top != nullptr);
  assert(strlen(top->msg) == SL_ERROR_MSG_LEN - 1);
  assert(memcmp(top->msg, long_str, SL_ERROR_MSG_LEN - 1) == 0);

  return true;
}

static bool test_unwind_empties_stack(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "first");
  SL_ERROR(&c, "second");
  SL_ERROR(&c, "third");

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  assert(mem);

  assert(sl_context_unwind_errors(&c, mem));
  fclose(mem);

  assert(!sl_error_occurred(&c.errors));
  assert(sl_error_depth(&c.errors) == 0);

  // LIFO: third printed first, then second, then first
  assert(strstr(buf, "third") != nullptr);
  assert(strstr(buf, "second") != nullptr);
  assert(strstr(buf, "first") != nullptr);
  assert(strstr(buf, "third") < strstr(buf, "second"));
  assert(strstr(buf, "second") < strstr(buf, "first"));

  free(buf);
  return true;
}

static bool test_unwind_output_format(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "boom");

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  assert(mem);

  assert(sl_context_unwind_errors(&c, mem));
  fclose(mem);

  assert(strstr(buf, "\"level\":\"error\"") != nullptr);
  assert(strstr(buf, "\"file\":\"") != nullptr);
  assert(strstr(buf, "\"line\":") != nullptr);
  assert(strstr(buf, "\"msg\":\"boom\"}") != nullptr);

  free(buf);
  return true;
}

static bool test_unwind_empty_stack_is_noop(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  struct sl_context c = {0};

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  assert(mem);

  assert(sl_context_unwind_errors(&c, mem));
  fclose(mem);

  assert(size == 0);

  free(buf);
  return true;
}

SL_TEST_MAIN(
    test_fresh_ctx_has_no_errors,
    test_sl_error_macro_no_format_args,
    test_sl_error_macro_formats_message,
    test_sl_error_macro_captures_file,
    test_sl_error_macro_captures_line,
    test_sl_error_macro_accumulates,
    test_sl_error_macro_msg_truncation,
    test_unwind_empties_stack,
    test_unwind_output_format,
    test_unwind_empty_stack_is_noop
)
