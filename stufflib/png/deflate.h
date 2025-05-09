#ifndef SL_DEFLATE_H_INCLUDED
#define SL_DEFLATE_H_INCLUDED
// DEFLATE PNG decoder.
// Only non-compressed encoding is implemented.
//
// Reference:
// 1. "RFC 1950" (Deutsch and Gailly, May 1996),
//    https://datatracker.ietf.org/doc/html/rfc1950,
//    accessed 2023-01-05
// 2. "RFC 1951" (Deutsch, May 1996),
//    https://datatracker.ietf.org/doc/html/rfc1951,
//    accessed 2023-01-05
// 3. "PNG (Portable Network Graphics) Specification, Version 1.2"
//    http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html,
//    accessed 2023-01-18

#define SL_DEFLATE_BLOCK_SIZE 8192

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib/hash/hash.h"
#include "stufflib/huffman/huffman.h"
#include "stufflib/macros/macros.h"
#include "stufflib/memory/memory.h"
#include "stufflib/misc/misc.h"
#include "stufflib/span/span.h"

struct sl_deflate_length_codes {
  size_t lengths[29];
  int extra[29];
};

struct sl_deflate_distance_codes {
  size_t distances[30];
  int extra[30];
};

struct sl_deflate_deflate_state {
  const struct sl_deflate_length_codes len_codes;
  const struct sl_deflate_distance_codes dist_codes;
  struct sl_span dst;
  const struct sl_span src;
  size_t dst_pos;
  size_t src_bit;
};

struct sl_deflate_length_codes sl_deflate_make_length_codes(void) {
  struct sl_deflate_length_codes codes = {0};
  codes.lengths[0] = 3;
  for (size_t i = 1; i < SL_ARRAY_LEN(codes.lengths); ++i) {
    const int extra_bits = (SL_MAX(1, (i - 1) / 4) - 1) % 6;
    codes.extra[i - 1] = extra_bits;
    codes.lengths[i] = codes.lengths[i - 1] + (1 << extra_bits);
  }
  codes.lengths[28] = 258;
  return codes;
}

struct sl_deflate_distance_codes sl_deflate_make_distance_codes(void) {
  struct sl_deflate_distance_codes codes = {0};
  codes.distances[0] = 1;
  for (unsigned i = 0; i < SL_ARRAY_LEN(codes.distances); ++i) {
    codes.extra[i] = SL_MAX(1, i / 2) - 1;
    if (i) {
      codes.distances[i] = codes.distances[i - 1] + (1 << codes.extra[i - 1]);
    }
  }
  return codes;
}

struct sl_huffman_tree sl_deflate_make_fixed_literal_tree(void) {
  const size_t max_literal = 287;
  size_t* code_lengths = sl_alloc(max_literal + 1, sizeof(size_t));
  {
    size_t symbol = 0;
    for (; symbol < 144; ++symbol) {
      code_lengths[symbol] = 8;
    }
    for (; symbol < 256; ++symbol) {
      code_lengths[symbol] = 9;
    }
    for (; symbol < 280; ++symbol) {
      code_lengths[symbol] = 7;
    }
    for (; symbol <= max_literal; ++symbol) {
      code_lengths[symbol] = 8;
    }
  }
  struct sl_huffman_tree tree = {0};
  sl_huffman_init(&tree, max_literal, code_lengths);
  sl_free(code_lengths);
  return tree;
}

struct sl_huffman_tree sl_deflate_make_fixed_distance_tree(void) {
  const size_t max_dist = 31;
  size_t* code_lengths = sl_alloc(max_dist + 1, sizeof(size_t));
  for (size_t symbol = 0; symbol <= max_dist; ++symbol) {
    code_lengths[symbol] = 5;
  }
  struct sl_huffman_tree tree = {0};
  sl_huffman_init(&tree, max_dist, code_lengths);
  sl_free(code_lengths);
  return tree;
}

size_t sl_deflate_next_bit(struct sl_deflate_deflate_state state[static 1]) {
  const size_t src_pos = state->src_bit / CHAR_BIT;
  const size_t src_bit = state->src_bit % CHAR_BIT;
  assert(src_pos < state->src.size);
  ++(state->src_bit);
  return (state->src.data[src_pos] >> src_bit) & 1;
}

size_t sl_deflate_next_n_bits(struct sl_deflate_deflate_state state[static 1],
                              const int count) {
  size_t res = 0;
  for (int bit = 0; bit < count; ++bit) {
    res |= sl_deflate_next_bit(state) << bit;
  }
  return res;
}

size_t sl_deflate_decode_next_code(
    const struct sl_huffman_tree codes[const static 1],
    struct sl_deflate_deflate_state state[static 1]) {
  size_t code = 0;
  for (size_t code_len = 1; code_len <= codes->max_code_len; ++code_len) {
    code = (code << 1) | sl_deflate_next_bit(state);
    if (sl_huffman_contains(codes, code, code_len)) {
      return sl_huffman_get(codes, code, code_len);
    }
  }
  return SIZE_MAX;
}

void sl_deflate_inflate_block(
    const struct sl_huffman_tree literal_tree[const static 1],
    const struct sl_huffman_tree distance_tree[const static 1],
    struct sl_deflate_deflate_state state[static 1]) {
  const size_t end_of_block = 256;

  while (true) {
    const size_t symbol = sl_deflate_decode_next_code(literal_tree, state);
    if (symbol < end_of_block) {
      assert(state->dst_pos < state->dst.size);
      state->dst.data[state->dst_pos++] = symbol & 0xff;
      continue;
    }
    if (symbol == end_of_block) {
      break;
    }
    assert(symbol < 286);
    const size_t len_symbol = symbol - 257;
    const size_t extra_len =
        sl_deflate_next_n_bits(state, state->len_codes.extra[len_symbol]);
    const size_t length = state->len_codes.lengths[len_symbol] + extra_len;

    const size_t dist_symbol =
        sl_deflate_decode_next_code(distance_tree, state);
    const size_t extra_dist =
        sl_deflate_next_n_bits(state, state->dist_codes.extra[dist_symbol]);
    const size_t back_distance =
        state->dist_codes.distances[dist_symbol] + extra_dist;

    const size_t copy_begin = state->dst_pos - back_distance;
    for (size_t i = copy_begin; i < copy_begin + length; ++i) {
      assert(state->dst_pos < state->dst.size);
      state->dst.data[state->dst_pos++] = state->dst.data[i];
    }
  }
}

bool sl_inflate_uncompressed_block(
    struct sl_deflate_deflate_state state[static 1]) {
  size_t src_byte_pos = state->src_bit / CHAR_BIT + 1;
  const size_t block_len =
      sl_misc_parse_lil_endian(2, state->src.data + src_byte_pos);
  src_byte_pos += 2;

  const size_t block_len_check =
      sl_misc_parse_lil_endian(2, state->src.data + src_byte_pos);
  src_byte_pos += 2;

  if ((~block_len & 0xffff) != block_len_check) {
    SL_LOG_ERROR("corrupted zlib block, ~LEN != NLEN");
    return false;
  }
  if (block_len > state->src.size - src_byte_pos) {
    SL_LOG_ERROR("corrupted zlib block, LEN too large");
    return false;
  }

  memcpy(state->dst.data + state->dst_pos,
         state->src.data + src_byte_pos,
         block_len);
  src_byte_pos += block_len;

  state->dst_pos += block_len;
  state->src_bit = src_byte_pos * CHAR_BIT;

  return true;
}

bool sl_inflate_dynamic_block(struct sl_deflate_deflate_state state[static 1]) {
  const size_t num_lengths = 257 + sl_deflate_next_n_bits(state, 5);
  const size_t num_distances = 1 + sl_deflate_next_n_bits(state, 5);
  const size_t num_length_lengths = 4 + sl_deflate_next_n_bits(state, 4);

  static const size_t length_order[] =
      {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  static const size_t max_length_length = 18;

  size_t* length_lengths = sl_alloc(max_length_length + 1, sizeof(size_t));
  for (size_t i = 0; i < num_length_lengths; ++i) {
    length_lengths[length_order[i]] = sl_deflate_next_n_bits(state, 3);
  }
  struct sl_huffman_tree length_tree = {0};
  sl_huffman_init(&length_tree, max_length_length, length_lengths);
  sl_free(length_lengths);

  size_t* dynamic_code_lengths =
      sl_alloc(num_lengths + num_distances, sizeof(size_t));
  for (size_t i = 0; i < num_lengths + num_distances;) {
    const size_t symbol = sl_deflate_decode_next_code(&length_tree, state);
    assert(symbol != SIZE_MAX);
    size_t code_len = 0;
    size_t num_repeats = 0;
    if (symbol < 16) {
      code_len = symbol;
      num_repeats = 1;
    } else if (symbol == 16) {
      assert(i > 0);
      code_len = dynamic_code_lengths[i - 1];
      num_repeats = 3 + sl_deflate_next_n_bits(state, 2);
    } else if (symbol == 17) {
      code_len = 0;
      num_repeats = 3 + sl_deflate_next_n_bits(state, 3);
    } else if (symbol == 18) {
      code_len = 0;
      num_repeats = 11 + sl_deflate_next_n_bits(state, 7);
    } else {
      SL_LOG_ERROR("unexpected symbol %zu in dynamic block", symbol);
      sl_free(dynamic_code_lengths);
      goto error;
    }
    for (size_t r = 0; r < num_repeats; ++r) {
      dynamic_code_lengths[i + r] = code_len;
    }
    i += num_repeats;
  }
  sl_huffman_destroy(&length_tree);

  const size_t max_length = num_lengths - 1;
  const size_t max_distance = num_distances - 1;

  struct sl_huffman_tree literal_tree = {0};
  sl_huffman_init(&literal_tree, max_length, dynamic_code_lengths);
  struct sl_huffman_tree distance_tree = {0};
  sl_huffman_init(&distance_tree,
                  max_distance,
                  dynamic_code_lengths + num_lengths);
  sl_free(dynamic_code_lengths);

  sl_deflate_inflate_block(&literal_tree, &distance_tree, state);
  sl_huffman_destroy(&literal_tree);
  sl_huffman_destroy(&distance_tree);
  return true;

error:
  return false;
}

bool sl_inflate_fixed_block(struct sl_deflate_deflate_state state[static 1]) {
  struct sl_huffman_tree literal_tree = sl_deflate_make_fixed_literal_tree();
  struct sl_huffman_tree distance_tree = sl_deflate_make_fixed_distance_tree();
  sl_deflate_inflate_block(&literal_tree, &distance_tree, state);
  sl_huffman_destroy(&literal_tree);
  sl_huffman_destroy(&distance_tree);
  return true;
}

size_t sl_inflate(struct sl_span dst, const struct sl_span src) {
  if (src.size < 3) {
    SL_LOG_ERROR("DEFLATE stream is too short");
    goto error;
  }
  if ((src.data[0] * 256 + src.data[1]) % 31) {
    SL_LOG_ERROR("DEFLATE stream is corrupted");
    goto error;
  }
  const int cmethod = src.data[0] & 0x0F;
  if (cmethod != 8) {
    SL_LOG_ERROR("unexpected compression method %d != 8", cmethod);
    goto error;
  }
  const int cinfo = (src.data[0] & 0xF0) >> 4;
  if (cinfo > 7) {
    SL_LOG_ERROR("too large compression info %d > 7", cinfo);
    goto error;
  }
  const int fdict = (src.data[1] & 0x20) >> 5;
  if (fdict) {
    SL_LOG_ERROR("dictionaries are not supported");
    goto error;
  }

  struct sl_deflate_deflate_state state = {
      .len_codes = sl_deflate_make_length_codes(),
      .dist_codes = sl_deflate_make_distance_codes(),
      .dst = dst,
      .src = src,
      .dst_pos = 0,
      .src_bit = 2 * CHAR_BIT,
  };

  for (bool is_final_block = false; !is_final_block;) {
    is_final_block = sl_deflate_next_bit(&state);
    const size_t block_type = sl_deflate_next_n_bits(&state, 2);
    switch (block_type) {
      case 0: {
        if (!sl_inflate_uncompressed_block(&state)) {
          SL_LOG_ERROR("failed inflating uncompressed block");
          goto error;
        }
      } break;
      case 1: {
        if (!sl_inflate_fixed_block(&state)) {
          SL_LOG_ERROR("failed inflating fixed block");
          goto error;
        }
      } break;
      case 2: {
        if (!sl_inflate_dynamic_block(&state)) {
          SL_LOG_ERROR("failed inflating dynamic block");
          goto error;
        }
      } break;
      default: {
        SL_LOG_ERROR("invalid block type %zu", block_type);
        goto error;
      } break;
    }
  }

  /* const size_t src_adler32 = sl_deflate_next_n_bits(state, 32); */
  /* const size_t dst_adler32 = */
  /*     sl_hash_adler32(state->dst.size, state->dst.data); */
  /* if (dst_adler32 != src_adler32) { */
  /*   SL_LOG_ERROR("output stream adler32 %zu not equal to expected
   * %zu", */
  /*                        dst_adler32, */
  /*                        src_adler32); */
  /*   goto error; */
  /* } */

  return state.dst_pos;

error:
  return 0;
}

#define SL_DEFLATE_BLOCK_SIZE 8192

size_t sl_deflate_uncompressed(struct sl_span dst, const struct sl_span src) {
  const size_t block_size = SL_DEFLATE_BLOCK_SIZE;
  size_t dst_pos = 0;
  size_t src_pos = 0;

  const int cmethod = 8;
  const int cinfo = 7;
  const int cmf = (cinfo << 4) | cmethod;
  dst.data[dst_pos++] = cmf;
  dst.data[dst_pos++] = 31 - (cmf * 256) % 31;

  for (bool is_final_block = false; !is_final_block;) {
    assert(src_pos < src.size);
    is_final_block = (src_pos + block_size) >= src.size;
    dst.data[dst_pos++] = is_final_block & 1;

    const size_t dist_to_end = src.size - src_pos;
    const uint16_t block_len = SL_MIN(block_size, dist_to_end) & 0xffff;

    const unsigned char* block_len_bytes =
        sl_misc_encode_lil_endian(2, (unsigned char[2]){0}, block_len);
    const unsigned char* block_len_check_bytes =
        sl_misc_encode_lil_endian(2, (unsigned char[2]){0}, ~block_len);

    memcpy(dst.data + dst_pos, block_len_bytes, 2);
    dst_pos += 2;
    memcpy(dst.data + dst_pos, block_len_check_bytes, 2);
    dst_pos += 2;

    memcpy(dst.data + dst_pos, src.data + src_pos, block_len);
    dst_pos += block_len;
    src_pos += block_len;
  }

  const unsigned char* adler32 =
      sl_misc_encode_big_endian(4,
                                (unsigned char[4]){0},
                                sl_hash_adler32(src.size, src.data));
  memcpy(dst.data + dst_pos, adler32, 4);
  dst_pos += 4;

  return dst_pos;
}

#endif  // SL_DEFLATE_H_INCLUDED
