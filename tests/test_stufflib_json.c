#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stufflib/json/json.h>
#include <stufflib/testing/testing.h>

#define SL_ASSERT_JSON_VALID(s)   SL_ASSERT_TRUE(sl_json_is_valid(ctx, strlen(s), s))
#define SL_ASSERT_JSON_INVALID(s) SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen(s), s))

SL_TEST(test_parse_null) {
  SL_ASSERT_JSON_VALID("null");
  return true;
}

SL_TEST(test_parse_true) {
  SL_ASSERT_JSON_VALID("true");
  return true;
}

SL_TEST(test_parse_false) {
  SL_ASSERT_JSON_VALID("false");
  return true;
}

SL_TEST(test_parse_integer) {
  SL_ASSERT_JSON_VALID("0");
  SL_ASSERT_JSON_VALID("1");
  SL_ASSERT_JSON_VALID("42");
  SL_ASSERT_JSON_VALID("-0");
  SL_ASSERT_JSON_VALID("-1");
  SL_ASSERT_JSON_VALID("-123");
  return true;
}

SL_TEST(test_parse_large_integer) {
  SL_ASSERT_JSON_VALID("2147483647");
  SL_ASSERT_JSON_VALID("-2147483648");
  SL_ASSERT_JSON_VALID("9223372036854775807");
  SL_ASSERT_JSON_VALID("-9223372036854775808");
  return true;
}

SL_TEST(test_parse_fraction) {
  SL_ASSERT_JSON_VALID("0.0");
  SL_ASSERT_JSON_VALID("3.14");
  SL_ASSERT_JSON_VALID("-0.5");
  return true;
}

SL_TEST(test_parse_exponent) {
  SL_ASSERT_JSON_VALID("1e10");
  SL_ASSERT_JSON_VALID("1E10");
  SL_ASSERT_JSON_VALID("1e+10");
  SL_ASSERT_JSON_VALID("1e-10");
  SL_ASSERT_JSON_VALID("2.5e3");
  SL_ASSERT_JSON_VALID("-3.14E+2");
  return true;
}

SL_TEST(test_parse_string_empty) {
  SL_ASSERT_JSON_VALID("\"\"");
  return true;
}

SL_TEST(test_parse_string_simple) {
  SL_ASSERT_JSON_VALID("\"hello\"");
  SL_ASSERT_JSON_VALID("\"123\"");
  SL_ASSERT_JSON_VALID("\" \"");
  SL_ASSERT_JSON_VALID("\"ö\"");
  return true;
}

SL_TEST(test_parse_string_long) {
  SL_ASSERT_JSON_VALID(
      "\"Vesi (kemiallinen kaava H2O, kemiallisena yhdisteenä voidaan käyttää myös systemaattisia "
      "nimiä; oksidaani, divetymonoksidi tai divetyoksidi) on huoneenlämmössä nesteenä esiintyvä "
      "vedyn ja hapen muodostama epäorgaaninen kemiallinen yhdiste ja vedyn palamistuote."
      "水（みず、（英: water、他言語呼称は「他言語での呼称」の項を参照）とは、化学式 H2O "
      "で表される、水素と酸素の化合物である。日本語においては特に湯と対比して用いられ、液体ではある"
      "が温度が低く、かつ凝固して氷にはなっていない物を言う。また、液状の物全般を指す。\""
  );
  return true;
}

SL_TEST(test_parse_string_escapes) {
  SL_ASSERT_JSON_VALID("\"\\\"\"");
  SL_ASSERT_JSON_VALID("\"\\\\\"");
  SL_ASSERT_JSON_VALID("\"\\/\"");
  SL_ASSERT_JSON_VALID("\"\\b\"");
  SL_ASSERT_JSON_VALID("\"\\f\"");
  SL_ASSERT_JSON_VALID("\"\\n\"");
  SL_ASSERT_JSON_VALID("\"\\r\"");
  SL_ASSERT_JSON_VALID("\"\\t\"");
  SL_ASSERT_JSON_VALID("\"\\u0041\"");
  SL_ASSERT_JSON_VALID("\"\\uFFFF\"");
  return true;
}

SL_TEST(test_parse_empty_object) {
  SL_ASSERT_JSON_VALID("{}");
  SL_ASSERT_JSON_VALID("{ }");
  SL_ASSERT_JSON_VALID("{  \n  }");
  return true;
}

SL_TEST(test_parse_object_one_member) {
  SL_ASSERT_JSON_VALID("{\"a\":1}");
  SL_ASSERT_JSON_VALID("{ \"a\" : 1 }");
  return true;
}

SL_TEST(test_parse_object_multiple_members) {
  SL_ASSERT_JSON_VALID("{\"a\":1,\"b\":2,\"c\":3}");
  SL_ASSERT_JSON_VALID("{ \"a\" : 1 , \"b\" : 2 }");
  return true;
}

SL_TEST(test_parse_empty_array) {
  SL_ASSERT_JSON_VALID("[]");
  SL_ASSERT_JSON_VALID("[ ]");
  SL_ASSERT_JSON_VALID("[  \t  ]");
  return true;
}

SL_TEST(test_parse_array_elements) {
  SL_ASSERT_JSON_VALID("[1]");
  SL_ASSERT_JSON_VALID("[1,2,3]");
  SL_ASSERT_JSON_VALID("[ 1 , 2 , 3 ]");
  SL_ASSERT_JSON_VALID("[\"a\",\"b\"]");
  SL_ASSERT_JSON_VALID("[true,false,null]");
  return true;
}

SL_TEST(test_parse_nested) {
  SL_ASSERT_JSON_VALID("{\"a\":{\"b\":1}}");
  SL_ASSERT_JSON_VALID("{\"a\":[1,2]}");
  SL_ASSERT_JSON_VALID("[[1],[2]]");
  SL_ASSERT_JSON_VALID("{\"a\":{\"b\":{\"c\":true}}}");
  SL_ASSERT_JSON_VALID("[{\"a\":1},{\"b\":2}]");
  return true;
}

SL_TEST(test_parse_whitespace_around_value) {
  SL_ASSERT_JSON_VALID("  null  ");
  SL_ASSERT_JSON_VALID("\n\ttrue\r\n");
  SL_ASSERT_JSON_VALID("  { \"key\" : \"val\" }  ");
  return true;
}

SL_TEST(test_parse_reject_empty) {
  SL_ASSERT_JSON_INVALID("");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  return true;
}

SL_TEST(test_parse_reject_bare_word) {
  SL_ASSERT_JSON_INVALID("hello");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("nul");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid literal");
  SL_ASSERT_JSON_INVALID("tru");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid literal");
  SL_ASSERT_JSON_INVALID("fals");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid literal");
  return true;
}

SL_TEST(test_parse_reject_trailing_content) {
  SL_ASSERT_JSON_INVALID("true false");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("1 2");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("{}{}");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  return true;
}

SL_TEST(test_parse_reject_leading_zero) {
  SL_ASSERT_JSON_INVALID("01");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("00");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("-01");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  return true;
}

SL_TEST(test_parse_reject_bad_number) {
  SL_ASSERT_JSON_INVALID("+1");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID(".5");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("1.");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_JSON_INVALID("1e");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_JSON_INVALID("1e+");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_JSON_INVALID("-");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  return true;
}

SL_TEST(test_parse_reject_bad_string) {
  SL_ASSERT_JSON_INVALID("\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_JSON_INVALID("\"\\\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_JSON_INVALID("\"\\x\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid escape character");
  SL_ASSERT_JSON_INVALID("\"\\u00G0\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid unicode escape");
  SL_ASSERT_JSON_INVALID("\"\\u00\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid unicode escape");
  return true;
}

SL_TEST(test_parse_reject_bad_object) {
  SL_ASSERT_JSON_INVALID("{");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_JSON_INVALID("{\"a\"}");
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected : after key");
  SL_ASSERT_JSON_INVALID("{\"a\":}");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("{\"a\":1,}");
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected \" for object key");
  SL_ASSERT_JSON_INVALID("{:1}");
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected } or \"");
  SL_ASSERT_JSON_INVALID("{1:2}");
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected } or \"");
  return true;
}

SL_TEST(test_parse_reject_bad_array) {
  SL_ASSERT_JSON_INVALID("[");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_JSON_INVALID("[1,]");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("[,1]");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_JSON_INVALID("[1,2,]");
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  return true;
}

SL_TEST(test_parse_reject_control_char_in_string) {
  SL_ASSERT_JSON_INVALID("\"\x01\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid control character");
  SL_ASSERT_JSON_INVALID("\"\n\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid control character");
  SL_ASSERT_JSON_INVALID("\"\t\"");
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid control character");
  return true;
}

SL_TEST(test_parse_error_unexpected_char) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("abc"), "abc"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("+1"), "+1"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");

  return true;
}

SL_TEST(test_parse_error_invalid_literal) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("tru"), "tru"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid literal");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("nul"), "nul"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid literal");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("fals"), "fals"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid literal");

  return true;
}

SL_TEST(test_parse_error_object) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{1:2}"), "{1:2}"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected } or \"");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{\"a\":1,2}"), "{\"a\":1,2}"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected \" for object key");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{\"a\"}"), "{\"a\"}"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected : after key");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{\"a\":1 2}"), "{\"a\":1 2}"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected , or } in object");

  return true;
}

SL_TEST(test_parse_error_array) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("[1 2]"), "[1 2]"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected , or ] in array");

  return true;
}

SL_TEST(test_parse_error_string) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("\"\x01\""), "\"\x01\""));
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid control character");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{\"\x01\":1}"), "{\"\x01\":1}"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid control character");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("\"\\x\""), "\"\\x\""));
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid escape character");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("\"\\u00GG\""), "\"\\u00GG\""));
  SL_ASSERT_ERROR_OCCURRED(ctx, "invalid unicode escape");

  return true;
}

SL_TEST(test_parse_error_number) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("-x"), "-x"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected digit after -");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("1.x"), "1.x"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected digit after decimal point");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("1ex"), "1ex"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected digit or sign in exponent");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("1e+x"), "1e+x"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected digit in exponent");

  return true;
}

SL_TEST(test_parse_error_after_value) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("1 2"), "1 2"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");

  return true;
}

SL_TEST(test_parse_error_end_of_input) {
  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{\"a\":"), "{\"a\":"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{"), "{"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("["), "["));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("\"abc"), "\"abc"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("-"), "-"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("1."), "1."));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("{\"a\":1,"), "{\"a\":1,"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  SL_ASSERT_FALSE(sl_json_is_valid(ctx, strlen("[1,"), "[1,"));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  return true;
}

SL_TEST(test_json_count_invalid) {
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen(""), ""), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("hello"), "hello"), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected character");
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("{"), "{"), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");
  return true;
}

SL_TEST(test_json_count_scalars) {
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("null"), "null"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("true"), "true"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("false"), "false"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("42"), "42"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("3.14"), "3.14"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("\"hello\""), "\"hello\""), 1);
  return true;
}

SL_TEST(test_json_count_containers) {
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("{}"), "{}"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("[]"), "[]"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("{\"a\":1}"), "{\"a\":1}"), 2);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(ctx, strlen("[1,2,3]"), "[1,2,3]"), 4);
  SL_ASSERT_EQ_LL(
      sl_json_count_nodes(ctx, strlen("{\"a\":{\"x\":1},\"d\":2}"), "{\"a\":{\"x\":1},\"d\":2}"),
      4
  );
  return true;
}

SL_TEST(test_find_invalid_path) {
  const char* json         = "{\"a\":1}";
  struct sl_json_node node = {0};

  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(""), "", &node));
  SL_ASSERT_ERROR_OCCURRED(ctx, "empty JSON path");

  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".a..b"), ".a..b", &node));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected key character after .");

  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".a[x]"), ".a[x]", &node));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected array index digit after '['");

  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".a[]"), ".a[]", &node));
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected array index digit after '['");

  return true;
}

SL_TEST(test_find_invalid_json) {
  const char* json         = "{\"a\":";
  struct sl_json_node node = {0};

  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".a"), ".a", &node));
  SL_ASSERT_ERROR_OCCURRED(ctx, "unexpected end of input");

  return true;
}

SL_TEST(test_find_top_level_int) {
  const char* json         = "{\"a\":42}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".a"), ".a", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 42);
  return true;
}

SL_TEST(test_find_top_level_negative_int) {
  const char* json         = "{\"n\":-42}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".n"), ".n", &node));
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, -42);
  return true;
}

SL_TEST(test_find_top_level_zero) {
  const char* json         = "{\"n\":0}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".n"), ".n", &node));
  long long val = -1;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 0);
  return true;
}

SL_TEST(test_find_top_level_str) {
  const char* json         = "{\"key\":\"hello\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_string);
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "hello");
  return true;
}

SL_TEST(test_find_non_first_key) {
  const char* json         = "{\"a\":1,\"b\":2,\"c\":3}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".b"), ".b", &node));
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 2);
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".c"), ".c", &node));
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 3);
  return true;
}

SL_TEST(test_find_not_found_key) {
  const char* json         = "{\"a\":1}";
  struct sl_json_node node = {0};
  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".z"), ".z", &node));
  return true;
}

SL_TEST(test_find_empty_object) {
  const char* json         = "{}";
  struct sl_json_node node = {0};
  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".a"), ".a", &node));
  return true;
}

SL_TEST(test_find_nested_key) {
  const char* json         = "{\"a\":{\"b\":7}}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".a.b"), ".a.b", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 7);
  return true;
}

SL_TEST(test_find_array_element) {
  const char* json         = "{\"a\":[10,20,30]}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".a[1]"), ".a[1]", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 20);
  return true;
}

SL_TEST(test_find_deep_path) {
  const char* json         = "{\"b\":\"a\",\"a\":{\"c\":0,\"b\":[10,20,30]}}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".a.b[1]"), ".a.b[1]", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 20);
  return true;
}

SL_TEST(test_find_top_level_array_index) {
  const char* json         = "[10,20,30]";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen("[2]"), "[2]", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 30);
  return true;
}

SL_TEST(test_find_index_out_of_range) {
  const char* json         = "{\"a\":[1,2]}";
  struct sl_json_node node = {0};
  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".a[5]"), ".a[5]", &node));
  return true;
}

SL_TEST(test_find_scalar_mid_path) {
  const char* json         = "{\"a\":42}";
  struct sl_json_node node = {0};
  SL_ASSERT_FALSE(sl_json_find(ctx, strlen(json), json, strlen(".a.b"), ".a.b", &node));
  return true;
}

SL_TEST(test_find_skip_nested_object) {
  const char* json         = "{\"a\":{\"x\":1},\"b\":2}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".b"), ".b", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 2);
  return true;
}

SL_TEST(test_find_skip_nested_array) {
  const char* json         = "{\"a\":[1,2,3],\"b\":4}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".b"), ".b", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 4);
  return true;
}

SL_TEST(test_find_array_of_objects) {
  const char* json         = "[{\"a\":1},{\"b\":2}]";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen("[1].b"), "[1].b", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&node, json, &val));
  SL_ASSERT_EQ_LL(val, 2);
  return true;
}

SL_TEST(test_find_wrong_type_check) {
  const char* json         = "{\"a\":1}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".a"), ".a", &node));
  SL_ASSERT_EQ_LL(node.type, sl_json_type_number);
  return true;
}

SL_TEST(test_find_str_escaped_quote) {
  const char* json         = "{\"key\":\"\\\"hello\\\"\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\"hello\"");
  return true;
}

SL_TEST(test_find_str_escaped_backslash) {
  const char* json         = "{\"key\":\"\\\\\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\\");
  return true;
}

SL_TEST(test_find_str_escaped_newline) {
  const char* json         = "{\"key\":\"\\n\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\n");
  return true;
}

SL_TEST(test_find_str_escapes) {
  const char* json         = "{\"key\":\"\\r\\t\\b\\f\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\r\t\b\f");
  return true;
}

SL_TEST(test_find_str_escaped_unicode) {
  const char* json         = "{\"key\":\"\\u0001\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_LL((unsigned char)val[0], 0x01);
  SL_ASSERT_EQ_LL(val[1], '\0');
  return true;
}

SL_TEST(test_find_str_buffer_too_small) {
  const char* json         = "{\"key\":\"hello\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[3];
  SL_ASSERT_FALSE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_ERROR_OCCURRED(ctx, "output buffer too small");
  return true;
}

SL_TEST(test_find_str_exact_fit) {
  const char* json         = "{\"key\":\"hi\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[3];  // 'h','i','\0'
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "hi");
  return true;
}

SL_TEST(test_find_str_empty) {
  const char* json         = "{\"key\":\"\"}";
  struct sl_json_node node = {0};
  SL_ASSERT_TRUE(sl_json_find(ctx, strlen(json), json, strlen(".key"), ".key", &node));
  char val[8];
  SL_ASSERT_TRUE(sl_json_get_str(ctx, &node, json, sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "");
  return true;
}

// helpers for path step assertions
#define SL_ASSERT_JSON_PATH_KEY(step, expected)                   \
  do {                                                            \
    SL_ASSERT_FALSE((step).is_index);                             \
    SL_ASSERT_EQ_LL((step).key_len, strlen(expected));            \
    SL_ASSERT_STRNCMP(0, (step).key, (expected), (step).key_len); \
  } while (false)

#define SL_ASSERT_JSON_PATH_IDX(step, expected) \
  do {                                          \
    SL_ASSERT_TRUE((step).is_index);            \
    SL_ASSERT_EQ_LL((step).index, expected);    \
  } while (false)

SL_TEST(test_parse_path_single_index) {
  const char* path = "[0]";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 1);
  SL_ASSERT_JSON_PATH_IDX(steps[0], 0);
  return true;
}

SL_TEST(test_parse_path_multi_digit_index) {
  const char* path = "[123]";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 1);
  SL_ASSERT_JSON_PATH_IDX(steps[0], 123);
  return true;
}

SL_TEST(test_parse_path_multiple_indices) {
  const char* path = "[0][1][2]";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 3);
  SL_ASSERT_JSON_PATH_IDX(steps[0], 0);
  SL_ASSERT_JSON_PATH_IDX(steps[1], 1);
  SL_ASSERT_JSON_PATH_IDX(steps[2], 2);
  return true;
}

SL_TEST(test_parse_path_multiple_large_indices) {
  const char* path = "[0][100][2000][30000][400000]";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 5);
  SL_ASSERT_JSON_PATH_IDX(steps[0], 0);
  SL_ASSERT_JSON_PATH_IDX(steps[1], 100);
  SL_ASSERT_JSON_PATH_IDX(steps[2], 2000);
  SL_ASSERT_JSON_PATH_IDX(steps[3], 30000);
  SL_ASSERT_JSON_PATH_IDX(steps[4], 400000);
  return true;
}

SL_TEST(test_parse_path_single_key) {
  const char* path = ".foo";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 1);
  SL_ASSERT_JSON_PATH_KEY(steps[0], "foo");
  return true;
}

SL_TEST(test_parse_path_nested_keys) {
  const char* path = ".a.b.c";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 3);
  SL_ASSERT_JSON_PATH_KEY(steps[0], "a");
  SL_ASSERT_JSON_PATH_KEY(steps[1], "b");
  SL_ASSERT_JSON_PATH_KEY(steps[2], "c");
  return true;
}

SL_TEST(test_parse_path_key_then_index) {
  const char* path = ".items[2]";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 2);
  SL_ASSERT_JSON_PATH_KEY(steps[0], "items");
  SL_ASSERT_JSON_PATH_IDX(steps[1], 2);
  return true;
}

SL_TEST(test_parse_path_index_then_key) {
  const char* path = "[0].name";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 2);
  SL_ASSERT_JSON_PATH_IDX(steps[0], 0);
  SL_ASSERT_JSON_PATH_KEY(steps[1], "name");
  return true;
}

SL_TEST(test_parse_path_complex) {
  const char* path = ".a.b[2].c";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 4);
  SL_ASSERT_JSON_PATH_KEY(steps[0], "a");
  SL_ASSERT_JSON_PATH_KEY(steps[1], "b");
  SL_ASSERT_JSON_PATH_IDX(steps[2], 2);
  SL_ASSERT_JSON_PATH_KEY(steps[3], "c");
  return true;
}

SL_TEST(test_parse_path_key_points_into_path) {
  // step.key must point into the original path string, not a copy
  const char* path = ".hello[0].world";
  struct sl_json_path_step steps[64];
  size_t n = sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps);
  SL_ASSERT_EQ_LL(n, 3);
  SL_ASSERT_EQ_PTR(steps[0].key, path + 1);   // ".hello" -> 'h' at path[1]
  SL_ASSERT_EQ_PTR(steps[2].key, path + 10);  // ".world" -> 'w' at path[10]
  return true;
}

SL_TEST(test_parse_path_error_bare_key) {
  const char* path = "foo";
  struct sl_json_path_step steps[64];
  SL_ASSERT_EQ_LL(sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected . or [");
  return true;
}

SL_TEST(test_parse_path_error_dot_only) {
  const char* path = ".";
  struct sl_json_path_step steps[64];
  SL_ASSERT_EQ_LL(sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected key character after .");
  return true;
}

SL_TEST(test_parse_path_error_double_dot) {
  const char* path = ".a..b";
  struct sl_json_path_step steps[64];
  SL_ASSERT_EQ_LL(sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected key character after .");
  return true;
}

SL_TEST(test_parse_path_error_non_digit_in_brackets) {
  const char* path = "[x]";
  struct sl_json_path_step steps[64];
  SL_ASSERT_EQ_LL(sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected array index digit after '['");
  return true;
}

SL_TEST(test_parse_path_error_empty_brackets) {
  const char* path = "[]";
  struct sl_json_path_step steps[64];
  SL_ASSERT_EQ_LL(sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected array index digit after '['");
  return true;
}

SL_TEST(test_parse_path_error_leading_zero_in_index) {
  const char* path = "[01]";
  struct sl_json_path_step steps[64];
  SL_ASSERT_EQ_LL(sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected ] but got");
  return true;
}

SL_TEST(test_parse_path_error_unclosed_bracket) {
  const char* path = ".a[1";
  struct sl_json_path_step steps[64];
  SL_ASSERT_EQ_LL(sl_json_parse_path(ctx, strlen(path), path, sizeof(steps), steps), 0);
  SL_ASSERT_ERROR_OCCURRED(ctx, "expected ] but got");
  return true;
}

SL_TEST_MAIN()
