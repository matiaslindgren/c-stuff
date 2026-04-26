#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <stufflib/json/json.h>

static inline bool sl_json_is_ws(char ch) {
  return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

static inline bool sl_json_is_hex(char ch) {
  return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
}

static inline bool sl_json_is_path_key(char ch) {
  return '!' <= ch && ch <= '~' && ch != '"' && ch != '.' && ch != '[' && ch != ']';
}

static inline void sl_json_parse_unget(struct sl_json_parser p[static 1]) {
  assert(p->pos);
  --(p->pos);
}

static inline bool sl_json_consume_literal(
    struct sl_json_parser p[restrict static 1],
    size_t len,
    const char json[restrict static len],
    const char pattern[restrict static 1]
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
    const char json[restrict static len]
) {
  for (size_t i = 1; i <= 4; ++i) {
    if (p->pos + i >= len || !sl_json_is_hex(json[p->pos + i])) {
      return false;
    }
  }
  p->pos += 4;
  return true;
}

static inline void
sl_json_parse_emit_literal(struct sl_json_parser p[static 1], enum sl_json_type literal_type) {
  p->state = sl_json_after_value;
  p->event = (struct sl_json_event){
      .emitted    = true,
      .type       = sl_json_value,
      .node_depth = p->depth,
      .node_type  = literal_type,
  };
}

static inline void sl_json_parse_emit_number(struct sl_json_parser p[static 1]) {
  sl_json_parse_unget(p);
  p->state = sl_json_after_value;
  p->event = (struct sl_json_event){
      .emitted    = true,
      .type       = sl_json_value,
      .node_depth = p->depth,
      .node_type  = sl_json_type_number,
  };
}

static inline void sl_json_parse_emit_end_container(
    struct sl_json_parser p[static 1],
    enum sl_json_type container_type
) {
  assert(p->depth);
  --(p->depth);
  p->state = sl_json_after_value;
  p->event = (struct sl_json_event){
      .emitted    = true,
      .type       = sl_json_end_node,
      .node_depth = p->depth,
      .node_type  = container_type,
  };
}

static inline bool sl_json_parse_advance(
    struct sl_json_parser p[restrict static 1],
    size_t len,
    const char json[restrict static len]
) {
  enum sl_json_parse_state current_state = p->state;

  char ch  = (char)(p->pos < len ? json[p->pos] : 0);
  // always assume ch is invalid and switch to a valid state only if ch is ok
  p->state = sl_json_error;
  p->event = (struct sl_json_event){0};

  switch (current_state) {
    case sl_json_element: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_element;
        break;
      }

      switch (ch) {
        case '{': {
          p->state = sl_json_object;
          p->event = (struct sl_json_event){
              .emitted    = true,
              .type       = sl_json_begin_node,
              .node_depth = p->depth,
              .node_type  = sl_json_type_object,
          };
          p->value_begin     = p->pos;
          p->stack[p->depth] = sl_json_container_object;
          ++(p->depth);
        } break;

        case '[': {
          p->state = sl_json_array;
          p->event = (struct sl_json_event){
              .emitted    = true,
              .type       = sl_json_begin_node,
              .node_depth = p->depth,
              .node_type  = sl_json_type_array,
          };
          p->value_begin         = p->pos;
          p->stack[p->depth]     = sl_json_container_array;
          p->array_pos[p->depth] = SIZE_MAX;
          ++(p->depth);
        } break;

        case '"': {
          p->state       = sl_json_string_value;
          p->value_begin = p->pos;
        } break;

        case 't': {
          p->value_begin = p->pos;
          if (sl_json_consume_literal(p, len, json, "true")) {
            sl_json_parse_emit_literal(p, sl_json_type_lit_true);
          }
        } break;

        case 'f': {
          p->value_begin = p->pos;
          if (sl_json_consume_literal(p, len, json, "false")) {
            sl_json_parse_emit_literal(p, sl_json_type_lit_false);
          }
        } break;

        case 'n': {
          p->value_begin = p->pos;
          if (sl_json_consume_literal(p, len, json, "null")) {
            sl_json_parse_emit_literal(p, sl_json_type_lit_null);
          }
        } break;

        case '-': {
          p->state       = sl_json_number_minus;
          p->value_begin = p->pos;
        } break;

        case '0': {
          p->state       = sl_json_number_zero;
          p->value_begin = p->pos;
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
          p->state       = sl_json_number_one_nine;
          p->value_begin = p->pos;
        } break;

        default: {
          p->state = sl_json_error;
        }
      }
    } break;  // case sl_json_element

    case sl_json_object: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_object;
      } else if (ch == '}') {
        assert(p->depth);
        --(p->depth);
        p->state = sl_json_after_value;
        p->event = (struct sl_json_event){
            .emitted    = true,
            .type       = sl_json_end_node,
            .node_depth = p->depth,
            .node_type  = sl_json_type_object,
        };
      } else if (ch == '"') {
        sl_json_parse_unget(p);
        p->state = sl_json_member;
      }
    } break;

    case sl_json_member: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_member;
      } else if (ch == '"') {
        p->state       = sl_json_string_key;
        p->value_begin = p->pos;
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
        p->event = (struct sl_json_event){
            .emitted    = true,
            .type       = sl_json_end_node,
            .node_depth = p->depth,
            .node_type  = sl_json_type_array,
        };
      } else {
        sl_json_parse_unget(p);
        p->state = sl_json_array_element;
      }
    } break;

    case sl_json_array_element: {
      if (sl_json_is_ws(ch)) {
        p->state = sl_json_array_element;
      } else {
        assert(p->depth);
        sl_json_parse_unget(p);
        p->state = sl_json_element;
        ++(p->array_pos[p->depth - 1]);
      }
    } break;

    case sl_json_string_value: {
      if (ch == '"') {
        p->state = sl_json_after_value;
        p->event = (struct sl_json_event){
            .emitted    = true,
            .type       = sl_json_value,
            .node_depth = p->depth,
            .node_type  = sl_json_type_string,
        };
      } else if (ch == '\\') {
        p->state = sl_json_string_escape_value;
      } else if ((unsigned char)ch >= 0x20) {
        p->state = sl_json_string_value;
      }
    } break;

    case sl_json_string_key: {
      if (ch == '"') {
        p->state = sl_json_colon;
        p->event = (struct sl_json_event){
            .emitted    = true,
            .type       = sl_json_key,
            .node_depth = p->depth,
            .node_type  = sl_json_type_string,
        };
        p->key_begin = p->value_begin + 1;
        p->key_len   = p->pos - p->value_begin - 1;
      } else if (ch == '\\') {
        p->state = sl_json_string_escape_key;
      } else if ((unsigned char)ch >= 0x20) {
        p->state = sl_json_string_key;
      }
    } break;

    case sl_json_string_escape_value:
    case sl_json_string_escape_key: {
      enum sl_json_parse_state body = current_state == sl_json_string_escape_value
                                          ? sl_json_string_value
                                          : sl_json_string_key;
      switch (ch) {
        case '"':
        case '\\':
        case '/':
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't': {
          p->state = body;
        } break;
        case 'u': {
          if (sl_json_consume_hex_escape(p, len, json)) {
            p->state = body;
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
        sl_json_parse_emit_number(p);
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
        sl_json_parse_emit_number(p);
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
        sl_json_parse_emit_number(p);
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
        sl_json_parse_emit_number(p);
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
          sl_json_parse_emit_end_container(p, sl_json_type_object);
        }
      } else if (p->depth && p->stack[p->depth - 1] == sl_json_container_array) {
        if (ch == ',') {
          p->state = sl_json_array_element;
        } else if (ch == ']') {
          sl_json_parse_emit_end_container(p, sl_json_type_array);
        }
      }
    } break;

    case sl_json_done:
    case sl_json_error: {
    } break;
  }

  return p->state != sl_json_error && ++(p->pos) <= len && p->depth < SL_JSON_PARSE_MAX_DEPTH;
}

size_t sl_json_count_nodes(size_t len, const char json[const static len]) {
  struct sl_json_parser p = {.state = sl_json_element};
  size_t count            = 0;
  while (sl_json_parse_advance(&p, len, json)) {
    count += (int)(p.event.emitted
                   && (p.event.type == sl_json_begin_node || p.event.type == sl_json_value));
  }
  return p.state == sl_json_done ? count : 0;
}

bool sl_json_is_valid(size_t len, const char json[const static len]) {
  return sl_json_count_nodes(len, json) > 0;
}

enum sl_json_path_parse_state {
  sl_json_path_step,
  sl_json_path_key_begin,
  sl_json_path_key,
  sl_json_path_idx_begin,
  sl_json_path_idx_digits,
  sl_json_path_idx_end,
  sl_json_path_done,
  sl_json_path_error,
};

size_t sl_json_parse_path(
    struct sl_context ctx[restrict static 1],
    size_t path_len,
    const char path[restrict static path_len],
    size_t capacity,
    struct sl_json_path_step steps[restrict static capacity]
) {
  if (path_len == 0) {
    SL_ERROR(ctx, "empty JSON path");
    return 0;
  }

  enum sl_json_path_parse_state state = sl_json_path_step;
  size_t path_steps                   = 0;
  size_t key_begin                    = 0;
  size_t idx_value                    = 0;

  for (size_t pos = 0; state != sl_json_path_done && state != sl_json_path_error && pos <= path_len
                       && path_steps < capacity;) {
    enum sl_json_path_parse_state current_state = state;

    char ch = (char)(pos < path_len ? path[pos] : 0);
    state   = sl_json_path_error;

    switch (current_state) {
      case sl_json_path_step: {
        switch (ch) {
          case 0: {
            state = sl_json_path_done;
          } break;
          case '.': {
            state     = sl_json_path_key_begin;
            key_begin = ++pos;
          } break;
          case '[': {
            state = sl_json_path_idx_begin;
            ++pos;
          } break;
          default: {
            SL_ERROR(
                ctx,
                "expected . or [ but got %#x at index %zu in JSON path %s",
                ch,
                pos,
                path
            );
          } break;
        }
      } break;

      case sl_json_path_key_begin: {
        if (sl_json_is_path_key(ch)) {
          state = sl_json_path_key;
        } else {
          SL_ERROR(
              ctx,
              "expected key character after . but got %#x at index %zu in JSON path %s",
              ch,
              pos,
              path
          );
        }
      } break;

      case sl_json_path_key: {
        if (sl_json_is_path_key(ch)) {
          state = sl_json_path_key;
          ++pos;
        } else {
          state               = sl_json_path_step;
          // finalize object key
          steps[path_steps++] = (struct sl_json_path_step){
              .is_index = false,
              .key      = path + key_begin,
              .key_len  = pos - key_begin,
          };
        }
      } break;

      case sl_json_path_idx_begin: {
        idx_value = 0;
        if ('1' <= ch && ch <= '9') {
          state = sl_json_path_idx_digits;
        } else if (ch == '0') {
          state = sl_json_path_idx_end;
          ++pos;
        } else {
          SL_ERROR(
              ctx,
              "expected array index digit after '[' but got %#x at index %zu in JSON path %s",
              ch,
              pos,
              path
          );
        }
      } break;

      case sl_json_path_idx_digits: {
        if ('0' <= ch && ch <= '9') {
          state     = sl_json_path_idx_digits;
          idx_value = (idx_value * 10) + (ch - '0');
          ++pos;
        } else {
          state = sl_json_path_idx_end;
        }
      } break;

      case sl_json_path_idx_end: {
        if (ch == ']') {
          state               = sl_json_path_step;
          // finalize array index
          steps[path_steps++] = (struct sl_json_path_step){
              .is_index = true,
              .index    = idx_value,
          };
          ++pos;
        } else {
          SL_ERROR(ctx, "expected ] but got %x at index %zu in JSON path %s", ch, pos, path);
        }
      } break;

      case sl_json_path_done:
      case sl_json_path_error: {
      } break;
    }
  }

  if (path_steps >= capacity) {
    SL_ERROR(ctx, "buffer overflow at capacity %zu while parsing JSON path %s", capacity, path);
    return 0;
  }
  if (state == sl_json_path_error) {
    return 0;
  }
  return path_steps;
}

bool sl_json_find_path(
    struct sl_context ctx[restrict static 1],
    size_t json_len,
    const char json[restrict static json_len],
    size_t n_steps,
    const struct sl_json_path_step steps[restrict static n_steps],
    struct sl_json_node node[restrict static 1]
) {
  struct sl_json_parser p = {0};
  size_t step_idx         = 0;

  // PHASE 1: navigate to the target node

  while (step_idx < n_steps && sl_json_parse_advance(&p, json_len, json)) {
    if (!p.event.emitted) {
      // parsing in progress
      continue;
    }

    if (p.event.node_depth < step_idx + 1) {
      // not searching for values at this level
      if (p.event.type != sl_json_begin_node) {
        // the previous path we descended into is gone, restart search
        // (if we didn't allow JSON with duplicate keys, we could early exit here)
        step_idx = 0;
      }
      // else: new node [ or {, skip and handle value on next iteration
      continue;
    } else if (p.event.node_depth > step_idx + 1) {
      // not searching for values at this level
      continue;
    }

    struct sl_json_path_step step = steps[step_idx];

    bool object_key_matches
        = (p.event.type == sl_json_key && !step.is_index && p.key_len == step.key_len
           && memcmp(json + p.key_begin, step.key, step.key_len) == 0);

    bool array_index_matches
        = (step.is_index && p.event.node_depth
           && p.stack[p.event.node_depth - 1] == sl_json_container_array
           && p.array_pos[p.event.node_depth - 1] == step.index);

    if (object_key_matches || array_index_matches) {
      ++step_idx;
    }
  }

  if (step_idx != n_steps) {
    goto not_found;
  }

  // PHASE 2: extract value at target node

  if (p.event.type == sl_json_key) {
    // advance parser until value
    while (sl_json_parse_advance(&p, json_len, json)
           && (!p.event.emitted || p.event.type == sl_json_key)) {
    }
  }

  if (p.state == sl_json_error) {
    goto not_found;
  }

  *node = (struct sl_json_node){
      .type        = p.event.node_type,
      .value_begin = p.value_begin,
      .value_len   = 0,
  };
  if (p.event.type == sl_json_value) {
    // non-container value => the parser already knows the length
    node->value_len = p.pos - p.value_begin;
    return true;
  }
  // container value => we need to continue parsing until closing ] or }

  // on sl_json_begin_node, depth is one greater than the event node_depth
  // keep parsing below this depth until we return to it
  size_t target_depth = p.event.node_depth;
  while (sl_json_parse_advance(&p, json_len, json)) {
    if (p.event.emitted && p.event.type == sl_json_end_node && p.event.node_depth == target_depth) {
      // we found the container, store its full length
      node->value_len = p.pos - node->value_begin;
      return true;
    }
  }

not_found:
  *node = (struct sl_json_node){0};
  if (p.state == sl_json_error) {
    SL_ERROR(ctx, "invalid JSON");
  }
  return false;
}

bool sl_json_find(
    struct sl_context ctx[static 1],
    size_t json_len,
    const char json[restrict static json_len],
    size_t path_len,
    const char path[restrict static path_len],
    struct sl_json_node node[static 1]
) {
  struct sl_json_path_step steps[SL_JSON_PARSE_MAX_DEPTH];
  size_t n_steps = sl_json_parse_path(ctx, path_len, path, sizeof(steps), steps);
  if (n_steps == 0) {
    return false;
  }
  return sl_json_find_path(ctx, json_len, json, n_steps, steps, node);
}

bool sl_json_get_str(
    struct sl_context ctx[static 1],
    const struct sl_json_node node[static 1],
    const char json[static 1],
    size_t out_len,
    char out[static out_len]
) {
  if (node->type != sl_json_type_string) {
    SL_ERROR(ctx, "node is not a string");
    return false;
  }

  // value_begin points at the opening '"', value_len covers both quotes
  const char* src     = json + node->value_begin + 1;
  const char* src_end = json + node->value_begin + node->value_len - 1;
  size_t i_out        = 0;

  while (src < src_end && i_out < out_len) {
    char ch = *src++;
    if (ch != '\\') {
      // not escaped, copy and go to next char
      out[i_out++] = ch;
      continue;
    }
    // ch is \ so it escapes something
    // go to next char and handle each escape sequence separately
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
        // 4 byte unicode escape
        assert(src + 4 <= src_end);
        char hex_buf[5] = {src[0], src[1], src[2], src[3], 0};
        out[i_out++]    = (char)strtoul(hex_buf, nullptr, 16);
        src += 4;
      } break;
      default: {
        assert(false);
        __builtin_unreachable();
      }
    }
  }

  if (i_out >= out_len) {
    SL_ERROR(ctx, "output buffer too small");
    return false;
  }
  out[i_out] = 0;

  assert(src == src_end);
  return true;
}

bool sl_json_get_int(
    const struct sl_json_node node[static 1],
    const char json[static 1],
    long long out[static 1]
) {
  if (node->type != sl_json_type_number) {
    return false;
  }
  char* end = nullptr;
  *out      = strtoll(json + node->value_begin, &end, 10);
  return end != json + node->value_begin;
}
