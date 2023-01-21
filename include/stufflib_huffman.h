#ifndef _STUFFLIB_HUFFMAN_H_INCLUDED
#define _STUFFLIB_HUFFMAN_H_INCLUDED
#include <stdlib.h>

#include "stufflib_macros.h"
#include "stufflib_misc.h"

typedef struct stufflib_huffman_tree stufflib_huffman_tree;
struct stufflib_huffman_tree {
  size_t max_code_len;
  // TODO add min_codes so we use (code - min_code) as index for symbols
  size_t* max_codes;
  size_t** symbols;
};

int stufflib_huffman_init(stufflib_huffman_tree tree[static 1],
                          const size_t max_symbol,
                          const size_t code_lengths[const max_symbol + 1]) {
  if (!max_symbol) {
    *tree = (stufflib_huffman_tree){0};
    return 1;
  }

  size_t* code_length_count = 0;
  size_t* next_code = 0;
  size_t* codes = 0;
  size_t* max_codes = 0;
  size_t** symbols = 0;

  const size_t max_code_len =
      stufflib_misc_vmax_size_t(max_symbol + 1, code_lengths);

  code_length_count = calloc(max_code_len + 1, sizeof(size_t));
  if (!code_length_count) {
    STUFFLIB_PRINT_ERROR("failed allocating code_length_count");
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    ++code_length_count[code_lengths[symbol]];
  }
  code_length_count[0] = 0;

  next_code = calloc(max_code_len + 1, sizeof(size_t));
  if (!next_code) {
    STUFFLIB_PRINT_ERROR("failed allocating next_code");
    goto error;
  }
  {
    size_t code = 0;
    for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
      code = (code + code_length_count[code_len - 1]) << 1;
      next_code[code_len] = code;
    }
  }

  codes = calloc(max_symbol + 1, sizeof(size_t));
  if (!codes) {
    STUFFLIB_PRINT_ERROR("failed allocating codes");
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      codes[symbol] = next_code[code_len];
      ++next_code[code_len];
    }
  }

  max_codes = calloc(max_code_len, sizeof(size_t));
  if (!max_codes) {
    STUFFLIB_PRINT_ERROR("failed allocating max_codes");
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      max_codes[code_len - 1] = STUFFLIB_MAX(max_codes[code_len - 1], code);
    }
  }

  symbols = calloc(max_code_len, sizeof(size_t*));
  if (!symbols) {
    STUFFLIB_PRINT_ERROR("failed allocating symbols");
    goto error;
  }
  for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
    const size_t max_code = max_codes[code_len - 1];
    if (!(symbols[code_len - 1] = calloc(max_code + 1, sizeof(size_t)))) {
      STUFFLIB_PRINT_ERROR("failed allocating symbols for code_len %zu",
                           code_len);
      goto error;
    }
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      symbols[code_len - 1][code] = symbol + 1;
    }
  }

  *tree = (stufflib_huffman_tree){.max_code_len = max_code_len,
                                  .max_codes = max_codes,
                                  .symbols = symbols};
  goto done;

error:
  if (symbols) {
    for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
      free(symbols[code_len - 1]);
    }
    free(symbols);
  }
  free(max_codes);
  *tree = (stufflib_huffman_tree){0};
done:
  free(codes);
  free(next_code);
  free(code_length_count);
  return tree->max_code_len > 0;
}

void stufflib_huffman_destroy(stufflib_huffman_tree tree[const static 1]) {
  if (tree) {
    for (size_t code_len = 1; code_len <= tree->max_code_len; ++code_len) {
      free(tree->symbols[code_len - 1]);
    }
    free(tree->max_codes);
    free(tree->symbols);
  }
  stufflib_huffman_init(tree, 0, 0);
}

int stufflib_huffman_contains(const stufflib_huffman_tree tree[const static 1],
                              const size_t code,
                              const size_t code_len) {
  return code_len > 0 && code <= tree->max_codes[code_len - 1] &&
         tree->symbols[code_len - 1][code];
}

size_t stufflib_huffman_get(const stufflib_huffman_tree tree[const static 1],
                            const size_t code,
                            const size_t code_len) {
  return tree->symbols[code_len - 1][code] - 1;
}

#endif  // _STUFFLIB_HUFFMAN_H_INCLUDED
