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

struct sl_deflate_length_codes sl_deflate_make_length_codes(void);

struct sl_deflate_distance_codes sl_deflate_make_distance_codes(void);

struct sl_huffman_tree sl_deflate_make_fixed_literal_tree(void);

struct sl_huffman_tree sl_deflate_make_fixed_distance_tree(void);

size_t sl_deflate_next_bit(struct sl_deflate_deflate_state state[static 1]);

size_t sl_deflate_next_n_bits(struct sl_deflate_deflate_state state[static 1],
                              const int count);

size_t sl_deflate_decode_next_code(
    const struct sl_huffman_tree codes[const static 1],
    struct sl_deflate_deflate_state state[static 1]);

void sl_deflate_inflate_block(
    const struct sl_huffman_tree literal_tree[const static 1],
    const struct sl_huffman_tree distance_tree[const static 1],
    struct sl_deflate_deflate_state state[static 1]);

bool sl_inflate_uncompressed_block(
    struct sl_deflate_deflate_state state[static 1]);

bool sl_inflate_dynamic_block(struct sl_deflate_deflate_state state[static 1]);

bool sl_inflate_fixed_block(struct sl_deflate_deflate_state state[static 1]);

size_t sl_inflate(struct sl_span dst, const struct sl_span src);

size_t sl_deflate_uncompressed(struct sl_span dst, const struct sl_span src);

#endif  // SL_DEFLATE_H_INCLUDED
