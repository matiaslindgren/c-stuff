#include <string.h>

#include <stufflib/testing/testing.h>

static struct sl_testing_test* sl_testing_find(const char name[const static 1]) {
  for (struct sl_testing_test* t = sl_testing_tests; t != nullptr; t = t->next) {
    if (strcmp(t->name, name) == 0) {
      return t;
    }
  }
  return nullptr;
}

SL_TEST(test_assert_true) {
  (void)ctx;
  SL_ASSERT_TRUE(true);
  return true;
}

SL_TEST(test_assert_false) {
  (void)ctx;
  SL_ASSERT_FALSE(false);
  return true;
}

SL_TEST(test_tests_are_registered) {
  (void)ctx;
  SL_ASSERT_TRUE(sl_testing_tests != nullptr);
  return true;
}

SL_TEST(test_all_registered_tests_have_name) {
  (void)ctx;
  for (struct sl_testing_test* t = sl_testing_tests; t != nullptr; t = t->next) {
    SL_ASSERT_TRUE(t->name != nullptr);
    SL_ASSERT_TRUE(t->name[0] != '\0');
  }
  return true;
}

SL_TEST(test_all_registered_tests_have_file) {
  (void)ctx;
  for (struct sl_testing_test* t = sl_testing_tests; t != nullptr; t = t->next) {
    SL_ASSERT_TRUE(t->file != nullptr);
    SL_ASSERT_TRUE(t->file[0] != '\0');
  }
  return true;
}

SL_TEST(test_all_registered_tests_have_func) {
  (void)ctx;
  for (struct sl_testing_test* t = sl_testing_tests; t != nullptr; t = t->next) {
    SL_ASSERT_TRUE(t->func != nullptr);
  }
  return true;
}

SL_TEST(test_known_tests_are_registered) {
  (void)ctx;
  SL_ASSERT_TRUE(sl_testing_find("test_assert_true") != nullptr);
  SL_ASSERT_TRUE(sl_testing_find("test_assert_false") != nullptr);
  SL_ASSERT_TRUE(sl_testing_find("test_tests_are_registered") != nullptr);
  SL_ASSERT_TRUE(sl_testing_find("test_known_tests_are_registered") != nullptr);
  return true;
}

SL_TEST(test_unknown_test_not_registered) {
  (void)ctx;
  SL_ASSERT_TRUE(sl_testing_find("this_test_does_not_exist") == nullptr);
  SL_ASSERT_TRUE(sl_testing_find("") == nullptr);
  return true;
}

SL_TEST(test_registered_test_name_matches_func) {
  (void)ctx;
  {
    struct sl_testing_test* t = sl_testing_find("test_assert_true");
    SL_ASSERT_TRUE(t != nullptr);
    SL_ASSERT_EQ_STR(t->name, "test_assert_true");
    SL_ASSERT_TRUE(t->func == test_assert_true);
  }
  {
    struct sl_testing_test* t = sl_testing_find("test_unknown_test_not_registered");
    SL_ASSERT_TRUE(t != nullptr);
    SL_ASSERT_EQ_STR(t->name, "test_unknown_test_not_registered");
    SL_ASSERT_TRUE(t->func == test_unknown_test_not_registered);
  }
  {
    struct sl_testing_test* t = sl_testing_find("test_registered_test_name_matches_func");
    SL_ASSERT_TRUE(t != nullptr);
    SL_ASSERT_EQ_STR(t->name, "test_registered_test_name_matches_func");
    SL_ASSERT_TRUE(t->func == test_registered_test_name_matches_func);
  }
  return true;
}

SL_TEST_MAIN()
