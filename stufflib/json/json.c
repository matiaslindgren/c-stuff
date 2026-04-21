#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <stufflib/json/json.h>

#ifndef SL_JSON_PARSE_MAX_DEPTH
  #define SL_JSON_PARSE_MAX_DEPTH 4096
#endif

static inline bool sl_json_is_ws(char ch) {
  return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

static inline bool sl_json_is_hex(char ch) {
  return ('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
}

bool sl_json_parse(size_t len, const char json[const static len]) {
  enum sl_json_parse_state {
    // json: begin
    element,
    // value: object
    object,
    // value: array
    array,
    // value: string
    string_body,
    string_escape,
    string_u1,
    string_u2,
    string_u3,
    string_u4,
    // value: number
    number_minus,
    number_zero,
    number_one_nine,
    number_frac_first,
    number_frac_digits,
    number_exp_sign,
    number_exp_first,
    number_exp_digits,
    // value: literal true
    lit_true_r,
    lit_true_u,
    lit_true_e,
    // value: literal false
    lit_false_a,
    lit_false_l,
    lit_false_s,
    lit_false_e,
    // value: literal null
    lit_null_u,
    lit_null_l1,
    lit_null_l2,
    // intermediate tokens
    member,
    colon,
    after_value,
    // end
    done,
    error,
  };
  enum sl_json_parse_container {
    container_object,
    container_array,
  };

  enum sl_json_parse_state state = element;
  enum sl_json_parse_container stack[SL_JSON_PARSE_MAX_DEPTH];
  bool string_is_key = false;
  size_t pos         = 0;
  size_t depth       = 0;
  for (; state != error && pos <= len && depth < SL_JSON_PARSE_MAX_DEPTH;) {
    char ch                          = (char)(pos < len ? json[pos] : 0);
    enum sl_json_parse_state current = state;
    state                            = error;
    switch (current) {
      case element: {
        if (sl_json_is_ws(ch)) {
          state = element;
          break;
        }
        switch (ch) {
          case '{': {
            stack[depth++] = container_object;
            state          = object;
          } break;
          case '[': {
            stack[depth++] = container_array;
            state          = array;
          } break;
          case '"': {
            string_is_key = false;
            state         = string_body;
          } break;
          case 't': {
            state = lit_true_r;
          } break;
          case 'f': {
            state = lit_false_a;
          } break;
          case 'n': {
            state = lit_null_u;
          } break;
          case '-': {
            state = number_minus;
          } break;
          case '0': {
            state = number_zero;
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
            state = number_one_nine;
          } break;
          default: {
            state = error;
          }
        }
      } break;

      case object: {
        if (sl_json_is_ws(ch)) {
          state = object;
        } else if (ch == '}') {
          assert(depth);
          --depth;
          state = after_value;
        } else if (ch == '"') {
          string_is_key = true;
          state         = string_body;
        }
      } break;

      case member: {
        if (sl_json_is_ws(ch)) {
          state = member;
        } else if (ch == '"') {
          string_is_key = true;
          state         = string_body;
        }
      } break;

      case colon: {
        if (sl_json_is_ws(ch)) {
          state = colon;
        } else if (ch == ':') {
          state = element;
        }
      } break;

      case array: {
        if (sl_json_is_ws(ch)) {
          state = array;
        } else if (ch == ']') {
          assert(depth);
          --depth;
          state = after_value;
        } else {
          assert(pos);
          --pos;
          state = element;
        }
      } break;

      case string_body: {
        if (ch == '"') {
          state = string_is_key ? colon : after_value;
        } else if (ch == '\\') {
          state = string_escape;
        } else if ((unsigned char)ch >= 0x20) {
          state = string_body;
        }
      } break;

      case string_escape: {
        switch (ch) {
          case '"':
          case '\\':
          case '/':
          case 'b':
          case 'f':
          case 'n':
          case 'r':
          case 't': {
            state = string_body;
          } break;
          case 'u': {
            state = string_u1;
          } break;
          default: {
          } break;
        }
      } break;

      case string_u1: {
        if (sl_json_is_hex(ch)) {
          state = string_u2;
        }
      } break;
      case string_u2: {
        if (sl_json_is_hex(ch)) {
          state = string_u3;
        }
      } break;
      case string_u3: {
        if (sl_json_is_hex(ch)) {
          state = string_u4;
        }
      } break;
      case string_u4: {
        if (sl_json_is_hex(ch)) {
          state = string_body;
        }
      } break;

      case number_minus: {
        if (ch == '0') {
          state = number_zero;
        } else if ('1' <= ch && ch <= '9') {
          state = number_one_nine;
        }
      } break;

      case number_zero: {
        if (ch == '.') {
          state = number_frac_first;
        } else if (ch == 'e' || ch == 'E') {
          state = number_exp_sign;
        } else {
          assert(pos);
          --pos;
          state = after_value;
        }
      } break;

      case number_one_nine: {
        if ('0' <= ch && ch <= '9') {
          state = number_one_nine;
        } else if (ch == '.') {
          state = number_frac_first;
        } else if (ch == 'e' || ch == 'E') {
          state = number_exp_sign;
        } else {
          assert(pos);
          --pos;
          state = after_value;
        }
      } break;

      case number_frac_first: {
        if ('0' <= ch && ch <= '9') {
          state = number_frac_digits;
        }
      } break;

      case number_frac_digits: {
        if ('0' <= ch && ch <= '9') {
          state = number_frac_digits;
        } else if (ch == 'e' || ch == 'E') {
          state = number_exp_sign;
        } else {
          assert(pos);
          --pos;
          state = after_value;
        }
      } break;

      case number_exp_sign: {
        if (ch == '+' || ch == '-') {
          state = number_exp_first;
        } else if ('0' <= ch && ch <= '9') {
          state = number_exp_digits;
        }
      } break;

      case number_exp_first: {
        if ('0' <= ch && ch <= '9') {
          state = number_exp_digits;
        }
      } break;

      case number_exp_digits: {
        if ('0' <= ch && ch <= '9') {
          state = number_exp_digits;
        } else {
          assert(pos);
          --pos;
          state = after_value;
        }
      } break;

      case lit_true_r: {
        if (ch == 'r') {
          state = lit_true_u;
        }
      } break;
      case lit_true_u: {
        if (ch == 'u') {
          state = lit_true_e;
        }
      } break;
      case lit_true_e: {
        if (ch == 'e') {
          state = after_value;
        }
      } break;

      case lit_false_a: {
        if (ch == 'a') {
          state = lit_false_l;
        }
      } break;
      case lit_false_l: {
        if (ch == 'l') {
          state = lit_false_s;
        }
      } break;
      case lit_false_s: {
        if (ch == 's') {
          state = lit_false_e;
        }
      } break;
      case lit_false_e: {
        if (ch == 'e') {
          state = after_value;
        }
      } break;
      case lit_null_u: {
        if (ch == 'u') {
          state = lit_null_l1;
        }
      } break;
      case lit_null_l1: {
        if (ch == 'l') {
          state = lit_null_l2;
        }
      } break;
      case lit_null_l2: {
        if (ch == 'l') {
          state = after_value;
        }
      } break;

      case after_value: {
        if (sl_json_is_ws(ch)) {
          state = after_value;
        } else if (depth == 0 && pos == len) {
          state = done;
        } else if (depth && stack[depth - 1] == container_object) {
          if (ch == ',') {
            state = member;
          } else if (ch == '}') {
            assert(depth);
            --depth;
            state = after_value;
          }
        } else if (depth && stack[depth - 1] == container_array) {
          if (ch == ',') {
            state = element;
          } else if (ch == ']') {
            assert(depth);
            --depth;
            state = after_value;
          }
        }
      } break;

      case done:
      case error: {
      } break;
    }
    ++pos;
  }

  return state == done;
}

static inline const char* sl_json_find_value(
    const char json[const restrict static 1],
    const char key[const restrict static 1]
) {
  enum sl_json_find_state {
    seek_open_quote,
    match_key,
    no_match,
    seek_colon,
    found,
  };
  enum sl_json_find_state state = seek_open_quote;

  size_t key_len  = strlen(key);
  const char* pos = json;
  for (size_t key_pos = 0; *pos && state != found && key_pos <= key_len; ++pos) {
    const char ch                   = *pos;
    enum sl_json_find_state current = state;
    state                           = seek_open_quote;
    switch (current) {
      case seek_open_quote: {
        if (ch == '"') {
          key_pos = 0;
          state   = match_key;
        } else {
          state = seek_open_quote;
        }
      } break;
      case match_key: {
        if (ch == '"') {
          state = seek_colon;
        } else if (key_pos < key_len && ch == key[key_pos]) {
          key_pos++;
          state = match_key;
        } else {
          state = no_match;
        }
      } break;
      case no_match: {
        if (ch == '"') {
          state = seek_open_quote;
        } else {
          state = no_match;
        }
      } break;
      case seek_colon: {
        if (ch == ':') {
          state = found;
        } else if (sl_json_is_ws(ch)) {
          state = seek_colon;
        }
      } break;
      case found: {
      } break;
    }
  }

  if (state != found) {
    return nullptr;
  }
  return pos;
}

bool sl_json_str(
    const char json[const restrict static 1],
    const char key[const restrict static 1],
    size_t len,
    char out[const restrict static len]
) {
  const char* pos = sl_json_find_value(json, key);
  if (!pos) {
    return false;
  }

  enum sl_json_str_state {
    start,
    normal,
    escape,
    unicode_1,
    unicode_2,
    unicode_3,
    unicode_4,
    done,
    error,
  };
  enum sl_json_str_state state = start;

  char hex_buf[5] = {0};
  size_t i_out    = 0;

  while (*pos && i_out < len && state != done && state != error) {
    char ch                        = *pos++;
    enum sl_json_str_state current = state;
    state                          = error;
    switch (current) {
      case start: {
        if (ch == '"') {
          state = normal;
        }
      } break;
      case normal: {
        switch (ch) {
          case '"': {
            state = done;
          } break;
          case '\\': {
            state = escape;
          } break;
          default: {
            out[i_out++] = ch;
            state        = normal;
          } break;
        }
        break;
      }
      case escape: {
        switch (ch) {
          case '"': {
            out[i_out++] = '"';
            state        = normal;
          } break;
          case '\\': {
            out[i_out++] = '\\';
            state        = normal;
          } break;
          case 'n': {
            out[i_out++] = '\n';
            state        = normal;
          } break;
          case 'r': {
            out[i_out++] = '\r';
            state        = normal;
          } break;
          case 't': {
            out[i_out++] = '\t';
            state        = normal;
          } break;
          case 'b': {
            out[i_out++] = '\b';
            state        = normal;
          } break;
          case 'f': {
            out[i_out++] = '\f';
            state        = normal;
          } break;
          case 'u': {
            state = unicode_1;
          } break;
          default: {
          } break;
        }
      } break;
      case unicode_1: {
        if (sl_json_is_hex(ch)) {
          hex_buf[0] = ch;
          state      = unicode_2;
        }
      } break;
      case unicode_2: {
        if (sl_json_is_hex(ch)) {
          hex_buf[1] = ch;
          state      = unicode_3;
        }
      } break;
      case unicode_3: {
        if (sl_json_is_hex(ch)) {
          hex_buf[2] = ch;
          state      = unicode_4;
        }
      } break;
      case unicode_4: {
        if (sl_json_is_hex(ch)) {
          hex_buf[3]   = ch;
          out[i_out++] = (char)strtoul(hex_buf, nullptr, 16);
          state        = normal;
        }
      } break;
      case done:
      case error: {
      } break;
    }
  }

  if (i_out >= len || state != done) {
    return false;
  }

  out[i_out] = 0;
  return true;
}

bool sl_json_int(
    const char json[const restrict static 1],
    const char key[const restrict static 1],
    long long out[const restrict static 1]
) {
  const char* pos = sl_json_find_value(json, key);
  if (!pos) {
    return false;
  }
  char* end = nullptr;
  *out      = strtoll(pos, &end, 10);
  return end != pos;
}
