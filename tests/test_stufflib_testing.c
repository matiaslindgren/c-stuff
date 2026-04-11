#include <stufflib/testing/testing.h>

SL_TEST(test_assert_true) {
  (void)ctx;
  (void)verbose;
  SL_ASSERT_TRUE(true);
  return true;
}

SL_TEST(test_assert_false) {
  (void)ctx;
  (void)verbose;
  SL_ASSERT_FALSE(false);
  return true;
}

SL_TEST_MAIN2()
