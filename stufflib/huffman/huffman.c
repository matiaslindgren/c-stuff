#include <stdlib.h>
#include <stufflib/huffman/huffman.h>
#include <stufflib/macros/macros.h>

void sl_huffman_init(
    struct sl_context ctx[static 1],
    struct sl_huffman_tree tree[static 1],
    const size_t max_symbol,
    const size_t code_lengths[const max_symbol + 1]
) {
  if (!max_symbol) { // TODO useless branch
    *tree = (struct sl_huffman_tree){0};
    return;
  }

  size_t* code_length_count = nullptr;
  size_t* next_code         = nullptr;
  size_t* codes             = nullptr;
  size_t* max_codes         = nullptr;
  size_t** symbols          = nullptr;

  const size_t max_code_len = sl_misc_vmax_size_t(max_symbol + 1, code_lengths);

  code_length_count = sl_alloc(ctx, max_code_len + 1, sizeof(size_t));
  if (!code_length_count) {
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    ++code_length_count[code_lengths[symbol]];
  }
  code_length_count[0] = 0;

  next_code = sl_alloc(ctx, max_code_len + 1, sizeof(size_t));
  if (!next_code) {
    goto error;
  }
  {
    size_t code = 0;
    for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
      code                = (code + code_length_count[code_len - 1]) << 1;
      next_code[code_len] = code;
    }
  }

  codes = sl_alloc(ctx, max_symbol + 1, sizeof(size_t));
  if (!codes) {
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      codes[symbol] = next_code[code_len];
      ++next_code[code_len];
    }
  }

  max_codes = sl_alloc(ctx, max_code_len, sizeof(size_t));
  if (!max_codes) {
    goto error;
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code     = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      max_codes[code_len - 1] = SL_MAX(max_codes[code_len - 1], code);
    }
  }

  symbols = sl_alloc(ctx, max_code_len, sizeof(size_t*));
  if (!symbols) {
    goto error;
  }
  for (size_t code_len = 1; code_len <= max_code_len; ++code_len) {
    const size_t max_code = max_codes[code_len - 1];
    symbols[code_len - 1] = sl_alloc(ctx, max_code + 1, sizeof(size_t));
    if (!symbols[code_len - 1]) {
      // free already-allocated symbol arrays
      for (size_t j = 0; j < code_len - 1; ++j) {
        sl_free(symbols[j]);
      }
      goto error;
    }
  }
  for (size_t symbol = 0; symbol <= max_symbol; ++symbol) {
    const size_t code     = codes[symbol];
    const size_t code_len = code_lengths[symbol];
    if (code_len) {
      symbols[code_len - 1][code] = symbol + 1;
    }
  }

  *tree = (struct sl_huffman_tree){.max_code_len = max_code_len,
                                   .max_codes    = max_codes,
                                   .symbols      = symbols};
  sl_free(codes);
  sl_free(next_code);
  sl_free(code_length_count);
  return;

error:
  sl_free(codes);
  sl_free(next_code);
  sl_free(code_length_count);
  sl_free(max_codes);
  sl_free(symbols);
  *tree = (struct sl_huffman_tree){0};
}

void sl_huffman_destroy(struct sl_huffman_tree tree[const static 1]) {
  if (tree) {
    for (size_t code_len = 1; code_len <= tree->max_code_len; ++code_len) {
      sl_free(tree->symbols[code_len - 1]);
    }
    sl_free(tree->max_codes);
    sl_free(tree->symbols);
  }
  *tree = (struct sl_huffman_tree){0};
}
