#include <stdlib.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/error/error.h>
#include <stufflib/macros/macros.h>
#include <stufflib/memory/memory.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_alloc_empty) {
  for (size_t num = 0; num < 2; ++num) {
    for (size_t size = 0; size < 2; ++size) {
      if (num == 0 || size == 0) {
        void* res = sl_alloc(ctx, num, size);
        SL_ASSERT_TRUE(res == nullptr);
        SL_ASSERT_TRUE(sl_context_error_occurred(ctx));
        struct sl_error_msg msg;
        SL_ASSERT_TRUE(sl_error_pop(&ctx->errors, &msg));
        SL_ASSERT_FALSE(sl_context_error_occurred(ctx));
      }
    }
  }
  return true;
}

SL_TEST_MAIN()
