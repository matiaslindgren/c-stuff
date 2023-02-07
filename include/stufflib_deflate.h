#ifndef _STUFFLIB_DEFLATE_H_INCLUDED
#define _STUFFLIB_DEFLATE_H_INCLUDED
// DEFLATE decoder.
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_hash.h"
#include "stufflib_huffman.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"

typedef struct _length_codes _length_codes;
struct _length_codes {
  size_t lengths[29];
  int extra[29];
};

typedef struct _distance_codes _distance_codes;
struct _distance_codes {
  size_t distances[30];
  int extra[30];
};

typedef struct _deflate_state _deflate_state;
struct _deflate_state {
  const _length_codes len_codes;
  const _distance_codes dist_codes;
  stufflib_data dst;
  const stufflib_data src;
  size_t dst_pos;
  size_t src_bit;
};

static _length_codes _make_length_codes() {
  _length_codes codes = {0};
  codes.lengths[0] = 3;
  for (size_t i = 1; i < STUFFLIB_ARRAY_LEN(codes.lengths); ++i) {
    const int extra_bits = (STUFFLIB_MAX(1, (i - 1) / 4) - 1) % 6;
    codes.extra[i - 1] = extra_bits;
    codes.lengths[i] = codes.lengths[i - 1] + (1 << extra_bits);
  }
  codes.lengths[28] = 258;
  return codes;
}

static _distance_codes _make_distance_codes() {
  _distance_codes codes = {0};
  codes.distances[0] = 1;
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(codes.distances); ++i) {
    codes.extra[i] = STUFFLIB_MAX(1, i / 2) - 1;
    if (i) {
      codes.distances[i] = codes.distances[i - 1] + (1 << codes.extra[i - 1]);
    }
  }
  return codes;
}

static stufflib_huffman_tree _make_fixed_literal_tree() {
  const size_t max_literal = 287;
  size_t* code_lengths = calloc(max_literal + 1, sizeof(size_t));
  if (!code_lengths) {
    goto error;
  }
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
  stufflib_huffman_tree tree = (stufflib_huffman_tree){0};
  if (!stufflib_huffman_init(&tree, max_literal, code_lengths)) {
    free(code_lengths);
    goto error;
  }
  free(code_lengths);
  return tree;

error:
  STUFFLIB_PRINT_ERROR(
      "failed building fixed literal huffman codes for DEFLATE");
  return (stufflib_huffman_tree){0};
}

static stufflib_huffman_tree _make_fixed_distance_tree() {
  const size_t max_dist = 31;
  size_t* code_lengths = calloc(max_dist + 1, sizeof(size_t));
  if (!code_lengths) {
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_dist; ++symbol) {
    code_lengths[symbol] = 5;
  }
  stufflib_huffman_tree tree = (stufflib_huffman_tree){0};
  if (!stufflib_huffman_init(&tree, max_dist, code_lengths)) {
    free(code_lengths);
    goto error;
  }
  free(code_lengths);
  return tree;

error:
  STUFFLIB_PRINT_ERROR(
      "failed building fixed distance huffman codes for DEFLATE");
  return (stufflib_huffman_tree){0};
}

static size_t _next_bit(_deflate_state state[static 1]) {
  const size_t src_pos = state->src_bit / CHAR_BIT;
  const size_t src_bit = state->src_bit % CHAR_BIT;
  assert(src_pos < state->src.size);
  ++(state->src_bit);
  return (state->src.data[src_pos] >> src_bit) & 1;
}

static size_t _next_n_bits(_deflate_state state[static 1], const size_t count) {
  size_t res = 0;
  for (size_t i = 0; i < count; ++i) {
    res |= _next_bit(state) << i;
  }
  return res;
}

static size_t _decode_next_code(
    const stufflib_huffman_tree codes[const static 1],
    _deflate_state state[static 1]) {
  size_t code = 0;
  for (size_t code_len = 1; code_len <= codes->max_code_len; ++code_len) {
    code = (code << 1) | _next_bit(state);
    if (stufflib_huffman_contains(codes, code, code_len)) {
      return stufflib_huffman_get(codes, code, code_len);
    }
  }
  return SIZE_MAX;
}

static void _inflate_block(
    const stufflib_huffman_tree literal_tree[const static 1],
    const stufflib_huffman_tree distance_tree[const static 1],
    _deflate_state state[static 1]) {
  const size_t end_of_block = 256;

  while (1) {
    const size_t symbol = _decode_next_code(literal_tree, state);
    if (symbol < end_of_block) {
      assert(state->dst_pos < state->dst.size);
      state->dst.data[state->dst_pos++] = symbol & STUFFLIB_ONES(1);
      continue;
    }
    if (symbol == end_of_block) {
      break;
    }
    assert(symbol < 286);
    const size_t len_symbol = symbol - 257;
    const size_t extra_len =
        _next_n_bits(state, state->len_codes.extra[len_symbol]);
    const size_t length = state->len_codes.lengths[len_symbol] + extra_len;

    const size_t dist_symbol = _decode_next_code(distance_tree, state);
    const size_t extra_dist =
        _next_n_bits(state, state->dist_codes.extra[dist_symbol]);
    const size_t back_distance =
        state->dist_codes.distances[dist_symbol] + extra_dist;

    const size_t copy_begin = state->dst_pos - back_distance;
    for (size_t i = copy_begin; i < copy_begin + length; ++i) {
      assert(state->dst_pos < state->dst.size);
      state->dst.data[state->dst_pos++] = state->dst.data[i];
    }
  }
}

int stufflib_inflate_uncompressed_block(_deflate_state state[static 1]) {
  size_t src_byte_pos = state->src_bit / CHAR_BIT + 1;
  const size_t block_len =
      stufflib_misc_parse_lil_endian(2, state->src.data + src_byte_pos);
  src_byte_pos += 2;

  const size_t block_len_check =
      stufflib_misc_parse_lil_endian(2, state->src.data + src_byte_pos);
  src_byte_pos += 2;

  if ((~block_len & STUFFLIB_ONES(2)) != block_len_check) {
    STUFFLIB_PRINT_ERROR("corrupted zlib block, ~LEN != NLEN");
    return 0;
  }
  if (block_len > state->src.size - src_byte_pos) {
    STUFFLIB_PRINT_ERROR("corrupted zlib block, LEN too large");
    return 0;
  }

  memcpy(state->dst.data + state->dst_pos,
         state->src.data + src_byte_pos,
         block_len);
  src_byte_pos += block_len;

  state->dst_pos += block_len;
  state->src_bit = src_byte_pos * CHAR_BIT;

  return 1;
}

int stufflib_inflate_dynamic_block(_deflate_state state[static 1]) {
  const size_t num_lengths = 257 + _next_n_bits(state, 5);
  const size_t num_distances = 1 + _next_n_bits(state, 5);
  const size_t num_length_lengths = 4 + _next_n_bits(state, 4);

  static const size_t length_order[] =
      {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  static const size_t max_length_length = 18;

  size_t* length_lengths = calloc(max_length_length + 1, sizeof(size_t));
  if (!length_lengths) {
    STUFFLIB_PRINT_ERROR("failed allocating dynamic length_lengths");
    goto error;
  }
  for (size_t i = 0; i < num_length_lengths; ++i) {
    length_lengths[length_order[i]] = _next_n_bits(state, 3);
  }
  stufflib_huffman_tree length_tree = (stufflib_huffman_tree){0};
  if (!stufflib_huffman_init(&length_tree, max_length_length, length_lengths)) {
    free(length_lengths);
    goto error;
  }
  free(length_lengths);

  size_t* dynamic_code_lengths =
      calloc(num_lengths + num_distances, sizeof(size_t));
  for (size_t i = 0; i < num_lengths + num_distances;) {
    const size_t symbol = _decode_next_code(&length_tree, state);
    assert(symbol != SIZE_MAX);
    size_t code_len = 0;
    size_t num_repeats = 0;
    if (symbol < 16) {
      code_len = symbol;
      num_repeats = 1;
    } else if (symbol == 16) {
      assert(i > 0);
      code_len = dynamic_code_lengths[i - 1];
      num_repeats = 3 + _next_n_bits(state, 2);
    } else if (symbol == 17) {
      code_len = 0;
      num_repeats = 3 + _next_n_bits(state, 3);
    } else if (symbol == 18) {
      code_len = 0;
      num_repeats = 11 + _next_n_bits(state, 7);
    } else {
      STUFFLIB_PRINT_ERROR("unexpected symbol %zu in dynamic block", symbol);
      free(dynamic_code_lengths);
      goto error;
    }
    for (size_t r = 0; r < num_repeats; ++r) {
      dynamic_code_lengths[i + r] = code_len;
    }
    i += num_repeats;
  }
  stufflib_huffman_destroy(&length_tree);

  const size_t max_length = num_lengths - 1;
  const size_t max_distance = num_distances - 1;

  stufflib_huffman_tree literal_tree = (stufflib_huffman_tree){0};
  if (!stufflib_huffman_init(&literal_tree, max_length, dynamic_code_lengths)) {
    free(dynamic_code_lengths);
    goto error;
  }
  stufflib_huffman_tree distance_tree = (stufflib_huffman_tree){0};
  if (!stufflib_huffman_init(&distance_tree,
                             max_distance,
                             dynamic_code_lengths + num_lengths)) {
    free(dynamic_code_lengths);
    stufflib_huffman_destroy(&literal_tree);
    goto error;
  }
  free(dynamic_code_lengths);

  _inflate_block(&literal_tree, &distance_tree, state);
  stufflib_huffman_destroy(&literal_tree);
  stufflib_huffman_destroy(&distance_tree);
  return 1;

error:
  return 0;
}

int stufflib_inflate_fixed_block(_deflate_state state[static 1]) {
  stufflib_huffman_tree literal_tree = _make_fixed_literal_tree();
  stufflib_huffman_tree distance_tree = _make_fixed_distance_tree();
  _inflate_block(&literal_tree, &distance_tree, state);
  stufflib_huffman_destroy(&literal_tree);
  stufflib_huffman_destroy(&distance_tree);
  return 1;
}

size_t stufflib_inflate(stufflib_data dst, const stufflib_data src) {
  if (src.size < 3) {
    STUFFLIB_PRINT_ERROR("DEFLATE stream is too short");
    goto error;
  }
  if ((src.data[0] * 256 + src.data[1]) % 31) {
    STUFFLIB_PRINT_ERROR("DEFLATE stream is corrupted");
    goto error;
  }
  const int cmethod = src.data[0] & 0x0F;
  if (cmethod != 8) {
    STUFFLIB_PRINT_ERROR("unexpected compression method %d != 8", cmethod);
    goto error;
  }
  const int cinfo = (src.data[0] & 0xF0) >> 4;
  if (cinfo > 7) {
    STUFFLIB_PRINT_ERROR("too large compression info %d > 7", cinfo);
    goto error;
  }
  const int fdict = (src.data[1] & 0x20) >> 5;
  if (fdict) {
    STUFFLIB_PRINT_ERROR("dictionaries are not supported");
    goto error;
  }

  _deflate_state* state = &(_deflate_state){
      .len_codes = _make_length_codes(),
      .dist_codes = _make_distance_codes(),
      .dst = dst,
      .src = src,
      .dst_pos = 0,
      .src_bit = 2 * CHAR_BIT,
  };

  for (int is_final_block = 0; !is_final_block;) {
    is_final_block = _next_bit(state);
    enum block_type {
      no_compression = 0,
      fixed_huffman_trees = 1,
      dynamic_huffman_trees = 2,
    };
    const enum block_type type = _next_n_bits(state, 2);
    switch (type) {
      case no_compression: {
        if (!stufflib_inflate_uncompressed_block(state)) {
          STUFFLIB_PRINT_ERROR("failed inflating uncompressed block");
          goto error;
        }
      } break;
      case dynamic_huffman_trees: {
        if (!stufflib_inflate_dynamic_block(state)) {
          STUFFLIB_PRINT_ERROR("failed inflating dynamic block");
          goto error;
        }
      } break;
      case fixed_huffman_trees: {
        if (!stufflib_inflate_fixed_block(state)) {
          STUFFLIB_PRINT_ERROR("failed inflating fixed block");
          goto error;
        }
      } break;
      default: {
        STUFFLIB_PRINT_ERROR("invalid block type %d", type);
        goto error;
      } break;
    }
  }

  /* const size_t src_adler32 = _next_n_bits(state, 32); */
  /* const size_t dst_adler32 = */
  /*     stufflib_hash_adler32(state->dst.size, state->dst.data); */
  /* if (dst_adler32 != src_adler32) { */
  /*   STUFFLIB_PRINT_ERROR("output stream adler32 %zu not equal to expected
   * %zu", */
  /*                        dst_adler32, */
  /*                        src_adler32); */
  /*   goto error; */
  /* } */

  return state->dst_pos;

error:
  return 0;
}

#define STUFFLIB_DEFLATE_BLOCK_SIZE 8192

size_t stufflib_deflate_uncompressed(stufflib_data dst,
                                     const stufflib_data src) {
  const size_t block_size = STUFFLIB_DEFLATE_BLOCK_SIZE;
  size_t dst_pos = 0;
  size_t src_pos = 0;

  const int cmethod = 8;
  const int cinfo = 7;
  const int cmf = (cinfo << 4) | cmethod;
  dst.data[dst_pos++] = cmf;
  dst.data[dst_pos++] = 31 - (cmf * 256) % 31;

  for (int is_final_block = 0; !is_final_block;) {
    assert(src_pos < src.size);
    is_final_block = src_pos + block_size >= src.size;
    dst.data[dst_pos++] = is_final_block & 1;

    const size_t dist_to_end = src.size - src_pos;
    const uint16_t block_len =
        STUFFLIB_MIN(block_size, dist_to_end) & STUFFLIB_ONES(2);

    const unsigned char* block_len_bytes =
        stufflib_misc_encode_lil_endian(2, (unsigned char[2]){0}, block_len);
    const unsigned char* block_len_check_bytes =
        stufflib_misc_encode_lil_endian(2, (unsigned char[2]){0}, ~block_len);

    memcpy(dst.data + dst_pos, block_len_bytes, 2);
    dst_pos += 2;
    memcpy(dst.data + dst_pos, block_len_check_bytes, 2);
    dst_pos += 2;

    memcpy(dst.data + dst_pos, src.data + src_pos, block_len);
    dst_pos += block_len;
    src_pos += block_len;
  }

  const unsigned char* adler32 = stufflib_misc_encode_big_endian(
      4,
      (unsigned char[4]){0},
      stufflib_hash_adler32(src.size, src.data));
  memcpy(dst.data + dst_pos, adler32, 4);
  dst_pos += 4;

  return dst_pos;
}

#endif  // _STUFFLIB_DEFLATE_H_INCLUDED
