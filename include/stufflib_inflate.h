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

void stufflib_inflate_tree_destroy(stufflib_huffman_tree tree[static 1]) {
  if (tree) {
    for (size_t code_len = 1; code_len <= tree->max_code_len; ++code_len) {
      free(tree->symbols[code_len - 1]);
    }
    free(tree->max_codes);
    free(tree->symbols);
  }
  stufflib_inflate_huffman_tree(tree, 0, 0);
}

static size_t _tree_get(const stufflib_huffman_tree tree[static 1],
                        const size_t code,
                        const size_t code_len) {
  return tree->symbols[code_len - 1][code] - 1;
}

static int _tree_contains(const stufflib_huffman_tree* tree,
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

static stufflib_huffman_tree _make_fixed_dist_codes() {
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

static int _next_bit(const unsigned char data[const static 1],
                     size_t* bit_pos) {
  const size_t bit = (*bit_pos)++;
  return (data[bit / CHAR_BIT] >> (bit % CHAR_BIT)) & 1;
}

static size_t _next_n_bits(const unsigned char data[const static 1],
                           size_t* pos,
                           const size_t count) {
  size_t res = 0;
  for (size_t i = 0; i < count; ++i) {
    res |= _next_bit(data, pos) << i;
  }
  return res;
}

size_t _decode_next_code(const stufflib_huffman_tree codes[const static 1],
                         const unsigned char src[const static 1],
                         size_t src_pos[static 1]) {
  size_t code = 0;
  for (size_t code_len = 1; code_len <= codes->max_code_len; ++code_len) {
    code = (code << 1) | _next_bit(src, src_pos);
    if (_tree_contains(codes, code, code_len)) {
      return _tree_get(codes, code, code_len);
    }
  }
  return SIZE_MAX;
}

size_t stufflib_inflate_uncompressed_block(const size_t dst_len,
                                           unsigned char dst[dst_len],
                                           const size_t src_len,
                                           const unsigned char src[src_len],
                                           size_t src_pos[static 1]) {
  size_t src_byte_pos = *src_pos / CHAR_BIT + 1;
  const size_t len = stufflib_misc_parse_lil_endian_u16(src + src_byte_pos);
  src_byte_pos += 2;
  const size_t nlen = stufflib_misc_parse_lil_endian_u16(src + src_byte_pos);
  src_byte_pos += 2;
  if ((~len & 0xffff) != nlen) {
    fprintf(stderr, "corrupted zlib block, ~LEN != NLEN\n");
    return 0;
  }
  memcpy(dst, (void*)(src + src_byte_pos), len);
  *src_pos += (src_byte_pos + len) * CHAR_BIT;
  return len;
}

size_t stufflib_inflate_dynamic_huffman_block(const size_t dst_len,
                                              unsigned char dst[dst_len],
                                              const size_t src_len,
                                              const unsigned char src[src_len],
                                              size_t src_pos[static 1]) {
  const size_t num_lengths = 257 + _next_n_bits(src, src_pos, 5);
  const size_t num_distances = 1 + _next_n_bits(src, src_pos, 5);
  const size_t num_len_lengths = 4 + _next_n_bits(src, src_pos, 4);
  printf("dynamic, code counts %zu %zu %zu\n",
         num_lengths,
         num_distances,
         num_len_lengths);
  static const size_t length_order[] =
      {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

  const size_t max_length = 18;
  size_t* len_lengths = calloc(max_length + 1, sizeof(size_t));
  if (!len_lengths) {
    fprintf(stderr, "failed allocating dynamic len_lengths\n");
    goto error;
  }
  for (size_t i = 0; i < num_len_lengths; ++i) {
    len_lengths[length_order[i]] = _next_n_bits(src, src_pos, 3);
  }
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(length_order); ++i) {
    if (len_lengths[length_order[i]]) {
      printf("len %zu %zu\n", length_order[i], len_lengths[length_order[i]]);
    }
  }
  stufflib_huffman_tree tree = (stufflib_huffman_tree){0};
  if (!stufflib_inflate_huffman_tree(&tree, max_length, len_lengths)) {
    free(len_lengths);
    goto error;
  }
  free(len_lengths);

  for (size_t code_len = 1; code_len <= tree.max_code_len; ++code_len) {
    for (size_t i = 0; i < tree.max_codes[code_len - 1]; ++i) {
      printf("%zu %zu %zu\n", code_len, i, tree.symbols[code_len - 1][i]);
    }
  }

  for (size_t i = 0; i < num_lengths; ++i) {
    const size_t symbol = _decode_next_code(&tree, src, src_pos);
    assert(symbol != SIZE_MAX);
    printf("%zu: %zu\n", i, symbol);
    if (symbol <= 15) {

    } else if (symbol == 16) {
    } else if (symbol == 17) {
    } else if (symbol == 18) {
    } else {
      assert(0);
    }
  }

  stufflib_inflate_tree_destroy(&tree);
  return 0;

error:
  return 0;
}

size_t stufflib_inflate_fixed_huffman_block(
    const stufflib_huffman_tree literal_codes[const static 1],
    const stufflib_huffman_tree dist_codes[const static 1],
    const size_t dst_len,
    unsigned char dst[dst_len],
    const size_t src_len,
    const unsigned char src[src_len],
    size_t src_pos[static 1]) {
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
    for (size_t i = 1; i < STUFFLIB_ARRAY_LEN(distances); ++i) {
      const int extra_bits = STUFFLIB_MAX(1, (i - 1) / 2) - 1;
      distances_extra[i - 1] = extra_bits;
      distances[i] = distances[i - 1] + (1 << extra_bits);
    }
  }

  size_t dst_pos = 0;
  while (1) {
    const size_t symbol = _decode_next_code(literal_codes, src, src_pos);
    assert(symbol < 286);
    if (symbol < 256) {
      dst[dst_pos++] = symbol & 0xff;
      continue;
    }
    if (symbol == 256) {
      break;
    }
    const size_t len_symbol = symbol - 257;
    const size_t num_extra_len_bits = lengths_extra[len_symbol];
    const size_t extra_len = _next_n_bits(src, src_pos, num_extra_len_bits);
    const size_t len = lengths[len_symbol] + extra_len;

    const size_t dist_symbol = _decode_next_code(dist_codes, src, src_pos);
    const size_t num_extra_dist_bits = distances_extra[dist_symbol];
    const size_t extra_dist = _next_n_bits(src, src_pos, num_extra_dist_bits);
    const size_t dist = distances[dist_symbol] + extra_dist;

    const size_t begin = dst_pos - dist;
    for (size_t pos = begin; pos < begin + len; ++pos) {
      dst[dst_pos++] = dst[pos];
    }
  }
  return dst_pos;
}

stufflib_data stufflib_inflate(const size_t n, const unsigned char data[n]) {
  unsigned char* output = 0;
  size_t output_size = 0;
  stufflib_huffman_tree fixed_literal_codes = _make_fixed_literal_codes();
  stufflib_huffman_tree fixed_dist_codes = _make_fixed_dist_codes();

  if (n < 3) {
    fprintf(stderr, "DEFLATE stream is too short\n");
    goto error;
  }
  if ((data[0] * 256 + data[1]) % 31) {
    fprintf(stderr, "DEFLATE stream is corrupted\n");
    goto error;
  }
  const int cmethod = data[0] & 0x0F;
  if (cmethod != 8) {
    fprintf(stderr, "unexpected compression method %d != 8\n", cmethod);
    goto error;
  }
  const int cinfo = (data[0] & 0xF0) >> 4;
  if (cinfo > 7) {
    fprintf(stderr, "too large compression info %d > 7\n", cinfo);
    goto error;
  }
  const int fdict = (data[1] & 0x20) >> 5;
  if (fdict) {
    fprintf(stderr, "dictionaries are not supported\n");
    goto error;
  }

#if 0
  for (size_t code_len = 1; code_len <= literal_codes.max_code_len;
       ++code_len) {
    if (literal_codes.max_codes[code_len - 1]) {
      printf("lc %3zu code_len max %zu\n",
             code_len,
             literal_codes.max_codes[code_len - 1]);
      for (size_t code = 0; code <= literal_codes.max_codes[code_len - 1];
           ++code) {
        if (_tree_contains(&literal_codes, code, code_len)) {
          printf("  %3zu -> %3zu\n",
                 code,
                 _tree_get(&literal_codes, code, code_len));
        }
      }
    }
  }
  for (size_t code_len = 1; code_len <= dist_codes.max_code_len; ++code_len) {
    if (dist_codes.max_codes[code_len - 1]) {
      printf("lc %3zu code_len max %zu\n",
             code_len,
             dist_codes.max_codes[code_len - 1]);
      for (size_t code = 0; code <= dist_codes.max_codes[code_len - 1];
           ++code) {
        if (_tree_contains(&dist_codes, code, code_len)) {
          printf("  %3zu -> %3zu\n",
                 code,
                 _tree_get(&dist_codes, code, code_len));
        }
      }
    }
  }
#endif

  const size_t window_size = 1 << (cinfo + 8);
  size_t src_pos[] = {2 * CHAR_BIT};

  for (int is_final_block = 0; !is_final_block;) {
    is_final_block = _next_bit(data, src_pos);

    {
      unsigned char* tmp = realloc(output, output_size + window_size);
      if (!tmp) {
        goto error;
      }
      output = tmp;
    }

    enum block_type {
      no_compression = 0,
      fixed_huffman_codes = 1,
      dynamic_huffman_codes = 2,
    };

    const int btype = _next_n_bits(data, src_pos, 2);

    switch (btype) {
      case no_compression: {
        output_size += stufflib_inflate_uncompressed_block(window_size,
                                                           output + output_size,
                                                           n,
                                                           data,
                                                           src_pos);
      } break;
      case dynamic_huffman_codes: {
        // TODO
        output_size +=
            stufflib_inflate_dynamic_huffman_block(window_size,
                                                   output + output_size,
                                                   n,
                                                   data,
                                                   src_pos);
        goto error;
      } break;
      case fixed_huffman_codes: {
        output_size +=
            stufflib_inflate_fixed_huffman_block(&fixed_literal_codes,
                                                 &fixed_dist_codes,
                                                 window_size,
                                                 output + output_size,
                                                 n,
                                                 data,
                                                 src_pos);
      } break;
      default: {
        fprintf(stderr, "invalid block type %d\n", btype);
        goto error;
      } break;
    }

    if (*src_pos == 0) {
      goto error;
    }

    {
      unsigned char* tmp = realloc(output, output_size);
      if (!tmp) {
        goto error;
      }
      output = tmp;
    }
  }

  goto done;

error:
  output_size = 0;
  free(output);
  output = 0;
done:
  stufflib_inflate_tree_destroy(&fixed_literal_codes);
  stufflib_inflate_tree_destroy(&fixed_dist_codes);
  return (stufflib_data){.size = output_size, .data = output};
}

#endif  // _STUFFLIB_INFLATE_H_INCLUDED
