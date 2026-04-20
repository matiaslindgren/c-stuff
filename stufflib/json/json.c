#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/json/json.h>

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
    const char item                 = *pos;
    enum sl_json_find_state current = state;
    state                           = seek_open_quote;
    switch (current) {
      case seek_open_quote: {
        if (item == '"') {
          key_pos = 0;
          state   = match_key;
        } else {
          state = seek_open_quote;
        }
      } break;
      case match_key: {
        if (item == '"') {
          state = seek_colon;
        } else if (key_pos < key_len && item == key[key_pos]) {
          key_pos++;
          state = match_key;
        } else {
          state = no_match;
        }
      } break;
      case no_match: {
        if (item == '"') {
          state = seek_open_quote;
        } else {
          state = no_match;
        }
      } break;
      case seek_colon: {
        if (item == ':') {
          state = found;
        } else if (item == ' ' || item == '\t' || item == '\n') {
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

static inline bool sl_json_is_hex(char c) {
  return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
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
    char item                      = *pos++;
    enum sl_json_str_state current = state;
    state                          = error;
    switch (current) {
      case start: {
        if (item == '"') {
          state = normal;
        }
      } break;
      case normal: {
        switch (item) {
          case '"': {
            state = done;
          } break;
          case '\\': {
            state = escape;
          } break;
          default: {
            out[i_out++] = item;
            state        = normal;
          } break;
        }
        break;
      }
      case escape: {
        switch (item) {
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
        if (sl_json_is_hex(item)) {
          hex_buf[0] = item;
          state      = unicode_2;
        }
      } break;
      case unicode_2: {
        if (sl_json_is_hex(item)) {
          hex_buf[1] = item;
          state      = unicode_3;
        }
      } break;
      case unicode_3: {
        if (sl_json_is_hex(item)) {
          hex_buf[2] = item;
          state      = unicode_4;
        }
      } break;
      case unicode_4: {
        if (sl_json_is_hex(item)) {
          hex_buf[3]   = item;
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
