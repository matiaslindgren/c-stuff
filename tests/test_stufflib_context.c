#include <stdio.h>
#include <string.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/macros/macros.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_fresh_ctx_has_no_errors) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ASSERT_TRUE(!sl_error_occurred(&c.errors));
  SL_ASSERT_TRUE(sl_error_depth(&c.errors) == 0);

  return true;
}

SL_TEST(test_sl_error_macro_no_format_args) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "something failed");

  SL_ASSERT_TRUE(sl_error_occurred(&c.errors));
  SL_ASSERT_TRUE(sl_error_depth(&c.errors) == 1);

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strcmp(top->msg, "something failed") == 0);

  return true;
}

SL_TEST(test_sl_error_macro_formats_message) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "value is %d and str is %s", 42, "hello");

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strcmp(top->msg, "value is 42 and str is hello") == 0);

  return true;
}

SL_TEST(test_sl_error_macro_captures_file) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "check file");

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strstr(top->file, "test_stufflib_context.c") != nullptr);

  return true;
}

SL_TEST(test_sl_error_macro_captures_line) {
  (void)ctx;
  struct sl_context c = {0};

  const int line_before = __LINE__;
  SL_ERROR(&c, "check line");

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(top->line == line_before + 1);

  return true;
}

SL_TEST(test_sl_error_macro_accumulates) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "first");
  SL_ERROR(&c, "second");
  SL_ERROR(&c, "third");

  SL_ASSERT_TRUE(sl_error_depth(&c.errors) == 3);

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strcmp(top->msg, "third") == 0);

  return true;
}

SL_TEST(test_sl_error_macro_msg_truncation) {
  (void)ctx;
  struct sl_context c = {0};

  // Format a message that would exceed SL_ERROR_MSG_LEN before truncation
  char long_str[SL_ERROR_MSG_LEN + 64];
  memset(long_str, 'z', sizeof(long_str) - 1);
  long_str[sizeof(long_str) - 1] = '\0';

  SL_ERROR(&c, "%s", long_str);

  SL_ASSERT_TRUE(strlen(long_str) > SL_ERROR_MSG_LEN - 1);

  const struct sl_error_msg* top = sl_error_peek(&c.errors);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strlen(top->msg) == SL_ERROR_MSG_LEN - 1);
  SL_ASSERT_TRUE(memcmp(top->msg, long_str, SL_ERROR_MSG_LEN - 1) == 0);

  return true;
}

SL_TEST(test_unwind_empties_stack) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "first");
  SL_ERROR(&c, "second");
  SL_ERROR(&c, "third");

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem);

  SL_ASSERT_TRUE(sl_context_unwind_errors(&c, mem));
  fclose(mem);

  SL_ASSERT_TRUE(!sl_error_occurred(&c.errors));
  SL_ASSERT_TRUE(sl_error_depth(&c.errors) == 0);

  // LIFO: third printed first, then second, then first
  SL_ASSERT_TRUE(strstr(buf, "third") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "second") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "first") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "third") < strstr(buf, "second"));
  SL_ASSERT_TRUE(strstr(buf, "second") < strstr(buf, "first"));

  free(buf);
  return true;
}

SL_TEST(test_unwind_output_format) {
  (void)ctx;
  struct sl_context c = {0};

  SL_ERROR(&c, "boom");

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem);

  SL_ASSERT_TRUE(sl_context_unwind_errors(&c, mem));
  fclose(mem);

  SL_ASSERT_TRUE(strstr(buf, "\"level\":\"error\"") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"file\":\"") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"line\":") != nullptr);
  SL_ASSERT_TRUE(strstr(buf, "\"msg\":\"boom\"}") != nullptr);

  free(buf);
  return true;
}

SL_TEST(test_unwind_empty_stack_is_noop) {
  (void)ctx;
  struct sl_context c = {0};

  char* buf   = nullptr;
  size_t size = 0;
  FILE* mem   = open_memstream(&buf, &size);
  SL_ASSERT_TRUE(mem);

  SL_ASSERT_TRUE(sl_context_unwind_errors(&c, mem));
  fclose(mem);

  SL_ASSERT_TRUE(size == 0);

  free(buf);
  return true;
}

SL_TEST_MAIN()
