#include <stddef.h>

#include <stufflib/json/json.h>
#include <stufflib/testing/testing.h>

SL_TEST(test_json_str) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"hello\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "hello");
  return true;
}

SL_TEST(test_json_str_escaped_quote) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\\"hello\\\"\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\"hello\"");
  return true;
}

SL_TEST(test_json_str_escaped_backslash) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\\\\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\\");
  return true;
}

SL_TEST(test_json_str_escaped_newline) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\n\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\n");
  return true;
}

SL_TEST(test_json_str_escaped_carriage_return) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\r\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\r");
  return true;
}

SL_TEST(test_json_str_escaped_tab) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\t\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\t");
  return true;
}

SL_TEST(test_json_str_escaped_backspace) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\b\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\b");
  return true;
}

SL_TEST(test_json_str_escaped_form_feed) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\f\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\f");
  return true;
}

SL_TEST(test_json_str_escaped_unicode) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(sl_json_str("{\"key\":\"\\u0001\"}", "key", sizeof(val), val));
  SL_ASSERT_EQ_LL((unsigned char)val[0], 0x01);
  SL_ASSERT_EQ_LL(val[1], '\0');
  return true;
}

SL_TEST(test_json_str_middle_key) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(
      sl_json_str("{\"a\":\"first\",\"b\":\"second\",\"c\":\"third\"}", "b", sizeof(val), val)
  );
  SL_ASSERT_EQ_STR(val, "second");
  return true;
}

SL_TEST(test_json_str_last_key) {
  (void)ctx;
  char val[64];
  SL_ASSERT_TRUE(
      sl_json_str("{\"a\":\"first\",\"b\":\"second\",\"c\":\"third\"}", "c", sizeof(val), val)
  );
  SL_ASSERT_EQ_STR(val, "third");
  return true;
}

SL_TEST(test_json_int_middle_key) {
  (void)ctx;
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_int("{\"a\":1,\"b\":2,\"c\":3}", "b", &val));
  SL_ASSERT_EQ_LL(val, 2);
  return true;
}

SL_TEST(test_json_int_last_key) {
  (void)ctx;
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_int("{\"a\":1,\"b\":2,\"c\":3}", "c", &val));
  SL_ASSERT_EQ_LL(val, 3);
  return true;
}

SL_TEST(test_json_str_not_found) {
  (void)ctx;
  char val[64];
  SL_ASSERT_FALSE(sl_json_str("{\"key\":\"hello\"}", "other", sizeof(val), val));
  return true;
}

SL_TEST(test_json_int) {
  (void)ctx;
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_int("{\"line\":42}", "line", &val));
  SL_ASSERT_EQ_LL(val, 42);
  return true;
}

SL_TEST(test_json_int_not_found) {
  (void)ctx;
  long long val = 0;
  SL_ASSERT_FALSE(sl_json_int("{\"line\":42}", "other", &val));
  return true;
}

SL_TEST_MAIN()
