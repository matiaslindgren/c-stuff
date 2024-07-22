#ifndef SL_HUFFMAN_H_INCLUDED
#define SL_HUFFMAN_H_INCLUDED

struct sl_huffman_tree {
  size_t max_code_len;
  // TODO add min_codes so we use (code - min_code) as index for symbols
  size_t* max_codes;
  size_t** symbols;
};

void sl_huffman_init(struct sl_huffman_tree tree[static 1],
                     const size_t max_symbol,
                     const size_t code_lengths[const max_symbol + 1]);

void sl_huffman_destroy(struct sl_huffman_tree tree[const static 1]);

bool sl_huffman_contains(const struct sl_huffman_tree tree[const static 1],
                         const size_t code,
                         const size_t code_len);

size_t sl_huffman_get(const struct sl_huffman_tree tree[const static 1],
                      const size_t code,
                      const size_t code_len);

#endif  // SL_HUFFMAN_H_INCLUDED
