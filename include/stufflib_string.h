#ifndef _STUFFLIB_STRING_H_INCLUDED
#define _STUFFLIB_STRING_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_data.h"
#include "stufflib_macros.h"

typedef struct stufflib_string stufflib_string;
struct stufflib_string {
  size_t length;
  stufflib_data data;
};

// Definition of well-formed UTF-8 sequences taken from the Unicode standard:
// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#G7404
// accessed 2023-02-18
bool stufflib_string_is_utf8(const size_t size,
                             const unsigned char data[const size]) {
  enum utf8_state {
    byte1_start = 0,
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
    ill_formed,
  } state = byte1_start;

  for (size_t i = 0; state != ill_formed && i < size; ++i) {
    const unsigned char byte = data[i];
    enum utf8_state current_state = state;
    state = ill_formed;
    switch (current_state) {
      case byte1_start: {
        if (byte <= 0x7f) {
          // 1-byte ASCII
          state = byte1_start;
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
          state = byte1_start;
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
          state = byte1_start;
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
          state = byte1_start;
        }
      } break;
      case ill_formed: {
        STUFFLIB_PRINT_ERROR("UTF-8 validator is in invalid state");
        assert(false);
        return false;
      }
    }
  }

  return state == byte1_start;
}

#endif  // _STUFFLIB_STRING_H_INCLUDED
