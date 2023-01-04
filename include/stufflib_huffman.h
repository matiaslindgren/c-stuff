#ifndef _STUFFLIB_HUFFMAN_H_INCLUDED
#define _STUFFLIB_HUFFMAN_H_INCLUDED
#include "stufflib_misc.h"

typedef struct stufflib_huffman_tree stufflib_huffman_tree;
struct stufflib_huffman_tree {
  size_t length;
  size_t code;
};

stufflib_huffman_tree* stufflib_huffman_tree_init(
    const size_t n,
    const size_t bit_lengths[static n]) {
  const size_t max_bits = stufflib_misc_vmax_size_t(n, bit_lengths);

  size_t* bit_length_count = calloc(max_bits + 1, sizeof(size_t));
  if (!bit_length_count) {
    return 0;
  }
  for (size_t i = 0; i < n; ++i) {
    ++bit_length_count[bit_lengths[i]];
  }

  size_t* next_code = calloc(max_bits + 1, sizeof(size_t));
  if (!next_code) {
    free(bit_length_count);
    return 0;
  }

  {
    size_t code = 0;
    for (size_t bits = 1; bits <= max_bits; ++bits) {
      code = (code + bit_length_count[bits - 1]) << 1;
      next_code[bits] = code;
    }
  }

  stufflib_huffman_tree* tree = calloc(n, sizeof(stufflib_huffman_tree));
  if (tree) {
    for (size_t i = 0; i < n; ++i) {
      const size_t len = bit_lengths[i];
      if (!len) {
        continue;
      }
      tree[i] = (stufflib_huffman_tree){
          .length = len,
          .code = next_code[len],
      };
      ++next_code[len];
    }
  }
  free(next_code);
  free(bit_length_count);
  return tree;
}

#endif  // _STUFFLIB_HUFFMAN_H_INCLUDED
