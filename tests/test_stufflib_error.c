#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/error/error.h>
#include <stufflib/macros/macros.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_empty_stack) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  SL_ASSERT_TRUE(sl_error_depth(&s) == 0);
  SL_ASSERT_TRUE(!sl_error_occurred(&s));
  SL_ASSERT_TRUE(sl_error_peek(&s) == nullptr);

  struct sl_error_msg out = {0};
  SL_ASSERT_TRUE(!sl_error_pop(&s, &out));

  return true;
}

SL_TEST(test_push_single) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  sl_error_push(&s, "file.c", 42, "something went wrong");

  SL_ASSERT_TRUE(sl_error_depth(&s) == 1);
  SL_ASSERT_TRUE(sl_error_occurred(&s));

  const struct sl_error_msg* top = sl_error_peek(&s);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strcmp(top->file, "file.c") == 0);
  SL_ASSERT_TRUE(top->line == 42);
  SL_ASSERT_TRUE(strcmp(top->msg, "something went wrong") == 0);

  return true;
}

SL_TEST(test_pop_single) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  sl_error_push(&s, "file.c", 7, "pop me");

  struct sl_error_msg out = {0};
  SL_ASSERT_TRUE(sl_error_pop(&s, &out));
  SL_ASSERT_TRUE(strcmp(out.file, "file.c") == 0);
  SL_ASSERT_TRUE(out.line == 7);
  SL_ASSERT_TRUE(strcmp(out.msg, "pop me") == 0);

  SL_ASSERT_TRUE(sl_error_depth(&s) == 0);
  SL_ASSERT_TRUE(!sl_error_occurred(&s));
  SL_ASSERT_TRUE(sl_error_peek(&s) == nullptr);

  return true;
}

SL_TEST(test_lifo_order) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  sl_error_push(&s, "a.c", 1, "first");
  sl_error_push(&s, "b.c", 2, "second");
  sl_error_push(&s, "c.c", 3, "third");

  SL_ASSERT_TRUE(sl_error_depth(&s) == 3);

  const struct sl_error_msg* top = sl_error_peek(&s);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strcmp(top->msg, "third") == 0);

  struct sl_error_msg out = {0};

  SL_ASSERT_TRUE(sl_error_pop(&s, &out));
  SL_ASSERT_TRUE(strcmp(out.msg, "third") == 0);
  SL_ASSERT_TRUE(sl_error_depth(&s) == 2);

  SL_ASSERT_TRUE(sl_error_pop(&s, &out));
  SL_ASSERT_TRUE(strcmp(out.msg, "second") == 0);
  SL_ASSERT_TRUE(sl_error_depth(&s) == 1);

  SL_ASSERT_TRUE(sl_error_pop(&s, &out));
  SL_ASSERT_TRUE(strcmp(out.msg, "first") == 0);
  SL_ASSERT_TRUE(sl_error_depth(&s) == 0);

  SL_ASSERT_TRUE(!sl_error_pop(&s, &out));

  return true;
}

SL_TEST(test_peek_does_not_pop) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  sl_error_push(&s, "x.c", 99, "persistent");

  for (size_t i = 0; i < 5; ++i) {
    const struct sl_error_msg* top = sl_error_peek(&s);
    SL_ASSERT_TRUE(top != nullptr);
    SL_ASSERT_TRUE(strcmp(top->msg, "persistent") == 0);
    SL_ASSERT_TRUE(sl_error_depth(&s) == 1);
  }

  return true;
}

SL_TEST(test_clear) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  sl_error_push(&s, "a.c", 1, "one");
  sl_error_push(&s, "b.c", 2, "two");
  sl_error_push(&s, "c.c", 3, "three");

  sl_error_clear(&s);

  SL_ASSERT_TRUE(sl_error_depth(&s) == 0);
  SL_ASSERT_TRUE(!sl_error_occurred(&s));
  SL_ASSERT_TRUE(sl_error_peek(&s) == nullptr);

  struct sl_error_msg out = {0};
  SL_ASSERT_TRUE(!sl_error_pop(&s, &out));

  return true;
}

SL_TEST(test_depth_tracking) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  const size_t n = SL_ERROR_STACK_DEPTH - 1;
  for (size_t i = 0; i < n; ++i) {
    sl_error_push(&s, "f.c", (int)i, "msg");
    SL_ASSERT_TRUE(sl_error_depth(&s) == i + 1);
  }

  for (size_t i = n; i > 0; --i) {
    SL_ASSERT_TRUE(sl_error_depth(&s) == i);
    struct sl_error_msg out = {0};
    SL_ASSERT_TRUE(sl_error_pop(&s, &out));
  }

  SL_ASSERT_TRUE(sl_error_depth(&s) == 0);

  return true;
}

SL_TEST(test_msg_truncation) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  char long_msg[SL_ERROR_MSG_LEN + 64];
  memset(long_msg, 'x', sizeof(long_msg) - 1);
  long_msg[sizeof(long_msg) - 1] = '\0';

  sl_error_push(&s, "f.c", 1, long_msg);

  const struct sl_error_msg* top = sl_error_peek(&s);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strlen(top->msg) == SL_ERROR_MSG_LEN - 1);

  return true;
}

SL_TEST(test_clear_then_reuse) {
  (void)ctx;
  (void)verbose;
  struct sl_error_stack s = {0};

  sl_error_push(&s, "a.c", 1, "before clear");
  sl_error_clear(&s);

  sl_error_push(&s, "b.c", 2, "after clear");

  SL_ASSERT_TRUE(sl_error_depth(&s) == 1);
  const struct sl_error_msg* top = sl_error_peek(&s);
  SL_ASSERT_TRUE(top != nullptr);
  SL_ASSERT_TRUE(strcmp(top->msg, "after clear") == 0);
  SL_ASSERT_TRUE(top->line == 2);

  return true;
}

SL_TEST_MAIN()
