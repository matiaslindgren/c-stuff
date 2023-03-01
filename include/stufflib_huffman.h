#ifndef _SL_HUFFMAN_H_INCLUDED
#define _SL_HUFFMAN_H_INCLUDED
#include <stdlib.h>

#include "stufflib_macros.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"

typedef struct sl_huffman_tree sl_huffman_tree;
struct sl_huffman_tree {
  size_t max_code_len;
  // TODO add min_codes so we use (code - min_code) as index for symbols
  size_t* max_codes;
  size_t** symbols;
};

bool sl_huffman_init(sl_huffman_tree tree[static 1],
                     const size_t max_symbol,
                     const size_t code_lengths[const max_symbol + 1]) {
  if (!max_symbol) {
    *tree = (sl_huffman_tree){0};
    return true;
  }

  size_t* code_length_count = nullptr;
  size_t* next_code = nullptr;
  size_t* codes = nullptr;
  size_t* max_codes = nullptr;
  size_t** symbols = nullptr;

  const size_t max_code_len = sl_misc_vmax_size_t(max_symbol + 1, code_lengths);

  code_length_count = sl_alloc(max_code_len + 1, sizeof(size_t));
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    ++code_length_count[code_lengths[symbol]];
  }
  code_length_count[0] = 0;

  next_code = sl_alloc(max_code_len + 1, sizeof(size_t));
  {
    size_t code = 0;
    for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
      code = (code + code_length_count[code_len - 1]) << 1;
      next_code[code_len] = code;
    }
  }

  codes = sl_alloc(max_symbol + 1, sizeof(size_t));
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      codes[symbol] = next_code[code_len];
      ++next_code[code_len];
    }
  }

  max_codes = sl_alloc(max_code_len, sizeof(size_t));
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      max_codes[code_len - 1] = SL_MAX(max_codes[code_len - 1], code);
    }
  }

  symbols = sl_alloc(max_code_len, sizeof(size_t*));
  for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
    const size_t max_code = max_codes[code_len - 1];
    symbols[code_len - 1] = sl_alloc(max_code + 1, sizeof(size_t));
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      symbols[code_len - 1][code] = symbol + 1;
    }
  }

  *tree = (sl_huffman_tree){.max_code_len = max_code_len,
                            .max_codes = max_codes,
                            .symbols = symbols};
  sl_free(codes);
  sl_free(next_code);
  sl_free(code_length_count);
  return tree->max_code_len > 0;
}

void sl_huffman_destroy(sl_huffman_tree tree[const static 1]) {
  if (tree) {
    for (size_t code_len = 1; code_len <= tree->max_code_len; ++code_len) {
      sl_free(tree->symbols[code_len - 1]);
    }
    sl_free(tree->max_codes);
    sl_free(tree->symbols);
  }
  sl_huffman_init(tree, 0, 0);
}

bool sl_huffman_contains(const sl_huffman_tree tree[const static 1],
                         const size_t code,
                         const size_t code_len) {
  return code_len > 0 && code <= tree->max_codes[code_len - 1] &&
         tree->symbols[code_len - 1][code];
}

size_t sl_huffman_get(const sl_huffman_tree tree[const static 1],
                      const size_t code,
                      const size_t code_len) {
  return tree->symbols[code_len - 1][code] - 1;
}

#endif  // _SL_HUFFMAN_H_INCLUDED
