#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stufflib/json/json.h>
#include <stufflib/testing/testing.h>

#define SL_JSON_VALID(s)   SL_ASSERT_TRUE(sl_json_is_valid(strlen(s), s))
#define SL_JSON_INVALID(s) SL_ASSERT_FALSE(sl_json_is_valid(strlen(s), s))

// sl_json_is_valid: valid inputs

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

// sl_json_is_valid: invalid inputs

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

// sl_json_is_str / sl_json_is_int / sl_json_get_str / sl_json_get_int

#define SL_JSON_SETUP(json_literal)        \
  const char* json       = (json_literal); \
  struct sl_json_doc doc = {0};            \
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc))

#define SL_JSON_TEARDOWN() sl_json_doc_destroy(&doc)

SL_TEST(test_json_get_str) {
  SL_JSON_SETUP("{\"key\":\"hello\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_is_str(&doc, json, "key"));
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "hello");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_quote) {
  SL_JSON_SETUP("{\"key\":\"\\\"hello\\\"\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\"hello\"");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_backslash) {
  SL_JSON_SETUP("{\"key\":\"\\\\\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\\");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_newline) {
  SL_JSON_SETUP("{\"key\":\"\\n\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\n");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_carriage_return) {
  SL_JSON_SETUP("{\"key\":\"\\r\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\r");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_tab) {
  SL_JSON_SETUP("{\"key\":\"\\t\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\t");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_backspace) {
  SL_JSON_SETUP("{\"key\":\"\\b\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\b");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_form_feed) {
  SL_JSON_SETUP("{\"key\":\"\\f\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "\f");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_escaped_unicode) {
  SL_JSON_SETUP("{\"key\":\"\\u0001\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "key", sizeof(val), val));
  SL_ASSERT_EQ_LL((unsigned char)val[0], 0x01);
  SL_ASSERT_EQ_LL(val[1], '\0');
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_middle_key) {
  SL_JSON_SETUP("{\"a\":\"first\",\"b\":\"second\",\"c\":\"third\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "b", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "second");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_last_key) {
  SL_JSON_SETUP("{\"a\":\"first\",\"b\":\"second\",\"c\":\"third\"}");
  char val[64];
  SL_ASSERT_TRUE(sl_json_get_str(&doc, json, "c", sizeof(val), val));
  SL_ASSERT_EQ_STR(val, "third");
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_str_not_found) {
  SL_JSON_SETUP("{\"key\":\"hello\"}");
  char val[64];
  SL_ASSERT_FALSE(sl_json_is_str(&doc, json, "other"));
  SL_ASSERT_FALSE(sl_json_get_str(&doc, json, "other", sizeof(val), val));
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_int) {
  SL_JSON_SETUP("{\"line\":42}");
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_is_int(&doc, json, "line"));
  SL_ASSERT_TRUE(sl_json_get_int(&doc, json, "line", &val));
  SL_ASSERT_EQ_LL(val, 42);
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_int_middle_key) {
  SL_JSON_SETUP("{\"a\":1,\"b\":2,\"c\":3}");
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&doc, json, "b", &val));
  SL_ASSERT_EQ_LL(val, 2);
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_int_last_key) {
  SL_JSON_SETUP("{\"a\":1,\"b\":2,\"c\":3}");
  long long val = 0;
  SL_ASSERT_TRUE(sl_json_get_int(&doc, json, "c", &val));
  SL_ASSERT_EQ_LL(val, 3);
  SL_JSON_TEARDOWN();
  return true;
}

SL_TEST(test_json_get_int_not_found) {
  SL_JSON_SETUP("{\"line\":42}");
  long long val = 0;
  SL_ASSERT_FALSE(sl_json_is_int(&doc, json, "other"));
  SL_ASSERT_FALSE(sl_json_get_int(&doc, json, "other", &val));
  SL_JSON_TEARDOWN();
  return true;
}

// sl_json_count_nodes

SL_TEST(test_json_count_invalid) {
  (void)ctx;
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen(""), ""), 0);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("hello"), "hello"), 0);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("{"), "{"), 0);
  return true;
}

SL_TEST(test_json_count_scalars) {
  (void)ctx;
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("null"), "null"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("true"), "true"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("false"), "false"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("42"), "42"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("3.14"), "3.14"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("\"hello\""), "\"hello\""), 1);
  return true;
}

SL_TEST(test_json_count_containers) {
  (void)ctx;
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("{}"), "{}"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("[]"), "[]"), 1);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("{\"a\":1}"), "{\"a\":1}"), 2);
  SL_ASSERT_EQ_LL(sl_json_count_nodes(strlen("[1,2,3]"), "[1,2,3]"), 4);
  SL_ASSERT_EQ_LL(
      sl_json_count_nodes(strlen("{\"a\":{\"x\":1},\"d\":2}"), "{\"a\":{\"x\":1},\"d\":2}"),
      4
  );
  return true;
}

// sl_json_read

SL_TEST(test_read_invalid) {
  struct sl_json_doc doc = {0};
  SL_ASSERT_FALSE(sl_json_read(ctx, strlen(""), "", &doc));
  sl_context_unwind_errors(ctx, stderr);
  SL_ASSERT_FALSE(sl_json_read(ctx, strlen("{"), "{", &doc));
  sl_context_unwind_errors(ctx, stderr);
  return true;
}

SL_TEST(test_read_null) {
  const char* json       = "null";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 1);
  struct sl_json_node* n = sl_json_nodes_get(&doc.nodes, 0);
  SL_ASSERT_EQ_LL(n->type, sl_json_type_lit_null);
  SL_ASSERT_EQ_LL(n->num_children, 0);
  SL_ASSERT_TRUE(n->next_sibling == SIZE_MAX);
  SL_ASSERT_EQ_LL(n->value_pos, 0);
  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_true) {
  const char* json       = "true";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->type, sl_json_type_lit_true);
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->value_pos, 0);
  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_number) {
  const char* json       = "42";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->type, sl_json_type_number);
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->value_pos, 0);
  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_string) {
  const char* json       = "\"hello\"";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->type, sl_json_type_string);
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->value_pos, 0);
  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_empty_object) {
  const char* json       = "{}";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 1);
  struct sl_json_node* n = sl_json_nodes_get(&doc.nodes, 0);
  SL_ASSERT_EQ_LL(n->type, sl_json_type_object);
  SL_ASSERT_EQ_LL(n->num_children, 0);
  SL_ASSERT_TRUE(n->next_sibling == SIZE_MAX);
  SL_ASSERT_EQ_LL(n->value_pos, 0);
  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_empty_array) {
  const char* json       = "[]";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 1);
  struct sl_json_node* n = sl_json_nodes_get(&doc.nodes, 0);
  SL_ASSERT_EQ_LL(n->type, sl_json_type_array);
  SL_ASSERT_EQ_LL(n->num_children, 0);
  SL_ASSERT_TRUE(n->next_sibling == SIZE_MAX);
  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_flat_object) {
  // {"a":1,"b":2}
  // node 0: object, 2 children
  // node 1: number "a", next_sibling=2
  // node 2: number "b", next_sibling=SIZE_MAX
  const char* json       = "{\"a\":1,\"b\":2}";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 3);

  struct sl_json_node* obj = sl_json_nodes_get(&doc.nodes, 0);
  SL_ASSERT_EQ_LL(obj->type, sl_json_type_object);
  SL_ASSERT_EQ_LL(obj->num_children, 2);
  SL_ASSERT_TRUE(obj->next_sibling == SIZE_MAX);
  SL_ASSERT_EQ_LL(obj->value_pos, 0);

  struct sl_json_node* a = sl_json_nodes_get(&doc.nodes, 1);
  SL_ASSERT_EQ_LL(a->type, sl_json_type_number);
  SL_ASSERT_EQ_LL(a->key_pos, 2);  // first char of "a"
  SL_ASSERT_EQ_LL(a->key_len, 1);
  SL_ASSERT_EQ_LL(a->next_sibling, 2);

  struct sl_json_node* b = sl_json_nodes_get(&doc.nodes, 2);
  SL_ASSERT_EQ_LL(b->type, sl_json_type_number);
  SL_ASSERT_EQ_LL(b->key_pos, 8);  // first char of "b"
  SL_ASSERT_EQ_LL(b->key_len, 1);
  SL_ASSERT_TRUE(b->next_sibling == SIZE_MAX);

  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_nested_object) {
  // {"a":{"x":1},"b":2}
  // node 0: object, 2 children
  // node 1: object "a", 1 child, next_sibling=3
  // node 2: number "x", next_sibling=SIZE_MAX
  // node 3: number "b", next_sibling=SIZE_MAX
  const char* json       = "{\"a\":{\"x\":1},\"b\":2}";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 4);

  struct sl_json_node* outer = sl_json_nodes_get(&doc.nodes, 0);
  SL_ASSERT_EQ_LL(outer->type, sl_json_type_object);
  SL_ASSERT_EQ_LL(outer->num_children, 2);
  SL_ASSERT_TRUE(outer->next_sibling == SIZE_MAX);

  struct sl_json_node* inner = sl_json_nodes_get(&doc.nodes, 1);
  SL_ASSERT_EQ_LL(inner->type, sl_json_type_object);
  SL_ASSERT_EQ_LL(inner->key_len, 1);
  SL_ASSERT_EQ_LL(inner->num_children, 1);
  SL_ASSERT_EQ_LL(inner->next_sibling, 3);

  struct sl_json_node* x = sl_json_nodes_get(&doc.nodes, 2);
  SL_ASSERT_EQ_LL(x->type, sl_json_type_number);
  SL_ASSERT_EQ_LL(x->key_len, 1);
  SL_ASSERT_TRUE(x->next_sibling == SIZE_MAX);

  struct sl_json_node* b = sl_json_nodes_get(&doc.nodes, 3);
  SL_ASSERT_EQ_LL(b->type, sl_json_type_number);
  SL_ASSERT_EQ_LL(b->key_len, 1);
  SL_ASSERT_TRUE(b->next_sibling == SIZE_MAX);

  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_array) {
  // [1,2,3]
  // node 0: array, 3 children
  // node 1: number, next_sibling=2
  // node 2: number, next_sibling=3
  // node 3: number, next_sibling=SIZE_MAX
  const char* json       = "[1,2,3]";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 4);

  struct sl_json_node* arr = sl_json_nodes_get(&doc.nodes, 0);
  SL_ASSERT_EQ_LL(arr->type, sl_json_type_array);
  SL_ASSERT_EQ_LL(arr->num_children, 3);
  SL_ASSERT_TRUE(arr->next_sibling == SIZE_MAX);

  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 1)->next_sibling, 2);
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 2)->next_sibling, 3);
  SL_ASSERT_TRUE(sl_json_nodes_get(&doc.nodes, 3)->next_sibling == SIZE_MAX);

  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_array_of_objects) {
  // [{"a":1},{"b":2}]
  // node 0: array, 2 children
  // node 1: object, 1 child, next_sibling=3
  // node 2: number "a", next_sibling=SIZE_MAX
  // node 3: object, 1 child, next_sibling=SIZE_MAX
  // node 4: number "b", next_sibling=SIZE_MAX
  const char* json       = "[{\"a\":1},{\"b\":2}]";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 5);

  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->num_children, 2);
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 1)->num_children, 1);
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 1)->next_sibling, 3);
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 3)->num_children, 1);
  SL_ASSERT_TRUE(sl_json_nodes_get(&doc.nodes, 3)->next_sibling == SIZE_MAX);

  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST(test_read_value_pos) {
  // Check value_pos for various types in an array
  // [null,true,false,42,"hi"]
  //  0    5    10    16  19
  const char* json       = "[null,true,false,42,\"hi\"]";
  struct sl_json_doc doc = {0};
  SL_ASSERT_TRUE(sl_json_read(ctx, strlen(json), json, &doc));
  SL_ASSERT_EQ_LL(sl_json_nodes_size(&doc.nodes), 6);

  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 0)->value_pos, 0);   // [
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 1)->value_pos, 1);   // null
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 2)->value_pos, 6);   // true
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 3)->value_pos, 11);  // false
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 4)->value_pos, 17);  // 42
  SL_ASSERT_EQ_LL(sl_json_nodes_get(&doc.nodes, 5)->value_pos, 20);  // "hi"

  sl_json_doc_destroy(&doc);
  return true;
}

SL_TEST_MAIN()
