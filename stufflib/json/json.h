#ifndef SL_JSON_H_INCLUDED
#define SL_JSON_H_INCLUDED

#include <stddef.h>

#include <stufflib/context/context.h>
#include <stufflib/tensor/sl_tensor.h>

#ifndef SL_JSON_PARSE_MAX_DEPTH
  #define SL_JSON_PARSE_MAX_DEPTH 4'096
#endif

enum sl_json_type {
  sl_json_type_NONE = 0,
  sl_json_type_object,
  sl_json_type_array,
  sl_json_type_string,
  sl_json_type_number,
  sl_json_type_lit_true,
  sl_json_type_lit_false,
  sl_json_type_lit_null,
  sl_json_type_COUNT,
};

struct sl_json_node {
  enum sl_json_type type;
  size_t key_pos;       // byte offset of first char inside key quotes (0 if no key)
  size_t key_len;       // raw byte length of key between quotes (0 if no key)
  size_t value_pos;     // byte offset of value start in JSON string
  size_t value_len;     // byte length of value in JSON string
  size_t next_sibling;  // absolute index of next sibling (SIZE_MAX = none)
  size_t num_children;  // number of direct children (0 for leaves and empty containers)
};

SL_VECTOR_DECLARE(struct sl_json_node, sl_json_nodes)

enum sl_json_parse_state {
  // json: begin
  sl_json_element = 0,
  // value: object
  sl_json_object,
  // value: array
  sl_json_array,
  // value: string
  sl_json_string_body,
  sl_json_string_escape,
  // value: number
  sl_json_number_minus,
  sl_json_number_zero,
  sl_json_number_one_nine,
  sl_json_number_frac_first,
  sl_json_number_frac_digits,
  sl_json_number_exp_sign,
  sl_json_number_exp_first,
  sl_json_number_exp_digits,
  // intermediate tokens
  sl_json_member,
  sl_json_colon,
  sl_json_after_value,
  // end
  sl_json_done,
  sl_json_error,
};

enum sl_json_container {
  sl_json_container_object = 0,
  sl_json_container_array,
};

enum sl_json_parse_event {
  sl_json_NO_EVENT = 0,
  sl_json_begin_object,
  sl_json_end_object,
  sl_json_begin_array,
  sl_json_end_array,
  sl_json_value_string,
  sl_json_value_number,
  sl_json_value_lit_true,
  sl_json_value_lit_false,
  sl_json_value_lit_null,
};

struct sl_json_parser {
  enum sl_json_parse_state state;
  enum sl_json_container stack[SL_JSON_PARSE_MAX_DEPTH];
  enum sl_json_parse_event event;
  size_t pos;
  size_t depth;
  size_t value_begin;
  size_t string_begin;
  size_t pending_key_pos;
  size_t pending_key_len;
  bool string_is_key;
  bool has_pending_key;
};

struct sl_json_doc {
  struct sl_json_nodes nodes;
};

bool sl_json_is_valid(size_t len, const char json[const static len]);
size_t sl_json_count_nodes(size_t len, const char json[const static len]);

bool sl_json_read(
    struct sl_context ctx[static 1],
    size_t len,
    const char json[const static len],
    struct sl_json_doc doc[static 1]
);

void sl_json_doc_destroy(struct sl_json_doc doc[static 1]);

bool sl_json_is_str(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1]
);

bool sl_json_is_int(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1]
);

bool sl_json_get_str(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1],
    size_t out_len,
    char out[restrict static out_len]
);

bool sl_json_get_int(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1],
    long long out[restrict static 1]
);

#endif  // SL_JSON_H_INCLUDED
