#ifndef SL_JSON_H_INCLUDED
#define SL_JSON_H_INCLUDED
/*
 * non-allocating streaming JSON parser
 * can read invalid JSON as long as it is valid up to and including the searched key-value pair
 * duplicate keys are not considered invalid
 * sl_json_find will continue searching until a match is found or input is exhausted
 *
 */

#include <stddef.h>

#include <stufflib/context/context.h>

#ifndef SL_JSON_PARSE_MAX_DEPTH
  // maximum level of object nesting
  #define SL_JSON_PARSE_MAX_DEPTH 512
#endif

enum sl_json_type : signed char {
  sl_json_type_NONE = 0,
  sl_json_type_object,
  sl_json_type_array,
  sl_json_type_string,
  sl_json_type_number,
  sl_json_type_lit_true,
  sl_json_type_lit_false,
  sl_json_type_lit_null,
};

struct sl_json_path_step {
  bool is_index;
  // pointer to key chars (not null-terminated), valid when !is_index
  const char* key;
  // valid when !is_index
  size_t key_len;
  // array index, valid when is_index
  size_t index;
};

struct sl_json_node {
  // valid only when sl_json_find returned true
  enum sl_json_type type;
  // byte offset of value start in JSON string
  size_t value_begin;
  // byte length of value in JSON string
  size_t value_len;
};

enum sl_json_parse_state : signed char {
  // begin
  sl_json_element = 0,
  // values (literals are consumed whole)
  sl_json_object,
  sl_json_array,
  sl_json_string_value,
  sl_json_string_key,
  sl_json_string_escape_value,
  sl_json_string_escape_key,
  sl_json_number_minus,
  sl_json_number_zero,
  sl_json_number_one_nine,
  sl_json_number_frac_first,
  sl_json_number_frac_digits,
  sl_json_number_exp_sign,
  sl_json_number_exp_first,
  sl_json_number_exp_digits,
  // intermediate non-value states
  sl_json_colon,
  sl_json_member,
  sl_json_array_element,
  sl_json_after_value,
  // end
  sl_json_done,
  sl_json_error,
};

enum sl_json_container : signed char {
  sl_json_container_NONE = 0,
  sl_json_container_object,
  sl_json_container_array,
};

enum sl_json_event_type : signed char {
  sl_json_key = 0,
  sl_json_begin_node,
  sl_json_end_node,
  sl_json_value,
};

struct sl_json_event {
  bool emitted;
  enum sl_json_event_type type;
  enum sl_json_type node_type;
  size_t node_depth;
};

struct sl_json_parser {
  // current byte offset in the json string
  size_t pos;

  // byte offset to the current key
  size_t key_begin;
  // length of the current key
  size_t key_len;

  // byte offset to the current value
  size_t value_begin;

  // current parse state
  enum sl_json_parse_state state;

  // current event, only populated on discovery of a new node
  struct sl_json_event event;

  // current depth, increases on { and [, decreases on } and ]
  size_t depth;
  // container type at depth-1
  enum sl_json_container stack[SL_JSON_PARSE_MAX_DEPTH];
  // index of current array element at depth-1 (only valid if stack[depth-1] is an array)
  size_t array_pos[SL_JSON_PARSE_MAX_DEPTH];
};

bool sl_json_is_valid(size_t len, const char json[const static len]);
size_t sl_json_count_nodes(size_t len, const char json[const static len]);

size_t sl_json_parse_path(
    struct sl_context ctx[restrict static 1],
    size_t path_len,
    const char path[restrict static path_len],
    size_t capacity,
    struct sl_json_path_step steps[restrict static capacity]
);

bool sl_json_find_path(
    struct sl_context ctx[restrict static 1],
    size_t json_len,
    const char json[restrict static json_len],
    size_t n_steps,
    const struct sl_json_path_step steps[restrict static n_steps],
    struct sl_json_node node[restrict static 1]
);

bool sl_json_find(
    struct sl_context ctx[static 1],
    size_t json_len,
    const char json[restrict static json_len],
    size_t path_len,
    const char path[restrict static path_len],
    struct sl_json_node node[static 1]
);

bool sl_json_get_str(
    struct sl_context ctx[static 1],
    const struct sl_json_node node[static 1],
    const char json[static 1],
    size_t out_len,
    char out[static out_len]
);

bool sl_json_get_int(
    const struct sl_json_node node[static 1],
    const char json[static 1],
    long long out[static 1]
);

#endif  // SL_JSON_H_INCLUDED
