#include <stddef.h>
#include <string.h>

#include <stufflib/json/json.h>
#include <stufflib/testing/testing.h>

#define SL_JSON_VALID(s)   SL_ASSERT_TRUE(sl_json_parse(strlen(s), s))
#define SL_JSON_INVALID(s) SL_ASSERT_FALSE(sl_json_parse(strlen(s), s))

// sl_json_parse: valid inputs

SL_TEST(test_parse_null) {
  (void)ctx;
  SL_JSON_VALID("null");
  return true;
}

SL_TEST(test_parse_true) {
  (void)ctx;
  SL_JSON_VALID("true");
  return true;
}

SL_TEST(test_parse_false) {
  (void)ctx;
  SL_JSON_VALID("false");
  return true;
}

SL_TEST(test_parse_integer) {
  (void)ctx;
  SL_JSON_VALID("0");
  SL_JSON_VALID("1");
  SL_JSON_VALID("42");
  SL_JSON_VALID("-0");
  SL_JSON_VALID("-1");
  SL_JSON_VALID("-123");
  return true;
}

SL_TEST(test_parse_fraction) {
  (void)ctx;
  SL_JSON_VALID("0.0");
  SL_JSON_VALID("3.14");
  SL_JSON_VALID("-0.5");
  return true;
}

SL_TEST(test_parse_exponent) {
  (void)ctx;
  SL_JSON_VALID("1e10");
  SL_JSON_VALID("1E10");
  SL_JSON_VALID("1e+10");
  SL_JSON_VALID("1e-10");
  SL_JSON_VALID("2.5e3");
  SL_JSON_VALID("-3.14E+2");
  return true;
}

SL_TEST(test_parse_string_empty) {
  (void)ctx;
  SL_JSON_VALID("\"\"");
  return true;
}

SL_TEST(test_parse_string_simple) {
  (void)ctx;
  SL_JSON_VALID("\"hello\"");
  return true;
}

SL_TEST(test_parse_string_escapes) {
  (void)ctx;
  SL_JSON_VALID("\"\\\"\"");
  SL_JSON_VALID("\"\\\\\"");
  SL_JSON_VALID("\"\\/\"");
  SL_JSON_VALID("\"\\b\"");
  SL_JSON_VALID("\"\\f\"");
  SL_JSON_VALID("\"\\n\"");
  SL_JSON_VALID("\"\\r\"");
  SL_JSON_VALID("\"\\t\"");
  SL_JSON_VALID("\"\\u0041\"");
  SL_JSON_VALID("\"\\uFFFF\"");
  return true;
}

SL_TEST(test_parse_empty_object) {
  (void)ctx;
  SL_JSON_VALID("{}");
  SL_JSON_VALID("{ }");
  SL_JSON_VALID("{  \n  }");
  return true;
}

SL_TEST(test_parse_object_one_member) {
  (void)ctx;
  SL_JSON_VALID("{\"a\":1}");
  SL_JSON_VALID("{ \"a\" : 1 }");
  return true;
}

SL_TEST(test_parse_object_multiple_members) {
  (void)ctx;
  SL_JSON_VALID("{\"a\":1,\"b\":2,\"c\":3}");
  SL_JSON_VALID("{ \"a\" : 1 , \"b\" : 2 }");
  return true;
}

SL_TEST(test_parse_empty_array) {
  (void)ctx;
  SL_JSON_VALID("[]");
  SL_JSON_VALID("[ ]");
  SL_JSON_VALID("[  \t  ]");
  return true;
}

SL_TEST(test_parse_array_elements) {
  (void)ctx;
  SL_JSON_VALID("[1]");
  SL_JSON_VALID("[1,2,3]");
  SL_JSON_VALID("[ 1 , 2 , 3 ]");
  SL_JSON_VALID("[\"a\",\"b\"]");
  SL_JSON_VALID("[true,false,null]");
  return true;
}

SL_TEST(test_parse_nested) {
  (void)ctx;
  SL_JSON_VALID("{\"a\":{\"b\":1}}");
  SL_JSON_VALID("{\"a\":[1,2]}");
  SL_JSON_VALID("[[1],[2]]");
  SL_JSON_VALID("{\"a\":{\"b\":{\"c\":true}}}");
  SL_JSON_VALID("[{\"a\":1},{\"b\":2}]");
  return true;
}

SL_TEST(test_parse_whitespace_around_value) {
  (void)ctx;
  SL_JSON_VALID("  null  ");
  SL_JSON_VALID("\n\ttrue\r\n");
  SL_JSON_VALID("  { \"key\" : \"val\" }  ");
  return true;
}

// sl_json_parse: invalid inputs

SL_TEST(test_parse_reject_empty) {
  (void)ctx;
  SL_JSON_INVALID("");
  return true;
}

SL_TEST(test_parse_reject_bare_word) {
  (void)ctx;
  SL_JSON_INVALID("hello");
  SL_JSON_INVALID("nul");
  SL_JSON_INVALID("tru");
  SL_JSON_INVALID("fals");
  return true;
}

SL_TEST(test_parse_reject_trailing_content) {
  (void)ctx;
  SL_JSON_INVALID("true false");
  SL_JSON_INVALID("1 2");
  SL_JSON_INVALID("{}{}");
  return true;
}

SL_TEST(test_parse_reject_leading_zero) {
  (void)ctx;
  SL_JSON_INVALID("01");
  SL_JSON_INVALID("00");
  SL_JSON_INVALID("-01");
  return true;
}

SL_TEST(test_parse_reject_bad_number) {
  (void)ctx;
  SL_JSON_INVALID("+1");
  SL_JSON_INVALID(".5");
  SL_JSON_INVALID("1.");
  SL_JSON_INVALID("1e");
  SL_JSON_INVALID("1e+");
  SL_JSON_INVALID("-");
  return true;
}

SL_TEST(test_parse_reject_bad_string) {
  (void)ctx;
  SL_JSON_INVALID("\"");
  SL_JSON_INVALID("\"\\\"");
  SL_JSON_INVALID("\"\\x\"");
  SL_JSON_INVALID("\"\\u00G0\"");
  SL_JSON_INVALID("\"\\u00\"");
  return true;
}

SL_TEST(test_parse_reject_bad_object) {
  (void)ctx;
  SL_JSON_INVALID("{");
  SL_JSON_INVALID("{\"a\"}");
  SL_JSON_INVALID("{\"a\":}");
  SL_JSON_INVALID("{\"a\":1,}");
  SL_JSON_INVALID("{:1}");
  SL_JSON_INVALID("{1:2}");
  return true;
}

SL_TEST(test_parse_reject_bad_array) {
  (void)ctx;
  SL_JSON_INVALID("[");
  SL_JSON_INVALID("[1,]");
  SL_JSON_INVALID("[,1]");
  SL_JSON_INVALID("[1,2,]");
  return true;
}

SL_TEST(test_parse_reject_control_char_in_string) {
  (void)ctx;
  SL_JSON_INVALID("\"\x01\"");
  SL_JSON_INVALID("\"\n\"");
  SL_JSON_INVALID("\"\t\"");
  return true;
}

// existing tests: sl_json_str / sl_json_int

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
