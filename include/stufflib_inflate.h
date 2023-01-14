#ifndef _STUFFLIB_INFLATE_H_INCLUDED
#define _STUFFLIB_INFLATE_H_INCLUDED
// INFLATE: DEFLATE decoding (no compression implemented)
// Implementation based on
// 1. RFC 1950 (Deutsch and Gailly, May 1996),
//    https://datatracker.ietf.org/doc/html/rfc1950 (accessed 2023-01-05)
// 2. RFC 1951 (Deutsch, May 1996),
//    https://datatracker.ietf.org/doc/html/rfc1951 (accessed 2023-01-05)
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_misc.h"

typedef struct stufflib_huffman_tree stufflib_huffman_tree;
struct stufflib_huffman_tree {
  size_t max_code_len;
  // TODO add min_codes so we use (code - min_code) as index for symbols
  size_t* max_codes;
  size_t** symbols;
};

typedef struct _stufflib_inflate_state _stufflib_inflate_state;
struct _stufflib_inflate_state {
  stufflib_data dst;
  const stufflib_data src;
  size_t dst_pos;
  size_t src_bit;
};

stufflib_huffman_tree* stufflib_inflate_huffman_tree(
    stufflib_huffman_tree tree[static 1],
    const size_t max_symbol,
    const size_t code_lengths[max_symbol + 1]) {
  if (!max_symbol) {
    goto return_empty;
  }

  const size_t max_code_len =
      stufflib_misc_vmax_size_t(max_symbol + 1, code_lengths);

  size_t* code_length_count = calloc(max_code_len + 1, sizeof(size_t));
  if (!code_length_count) {
    fprintf(stderr, "failed allocating code_length_count\n");
    goto return_empty;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    ++code_length_count[code_lengths[symbol]];
  }
  code_length_count[0] = 0;

  size_t* next_code = calloc(max_code_len + 1, sizeof(size_t));
  if (!next_code) {
    free(code_length_count);
    fprintf(stderr, "failed allocating next_code\n");
    goto return_empty;
  }
  {
    size_t code = 0;
    for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
      code = (code + code_length_count[code_len - 1]) << 1;
      next_code[code_len] = code;
    }
  }
  free(code_length_count);

  size_t* codes = calloc(max_symbol + 1, sizeof(size_t));
  if (!codes) {
    free(next_code);
    fprintf(stderr, "failed allocating codes\n");
    goto return_empty;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      codes[symbol] = next_code[code_len];
      ++next_code[code_len];
    }
  }
  free(next_code);

  size_t* max_codes = calloc(max_code_len, sizeof(size_t));
  if (!max_codes) {
    free(codes);
    fprintf(stderr, "failed allocating max_codes\n");
    goto return_empty;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      max_codes[code_len - 1] = STUFFLIB_MAX(max_codes[code_len - 1], code);
    }
  }

  size_t** symbols = calloc(max_code_len, sizeof(size_t*));
  if (!symbols) {
    free(max_codes);
    free(codes);
    fprintf(stderr, "failed allocating symbols\n");
    goto return_empty;
  }
  for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
    const size_t max_code = max_codes[code_len - 1];
    if (!(symbols[code_len - 1] = calloc(max_code + 1, sizeof(size_t)))) {
      free(max_codes);
      // TODO proper free for every pointer in symbols up to code_len
      free(symbols);
      free(codes);
      fprintf(stderr, "failed allocating symbols for code_len %zu\n", code_len);
      goto return_empty;
    }
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      symbols[code_len - 1][code] = symbol + 1;
    }
  }
  free(codes);

  *tree = (stufflib_huffman_tree){.max_code_len = max_code_len,
                                  .max_codes = max_codes,
                                  .symbols = symbols};
  return tree;

return_empty:
  *tree = (stufflib_huffman_tree){0};
  return tree;
}

void stufflib_inflate_tree_destroy(stufflib_huffman_tree tree[const static 1]) {
  if (tree) {
    for (size_t code_len = 1; code_len <= tree->max_code_len; ++code_len) {
      free(tree->symbols[code_len - 1]);
    }
    free(tree->max_codes);
    free(tree->symbols);
  }
  stufflib_inflate_huffman_tree(tree, 0, 0);
}

static size_t _tree_get(const stufflib_huffman_tree tree[const static 1],
                        const size_t code,
                        const size_t code_len) {
  return tree->symbols[code_len - 1][code] - 1;
}

static int _tree_contains(const stufflib_huffman_tree tree[const static 1],
                          const size_t code,
                          const size_t code_len) {
  return code_len > 0 && code <= tree->max_codes[code_len - 1] &&
         tree->symbols[code_len - 1][code];
}

static stufflib_huffman_tree _make_fixed_literal_codes() {
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
  if (!stufflib_inflate_huffman_tree(&tree, max_literal, code_lengths)) {
    free(code_lengths);
    goto error;
  }
  free(code_lengths);
  return tree;

error:
  fprintf(stderr, "failed building fixed literal huffman codes for INFLATE\n");
  return (stufflib_huffman_tree){0};
}

static stufflib_huffman_tree _make_fixed_distance_codes() {
  const size_t max_dist = 31;
  size_t* code_lengths = calloc(max_dist + 1, sizeof(size_t));
  if (!code_lengths) {
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_dist; ++symbol) {
    code_lengths[symbol] = 5;
  }
  stufflib_huffman_tree tree = (stufflib_huffman_tree){0};
  if (!stufflib_inflate_huffman_tree(&tree, max_dist, code_lengths)) {
    free(code_lengths);
    goto error;
  }
  free(code_lengths);
  return tree;

error:
  fprintf(stderr, "failed building fixed distance huffman codes for INFLATE\n");
  return (stufflib_huffman_tree){0};
}

static size_t _next_bit(_stufflib_inflate_state state[static 1]) {
  const size_t src_pos = state->src_bit / CHAR_BIT;
  const size_t src_bit = state->src_bit % CHAR_BIT;
  assert(src_pos < state->src.size);
  ++(state->src_bit);
  return (state->src.data[src_pos] >> src_bit) & 1;
}

static size_t _next_n_bits(_stufflib_inflate_state state[static 1],
                           const size_t count) {
  size_t res = 0;
  for (size_t i = 0; i < count; ++i) {
    res |= _next_bit(state) << i;
  }
  return res;
}

static size_t _decode_next_code(
    const stufflib_huffman_tree codes[const static 1],
    _stufflib_inflate_state state[static 1]) {
  size_t code = 0;
  for (size_t code_len = 1; code_len <= codes->max_code_len; ++code_len) {
    code = (code << 1) | _next_bit(state);
    if (_tree_contains(codes, code, code_len)) {
      return _tree_get(codes, code, code_len);
    }
  }
  return SIZE_MAX;
}

static void _inflate_block(
    const stufflib_huffman_tree literal_codes[const static 1],
    const stufflib_huffman_tree distance_codes[const static 1],
    _stufflib_inflate_state state[static 1]) {
  static size_t lengths[29] = {0};
  static int lengths_extra[29] = {0};
  static size_t distances[30] = {0};
  static int distances_extra[30] = {0};

  static int first_call = 1;
  if (first_call) {
    first_call = 0;

    lengths[0] = 3;
    for (size_t i = 1; i < STUFFLIB_ARRAY_LEN(lengths); ++i) {
      const int extra_bits = (STUFFLIB_MAX(1, (i - 1) / 4) - 1) % 6;
      lengths_extra[i - 1] = extra_bits;
      lengths[i] = lengths[i - 1] + (1 << extra_bits);
    }
    lengths[28] = 258;

    distances[0] = 1;
    for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(distances); ++i) {
      distances_extra[i] = STUFFLIB_MAX(1, i / 2) - 1;
      if (i) {
        distances[i] = distances[i - 1] + (1 << distances_extra[i - 1]);
      }
    }
  }

  const size_t end_of_block = 256;

  while (1) {
    const size_t symbol = _decode_next_code(literal_codes, state);
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
    const size_t extra_len = _next_n_bits(state, lengths_extra[len_symbol]);
    const size_t length = lengths[len_symbol] + extra_len;

    const size_t dist_symbol = _decode_next_code(distance_codes, state);
    const size_t extra_dist = _next_n_bits(state, distances_extra[dist_symbol]);
    const size_t back_distance = distances[dist_symbol] + extra_dist;

    const size_t copy_begin = state->dst_pos - back_distance;
    for (size_t i = copy_begin; i < copy_begin + length; ++i) {
      assert(state->dst_pos < state->dst.size);
      state->dst.data[state->dst_pos++] = state->dst.data[i];
    }
  }
}

int stufflib_inflate_uncompressed_block(
    _stufflib_inflate_state state[static 1]) {
  size_t src_byte_pos = state->src_bit / CHAR_BIT + 1;
  const size_t block_length =
      stufflib_misc_parse_lil_endian_u16(state->src.data + src_byte_pos);
  src_byte_pos += 2;
  const size_t block_length_check =
      stufflib_misc_parse_lil_endian_u16(state->src.data + src_byte_pos);
  src_byte_pos += 2;
  if ((~block_length & 0xffff) != block_length_check) {
    fprintf(stderr, "corrupted zlib block, ~LEN != NLEN\n");
    return 1;
  }
  memcpy(state->dst.data + state->dst_pos,
         (void*)(state->src.data + src_byte_pos),
         block_length);
  state->dst_pos += block_length;
  state->src_bit += (src_byte_pos + block_length) * CHAR_BIT;
  return 0;
}

int stufflib_inflate_dynamic_block(_stufflib_inflate_state state[static 1]) {
  const size_t num_lengths = 257 + _next_n_bits(state, 5);
  const size_t num_distances = 1 + _next_n_bits(state, 5);
  const size_t num_length_lengths = 4 + _next_n_bits(state, 4);

  static const size_t length_order[] =
      {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  static const size_t max_length_length = 18;

  size_t* length_lengths = calloc(max_length_length + 1, sizeof(size_t));
  if (!length_lengths) {
    fprintf(stderr, "failed allocating dynamic length_lengths\n");
    goto error;
  }
  for (size_t i = 0; i < num_length_lengths; ++i) {
    length_lengths[length_order[i]] = _next_n_bits(state, 3);
  }
  stufflib_huffman_tree length_codes = (stufflib_huffman_tree){0};
  if (!stufflib_inflate_huffman_tree(&length_codes,
                                     max_length_length,
                                     length_lengths)) {
    free(length_lengths);
    goto error;
  }
  free(length_lengths);

  size_t* dynamic_code_lengths =
      calloc(num_lengths + num_distances, sizeof(size_t));
  for (size_t i = 0; i < num_lengths + num_distances;) {
    const size_t symbol = _decode_next_code(&length_codes, state);
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
      fprintf(stderr, "unexpected symbol %zu in dynamic block\n", symbol);
      free(dynamic_code_lengths);
      goto error;
    }
    for (size_t r = 0; r < num_repeats; ++r) {
      dynamic_code_lengths[i + r] = code_len;
    }
    i += num_repeats;
  }
  stufflib_inflate_tree_destroy(&length_codes);

  const size_t max_length = num_lengths - 1;
  const size_t max_distance = num_distances - 1;

  stufflib_huffman_tree literal_codes = (stufflib_huffman_tree){0};
  if (!stufflib_inflate_huffman_tree(&literal_codes,
                                     max_length,
                                     dynamic_code_lengths)) {
    free(dynamic_code_lengths);
    goto error;
  }
  stufflib_huffman_tree distance_codes = (stufflib_huffman_tree){0};
  if (!stufflib_inflate_huffman_tree(&distance_codes,
                                     max_distance,
                                     dynamic_code_lengths + num_lengths)) {
    free(dynamic_code_lengths);
    stufflib_inflate_tree_destroy(&literal_codes);
    goto error;
  }
  free(dynamic_code_lengths);

  _inflate_block(&literal_codes, &distance_codes, state);
  stufflib_inflate_tree_destroy(&literal_codes);
  stufflib_inflate_tree_destroy(&distance_codes);
  return 0;

error:
  return 1;
}

int stufflib_inflate_fixed_block(_stufflib_inflate_state state[static 1]) {
  stufflib_huffman_tree literal_codes = _make_fixed_literal_codes();
  stufflib_huffman_tree distance_codes = _make_fixed_distance_codes();
  _inflate_block(&literal_codes, &distance_codes, state);
  stufflib_inflate_tree_destroy(&literal_codes);
  stufflib_inflate_tree_destroy(&distance_codes);
  return 0;
}

size_t stufflib_inflate(stufflib_data dst, const stufflib_data src) {
  if (src.size < 3) {
    fprintf(stderr, "DEFLATE stream is too short\n");
    goto error;
  }
  if ((src.data[0] * 256 + src.data[1]) % 31) {
    fprintf(stderr, "DEFLATE stream is corrupted\n");
    goto error;
  }
  const int cmethod = src.data[0] & 0x0F;
  if (cmethod != 8) {
    fprintf(stderr, "unexpected compression method %d != 8\n", cmethod);
    goto error;
  }
  const int cinfo = (src.data[0] & 0xF0) >> 4;
  if (cinfo > 7) {
    fprintf(stderr, "too large compression info %d > 7\n", cinfo);
    goto error;
  }
  const int fdict = (src.data[1] & 0x20) >> 5;
  if (fdict) {
    fprintf(stderr, "dictionaries are not supported\n");
    goto error;
  }

  _stufflib_inflate_state* state = &(_stufflib_inflate_state){
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
        if (stufflib_inflate_uncompressed_block(state)) {
          fprintf(stderr, "failed inflating uncompressed block\n");
          goto error;
        }
      } break;
      case dynamic_huffman_trees: {
        if (stufflib_inflate_dynamic_block(state)) {
          fprintf(stderr, "failed inflating dynamic block\n");
          goto error;
        }
      } break;
      case fixed_huffman_trees: {
        if (stufflib_inflate_fixed_block(state)) {
          fprintf(stderr, "failed inflating fixed block\n");
          goto error;
        }
      } break;
      default: {
        fprintf(stderr, "invalid block type %d\n", type);
        goto error;
      } break;
    }
  }

  /* const size_t src_adler32 = _next_n_bits(state, 32); */
  /* const size_t dst_adler32 = */
  /*     stufflib_misc_adler32(state->dst.size, state->dst.data); */
  /* if (dst_adler32 != src_adler32) { */
  /*   fprintf(stderr, */
  /*           "output stream adler32 %zu not equal to expected %zu\n", */
  /*           dst_adler32, */
  /*           src_adler32); */
  /*   goto error; */
  /* } */

  return state->dst_pos;

error:
  return 0;
}

#endif  // _STUFFLIB_INFLATE_H_INCLUDED
