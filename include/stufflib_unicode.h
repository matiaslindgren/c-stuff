#ifndef _STUFFLIB_UNICODE_H_INCLUDED
#define _STUFFLIB_UNICODE_H_INCLUDED
// Definition of well-formed UTF-8 sequences taken from the Unicode standard:
// "The Unicode® Standard Version 15.0" – Core Specification
// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#G7404
// accessed 2023-02-18
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "stufflib_data.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"

#ifndef __STDC_ISO_10646__
#error ”wchar_t wide characters have to be Unicode code points”
#endif
#ifdef __STDC_MB_MIGHT_NEQ_WC__
#error ”basic character codes must agree on char and wchar_t”
#endif

const wchar_t stufflib_unicode_error_value = WEOF;

size_t stufflib_unicode_codepoint_width(const wchar_t value) {
  if (value < 0x000080) {
    return 1;
  }
  if (value < 0x000800) {
    return 2;
  }
  if (value < 0x010000) {
    return 3;
  }
  if (value < 0x110000) {
    return 4;
  }
  return 0;
}

size_t stufflib_unicode_codepoint_width_from_utf8(
    const size_t size,
    const unsigned char bytes[const size]) {
  // See Table 3-7 in Chapter 3
  enum decode_state {
    start = 0,
    byte2_c2,
    byte2_e,
    byte2_e0,
    byte2_ed,
    byte2_f,
    byte2_f0,
    byte2_f4,
    byte3_e,
    byte3_f,
    byte4_f,
    end,
    ill_formed,
  } state = start;

  size_t width = 0;

  while (state != ill_formed && state != end && width < 4 && width < size) {
    const unsigned char byte = bytes[width++];
    enum decode_state current_state = state;
    state = ill_formed;
    switch (current_state) {
      case start: {
        if (byte <= 0x7f) {
          // 1-byte ASCII
          state = end;
        } else if (0xc2 <= byte && byte <= 0xdf) {
          // 2-byte code point
          state = byte2_c2;
        } else if (0xe0 <= byte && byte <= 0xef) {
          // 3-byte code point
          if (byte == 0xe0) {
            state = byte2_e0;
          } else if (byte == 0xed) {
            state = byte2_ed;
          } else {
            state = byte2_e;
          }
        } else if (0xf0 <= byte && byte <= 0xf4) {
          // 4-byte code point
          if (byte == 0xf0) {
            state = byte2_f0;
          } else if (byte == 0xf4) {
            state = byte2_f4;
          } else {
            state = byte2_f;
          }
        }
      } break;
      case byte2_c2: {
        if (0x80 <= byte && byte <= 0xbf) {
          state = end;
        }
      } break;
      case byte2_e: {
        if (0x80 <= byte && byte <= 0xbf) {
          state = byte3_e;
        }
      } break;
      case byte2_e0: {
        if (0xa0 <= byte && byte <= 0xbf) {
          state = byte3_e;
        }
      } break;
      case byte2_ed: {
        if (0x80 <= byte && byte <= 0x9f) {
          state = byte3_e;
        }
      } break;
      case byte3_e: {
        if (0x80 <= byte && byte <= 0xbf) {
          state = end;
        }
      } break;
      case byte2_f0: {
        if (0x90 <= byte && byte <= 0xbf) {
          state = byte3_f;
        }
      } break;
      case byte2_f4: {
        if (0x80 <= byte && byte <= 0x8f) {
          state = byte3_f;
        }
      } break;
      case byte2_f: {
        if (0x80 <= byte && byte <= 0xbf) {
          state = byte3_f;
        }
      } break;
      case byte3_f: {
        if (0x80 <= byte && byte <= 0xbf) {
          state = byte4_f;
        }
      } break;
      case byte4_f: {
        if (0x80 <= byte && byte <= 0xbf) {
          state = end;
        }
      } break;
      default: {
        goto error;
      }
    }
  }

  if (state == end) {
    return width;
  }

error:
  return 0;
}

wchar_t stufflib_unicode_codepoint_from_utf8(
    const size_t width,
    const unsigned char bytes[const width]) {
  switch (width) {
    case 4: {
      const wchar_t byte1 = bytes[0] & 0x07;
      const wchar_t byte2 = bytes[1] & 0x3f;
      const wchar_t byte3 = bytes[2] & 0x3f;
      const wchar_t byte4 = bytes[3] & 0x3f;
      return (byte1 << 18) | (byte2 << 12) | (byte3 << 6) | byte4;
    }
    case 3: {
      const wchar_t byte1 = bytes[0] & 0x0f;
      const wchar_t byte2 = bytes[1] & 0x3f;
      const wchar_t byte3 = bytes[2] & 0x3f;
      return (byte1 << 12) | (byte2 << 6) | byte3;
    }
    case 2: {
      const wchar_t byte1 = bytes[0] & 0x1f;
      const wchar_t byte2 = bytes[1] & 0x3f;
      return (byte1 << 6) | byte2;
    }
    case 1: {
      const wchar_t byte1 = bytes[0] & 0x7f;
      return byte1;
    }
    default: {
      return stufflib_unicode_error_value;
    }
  }
}

bool stufflib_unicode_is_valid_utf8(const stufflib_data data[const static 1]) {
  size_t byte_pos = 0;
  while (byte_pos < data->size) {
    const size_t codepoint_width =
        stufflib_unicode_codepoint_width_from_utf8(data->size - byte_pos,
                                                   data->data + byte_pos);
    if (codepoint_width == 0) {
      return false;
    }
    byte_pos += codepoint_width;
  }
  return byte_pos == data->size;
}

size_t stufflib_unicode_iter_item_width(
    stufflib_iterator iter[const static 1]) {
  const stufflib_data* data = (const stufflib_data*)(iter->begin);
  const unsigned char* item = data->data + iter->index;
  return stufflib_unicode_codepoint_width_from_utf8(data->size - iter->index,
                                                    item);
}

void stufflib_unicode_iter_advance(stufflib_iterator iter[const static 1]) {
  // TODO find next valid code point instead of advance by 1
  const size_t codepoint_width = stufflib_unicode_iter_item_width(iter);
  iter->index += STUFFLIB_MAX(1, codepoint_width);
  iter->pos += 1;
}

bool stufflib_unicode_iter_end(stufflib_iterator iter[const static 1]) {
  const stufflib_data* data = (const stufflib_data*)(iter->begin);
  return iter->index >= data->size;
}

void* stufflib_unicode_iter_get(stufflib_iterator iter[const static 1]) {
  return stufflib_data_iter_get(iter);
}

wchar_t stufflib_unicode_iter_decode_item(
    stufflib_iterator iter[const static 1]) {
  return stufflib_unicode_codepoint_from_utf8(
      stufflib_unicode_iter_item_width(iter),
      stufflib_unicode_iter_get(iter));
}

stufflib_iterator stufflib_unicode_iter(
    const stufflib_data data[const static 1]) {
  return (stufflib_iterator){
      .begin = (void*)data,
      .get = stufflib_unicode_iter_get,
      .advance = stufflib_unicode_iter_advance,
      .end = stufflib_unicode_iter_end,
  };
}

size_t stufflib_unicode_length(const stufflib_data data[const static 1]) {
  if (!stufflib_unicode_is_valid_utf8(data)) {
    return 0;
  }
  stufflib_iterator iter = stufflib_unicode_iter(data);
  while (!stufflib_unicode_iter_end(&iter)) {
    stufflib_unicode_iter_advance(&iter);
  }
  return iter.pos;
}

#endif  // _STUFFLIB_UNICODE_H_INCLUDED
