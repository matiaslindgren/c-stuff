#ifndef SL_UNICODE_H_INCLUDED
#define SL_UNICODE_H_INCLUDED
// Definition of well-formed UTF-8 sequences taken from the Unicode standard:
// "The Unicode® Standard Version 15.0" – Core Specification
// https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf#G7404
// accessed 2023-02-18
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib/iterator/iterator.h"
#include "stufflib/macros/macros.h"
#include "stufflib/span/span.h"

static const uint32_t sl_unicode_error_value = UINT32_MAX;
static const size_t sl_unicode_error_width = 0;

size_t sl_unicode_codepoint_width(uint32_t value) {
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
  return sl_unicode_error_width;
}

size_t sl_unicode_codepoint_width_from_utf8(size_t size,
                                            unsigned char bytes[const size]) {
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

  size_t max_width = SL_MIN(4, size);
  size_t width = 0;

  while (state != ill_formed && state != end && width < max_width) {
    unsigned char byte = bytes[width++];
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
      case end:
      case ill_formed:
        goto error;
    }
  }

  if (state == end) {
    return width;
  }

error:
  return sl_unicode_error_width;
}

uint32_t sl_unicode_codepoint_from_utf8(size_t width,
                                        unsigned char bytes[const width]) {
  switch (width) {
    case 4: {
      uint32_t byte1 = bytes[0] & 0x07;
      uint32_t byte2 = bytes[1] & 0x3f;
      uint32_t byte3 = bytes[2] & 0x3f;
      uint32_t byte4 = bytes[3] & 0x3f;
      return (byte1 << 18) | (byte2 << 12) | (byte3 << 6) | byte4;
    }
    case 3: {
      uint32_t byte1 = bytes[0] & 0x0f;
      uint32_t byte2 = bytes[1] & 0x3f;
      uint32_t byte3 = bytes[2] & 0x3f;
      return (byte1 << 12) | (byte2 << 6) | byte3;
    }
    case 2: {
      uint32_t byte1 = bytes[0] & 0x1f;
      uint32_t byte2 = bytes[1] & 0x3f;
      return (byte1 << 6) | byte2;
    }
    case 1: {
      uint32_t byte1 = bytes[0] & 0x7f;
      return byte1;
    }
    default: {
      return sl_unicode_error_value;
    }
  }
}

bool sl_unicode_is_valid_utf8(struct sl_span data[const static 1]) {
  size_t byte_pos = 0;
  while (byte_pos < data->size) {
    size_t codepoint_width =
        sl_unicode_codepoint_width_from_utf8(data->size - byte_pos,
                                             data->data + byte_pos);
    if (codepoint_width == sl_unicode_error_width) {
      return false;
    }
    byte_pos += codepoint_width;
  }
  return byte_pos == data->size;
}

size_t sl_unicode_iter_item_width(struct sl_iterator iter[const static 1]) {
  struct sl_span* data = iter->data;
  unsigned char* item = data->data + iter->index;
  return sl_unicode_codepoint_width_from_utf8(data->size - iter->index, item);
}

void sl_unicode_iter_advance(struct sl_iterator iter[const static 1]) {
  size_t codepoint_width = sl_unicode_iter_item_width(iter);
  if (codepoint_width == sl_unicode_error_width) {
    iter->index += 1;
  } else {
    iter->index += codepoint_width;
  }
  iter->pos += 1;
}

bool sl_unicode_iter_is_done(struct sl_iterator iter[const static 1]) {
  struct sl_span* data = iter->data;
  return iter->index >= data->size;
}

void* sl_unicode_iter_get(struct sl_iterator iter[const static 1]) {
  return sl_span_iter_get(iter);
}

uint32_t sl_unicode_iter_decode_item(struct sl_iterator iter[const static 1]) {
  return sl_unicode_codepoint_from_utf8(sl_unicode_iter_item_width(iter),
                                        sl_unicode_iter_get(iter));
}

struct sl_iterator sl_unicode_iter(struct sl_span data[const static 1]) {
  return (struct sl_iterator){.data = (void*)data};
}

size_t sl_unicode_length(struct sl_span data[const static 1]) {
  if (!sl_unicode_is_valid_utf8(data)) {
    return 0;
  }
  struct sl_iterator iter = sl_unicode_iter(data);
  while (!sl_unicode_iter_is_done(&iter)) {
    sl_unicode_iter_advance(&iter);
  }
  return iter.pos;
}

#endif  // SL_UNICODE_H_INCLUDED
