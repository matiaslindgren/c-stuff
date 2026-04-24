#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <stufflib/json/json.h>

SL_VECTOR_IMPLEMENT(struct sl_json_node, sl_json_nodes)

static inline bool sl_json_is_ws(char ch) {
  return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

static inline bool sl_json_is_hex(char ch) {
  return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
}

static inline bool sl_json_consume_literal(
    struct sl_json_parser p[restrict static 1],
    size_t len,
    const char json[const restrict static len],
    const char pattern[const restrict static 1]
) {
  size_t i = 0;
  for (; pattern[i]; ++i) {
    if (p->pos + i >= len || json[p->pos + i] != pattern[i]) {
      return false;
    }
  }
  p->pos += i - 1;
  return true;
}

static inline bool sl_json_consume_hex_escape(
    struct sl_json_parser p[restrict static 1],
    size_t len,
    const char json[const restrict static len]
) {
  for (size_t i = 1; i <= 4; ++i) {
    if (p->pos + i >= len || !sl_json_is_hex(json[p->pos + i])) {
      return false;
    }
  }
  p->pos += 4;
  return true;
}

static inline bool sl_json_parse_advance(
    struct sl_json_parser p[restrict static 1],
    size_t len,
    const char json[const restrict static len]
) {
  enum sl_json_parse_state init_state = p->state;

  char ch  = (char)(p->pos < len ? json[p->pos] : 0);
  p->state = sl_json_error;
  p->event = sl_json_NO_EVENT;

  switch (init_state) {
    case sl_json_element: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_element;
        break;
      }
      switch (ch) {
        case '{': {
          p->value_begin       = p->pos;
          p->stack[p->depth++] = sl_json_container_object;
          p->state             = sl_json_object;
          p->event             = sl_json_begin_object;
        } break;
        case '[': {
          p->value_begin       = p->pos;
          p->stack[p->depth++] = sl_json_container_array;
          p->state             = sl_json_array;
          p->event             = sl_json_begin_array;
        } break;
        case '"': {
          p->value_begin   = p->pos;
          p->string_begin  = p->pos;
          p->string_is_key = false;
          p->state         = sl_json_string_body;
        } break;
        case 't': {
          p->value_begin = p->pos;
          if (sl_json_consume_literal(p, len, json, "true")) {
            p->state = sl_json_after_value;
            p->event = sl_json_value_lit_true;
          }
        } break;
        case 'f': {
          p->value_begin = p->pos;
          if (sl_json_consume_literal(p, len, json, "false")) {
            p->state = sl_json_after_value;
            p->event = sl_json_value_lit_false;
          }
        } break;
        case 'n': {
          p->value_begin = p->pos;
          if (sl_json_consume_literal(p, len, json, "null")) {
            p->state = sl_json_after_value;
            p->event = sl_json_value_lit_null;
          }
        } break;
        case '-': {
          p->value_begin = p->pos;
          p->state       = sl_json_number_minus;
        } break;
        case '0': {
          p->value_begin = p->pos;
          p->state       = sl_json_number_zero;
        } break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
          p->value_begin = p->pos;
          p->state       = sl_json_number_one_nine;
        } break;
        default: {
          p->state = sl_json_error;
        }
      }
    } break;

    case sl_json_object: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_object;
      } else if (ch == '}') {
        assert(p->depth);
        --(p->depth);
        p->state = sl_json_after_value;
        p->event = sl_json_end_object;
      } else if (ch == '"') {
        p->string_begin  = p->pos;
        p->string_is_key = true;
        p->state         = sl_json_string_body;
      }
    } break;

    case sl_json_member: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_member;
      } else if (ch == '"') {
        p->string_begin  = p->pos;
        p->string_is_key = true;
        p->state         = sl_json_string_body;
      }
    } break;

    case sl_json_colon: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_colon;
      } else if (ch == ':') {
        p->state = sl_json_element;
      }
    } break;

    case sl_json_array: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_array;
      } else if (ch == ']') {
        assert(p->depth);
        --(p->depth);
        p->state = sl_json_after_value;
        p->event = sl_json_end_array;
      } else {
        assert(p->pos);
        --(p->pos);
        p->state = sl_json_element;
      }
    } break;

    case sl_json_string_body: {
      if (ch == '"') {
        if (p->string_is_key) {
          p->pending_key_pos = p->string_begin + 1;
          p->pending_key_len = p->pos - p->string_begin - 1;
          p->has_pending_key = true;
          p->state           = sl_json_colon;
        } else {
          p->state = sl_json_after_value;
          p->event = sl_json_value_string;
        }
      } else if (ch == '\\') {
        p->state = sl_json_string_escape;
      } else if ((unsigned char)ch >= 0x20) {
        p->state = sl_json_string_body;
      }
    } break;

    case sl_json_string_escape: {
      switch (ch) {
        case '"':
        case '\\':
        case '/':
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't': {
          p->state = sl_json_string_body;
        } break;
        case 'u': {
          if (sl_json_consume_hex_escape(p, len, json)) {
            p->state = sl_json_string_body;
          }
        } break;
        default: {
        } break;
      }
    } break;

    case sl_json_number_minus: {
      if (ch == '0') {
        p->state = sl_json_number_zero;
      } else if ('1' <= ch && ch <= '9') {
        p->state = sl_json_number_one_nine;
      }
    } break;

    case sl_json_number_zero: {
      if (ch == '.') {
        p->state = sl_json_number_frac_first;
      } else if (ch == 'e' || ch == 'E') {
        p->state = sl_json_number_exp_sign;
      } else {
        assert(p->pos);
        --(p->pos);
        p->state = sl_json_after_value;
        p->event = sl_json_value_number;
      }
    } break;

    case sl_json_number_one_nine: {
      if ('0' <= ch && ch <= '9') {
        p->state = sl_json_number_one_nine;
      } else if (ch == '.') {
        p->state = sl_json_number_frac_first;
      } else if (ch == 'e' || ch == 'E') {
        p->state = sl_json_number_exp_sign;
      } else {
        assert(p->pos);
        --(p->pos);
        p->state = sl_json_after_value;
        p->event = sl_json_value_number;
      }
    } break;

    case sl_json_number_frac_first: {
      if ('0' <= ch && ch <= '9') {
        p->state = sl_json_number_frac_digits;
      }
    } break;

    case sl_json_number_frac_digits: {
      if ('0' <= ch && ch <= '9') {
        p->state = sl_json_number_frac_digits;
      } else if (ch == 'e' || ch == 'E') {
        p->state = sl_json_number_exp_sign;
      } else {
        assert(p->pos);
        --(p->pos);
        p->state = sl_json_after_value;
        p->event = sl_json_value_number;
      }
    } break;

    case sl_json_number_exp_sign: {
      if (ch == '+' || ch == '-') {
        p->state = sl_json_number_exp_first;
      } else if ('0' <= ch && ch <= '9') {
        p->state = sl_json_number_exp_digits;
      }
    } break;

    case sl_json_number_exp_first: {
      if ('0' <= ch && ch <= '9') {
        p->state = sl_json_number_exp_digits;
      }
    } break;

    case sl_json_number_exp_digits: {
      if ('0' <= ch && ch <= '9') {
        p->state = sl_json_number_exp_digits;
      } else {
        assert(p->pos);
        --(p->pos);
        p->state = sl_json_after_value;
        p->event = sl_json_value_number;
      }
    } break;

    case sl_json_after_value: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_after_value;
      } else if (p->depth == 0 && p->pos == len) {
        p->state = sl_json_done;
      } else if (p->depth && p->stack[p->depth - 1] == sl_json_container_object) {
        if (ch == ',') {
          p->state = sl_json_member;
        } else if (ch == '}') {
          assert(p->depth);
          --(p->depth);
          p->state = sl_json_after_value;
          p->event = sl_json_end_object;
        }
      } else if (p->depth && p->stack[p->depth - 1] == sl_json_container_array) {
        if (ch == ',') {
          p->state = sl_json_element;
        } else if (ch == ']') {
          assert(p->depth);
          --(p->depth);
          p->state = sl_json_after_value;
          p->event = sl_json_end_array;
        }
      }
    } break;

    case sl_json_done:
    case sl_json_error: {
    } break;
  }

  ++(p->pos);

  return p->state != sl_json_error && p->pos <= len && p->depth < SL_JSON_PARSE_MAX_DEPTH;
}

size_t sl_json_count_nodes(size_t len, const char json[const static len]) {
  struct sl_json_parser p = {.state = sl_json_element};
  size_t count            = 0;
  while (sl_json_parse_advance(&p, len, json)) {
    switch (p.event) {
      case sl_json_NO_EVENT:
      case sl_json_end_object:
      case sl_json_end_array: {
      } break;
      case sl_json_begin_object:
      case sl_json_begin_array:
      case sl_json_value_string:
      case sl_json_value_number:
      case sl_json_value_lit_true:
      case sl_json_value_lit_false:
      case sl_json_value_lit_null: {
        ++count;
      } break;
    }
  }
  return p.state == sl_json_done ? count : 0;
}

bool sl_json_is_valid(size_t len, const char json[const static len]) {
  return sl_json_count_nodes(len, json) > 0;
}

bool sl_json_read(
    struct sl_context ctx[static 1],
    size_t len,
    const char json[const static len],
    struct sl_json_doc doc[static 1]
) {
  size_t count = sl_json_count_nodes(len, json);
  if (!count) {
    SL_ERROR(ctx, "invalid or empty JSON");
    return false;
  }
  if (!sl_json_nodes_create(ctx, &doc->nodes, count)) {
    SL_ERROR(ctx, "failed creating %zu JSON nodes", count);
    return false;
  }

  // container_at[d]: node index of the open container at depth d
  // prev_sibling[d]: node index of the previous sibling emitted at depth d (SIZE_MAX = none)
  size_t container_at[SL_JSON_PARSE_MAX_DEPTH];
  size_t prev_sibling[SL_JSON_PARSE_MAX_DEPTH];
  prev_sibling[0] = SIZE_MAX;

  struct sl_json_parser p = {.state = sl_json_element};
  size_t reader_depth     = 0;
  size_t emit_index       = 0;

  while (sl_json_parse_advance(&p, len, json)) {
    switch (p.event) {
      case sl_json_NO_EVENT: {
      } break;

      case sl_json_end_object:
      case sl_json_end_array: {
        --reader_depth;
        sl_json_nodes_get(&doc->nodes, container_at[reader_depth])->value_len
            = p.pos - sl_json_nodes_get(&doc->nodes, container_at[reader_depth])->value_pos;
      } break;

      case sl_json_begin_object:
      case sl_json_begin_array:
      case sl_json_value_string:
      case sl_json_value_number:
      case sl_json_value_lit_true:
      case sl_json_value_lit_false:
      case sl_json_value_lit_null: {
        static const enum sl_json_type event_type[] = {
            [sl_json_begin_object]    = sl_json_type_object,
            [sl_json_begin_array]     = sl_json_type_array,
            [sl_json_value_string]    = sl_json_type_string,
            [sl_json_value_number]    = sl_json_type_number,
            [sl_json_value_lit_true]  = sl_json_type_lit_true,
            [sl_json_value_lit_false] = sl_json_type_lit_false,
            [sl_json_value_lit_null]  = sl_json_type_lit_null,
        };
        size_t idx               = emit_index++;
        struct sl_json_node node = {
            .type         = event_type[p.event],
            .value_pos    = p.value_begin,
            .value_len    = p.pos - p.value_begin,
            .next_sibling = SIZE_MAX,
            .num_children = 0,
        };
        if (p.has_pending_key) {
          node.key_pos      = p.pending_key_pos;
          node.key_len      = p.pending_key_len;
          p.has_pending_key = false;
        }
        // link to previous sibling at this depth
        if (prev_sibling[reader_depth] != SIZE_MAX) {
          sl_json_nodes_get(&doc->nodes, prev_sibling[reader_depth])->next_sibling = idx;
        }
        prev_sibling[reader_depth] = idx;
        // increment parent's child count
        if (reader_depth > 0) {
          sl_json_nodes_get(&doc->nodes, container_at[reader_depth - 1])->num_children++;
        }
        sl_json_nodes_set(&doc->nodes, idx, node);
        // push container frame
        if (node.type == sl_json_type_object || node.type == sl_json_type_array) {
          container_at[reader_depth] = idx;
          ++reader_depth;
          prev_sibling[reader_depth] = SIZE_MAX;
        }
      } break;
    }
  }

  return true;
}

void sl_json_doc_destroy(struct sl_json_doc doc[static 1]) {
  sl_json_nodes_destroy(&doc->nodes);
  *doc = (struct sl_json_doc){0};
}

static const struct sl_json_node* sl_json_find_key(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1],
    enum sl_json_type type
) {
  size_t key_len = strlen(key);
  size_t count   = sl_json_nodes_size(&doc->nodes);
  for (size_t i = 0; i < count; i++) {
    const struct sl_json_node* node = sl_json_nodes_get(&doc->nodes, i);
    if (node->type == type && node->key_len == key_len
        && memcmp(json + node->key_pos, key, key_len) == 0) {
      return node;
    }
  }
  return nullptr;
}

bool sl_json_is_str(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1]
) {
  return sl_json_find_key(doc, json, key, sl_json_type_string) != nullptr;
}

bool sl_json_is_int(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1]
) {
  return sl_json_find_key(doc, json, key, sl_json_type_number) != nullptr;
}

bool sl_json_get_str(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1],
    size_t out_len,
    char out[restrict static out_len]
) {
  const struct sl_json_node* node = sl_json_find_key(doc, json, key, sl_json_type_string);
  if (!node) {
    return false;
  }

  // value_pos points at the opening '"', value_len covers both quotes
  const char* src     = json + node->value_pos + 1;
  const char* src_end = json + node->value_pos + node->value_len - 1;
  size_t i_out        = 0;

  while (src < src_end && i_out < out_len) {
    char ch = *src++;
    if (ch != '\\') {
      out[i_out++] = ch;
      continue;
    }
    assert(src < src_end);
    switch ((ch = *src++)) {
      case '"': {
        out[i_out++] = '"';
      } break;
      case '\\': {
        out[i_out++] = '\\';
      } break;
      case 'n': {
        out[i_out++] = '\n';
      } break;
      case 'r': {
        out[i_out++] = '\r';
      } break;
      case 't': {
        out[i_out++] = '\t';
      } break;
      case 'b': {
        out[i_out++] = '\b';
      } break;
      case 'f': {
        out[i_out++] = '\f';
      } break;
      case 'u': {
        if (src + 4 > src_end) {
          return false;
        }
        char hex_buf[5] = {src[0], src[1], src[2], src[3], 0};
        out[i_out++]    = (char)strtoul(hex_buf, nullptr, 16);
        src += 4;
      } break;
      default: {
        __builtin_unreachable();
      }
    }
  }

  if (i_out >= out_len) {
    return false;
  }
  out[i_out] = 0;

  assert(src == src_end);
  return true;
}

bool sl_json_get_int(
    struct sl_json_doc doc[restrict static 1],
    const char json[restrict static 1],
    const char key[restrict static 1],
    long long out[restrict static 1]
) {
  const struct sl_json_node* node = sl_json_find_key(doc, json, key, sl_json_type_number);
  if (!node) {
    return false;
  }
  char* end = nullptr;
  *out      = strtoll(json + node->value_pos, &end, 10);
  return end != json + node->value_pos;
}
