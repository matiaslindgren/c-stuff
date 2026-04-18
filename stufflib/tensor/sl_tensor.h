#ifndef SL_TENSOR_H_INCLUDED
#define SL_TENSOR_H_INCLUDED

#include <stddef.h>
#include <stdlib.h>

#define SL_TENSOR_DECLARE(T, NAME, DIM)                                          \
                                                                                 \
  struct NAME {                                                                  \
    T* restrict data;                                                            \
    size_t length[DIM];                                                          \
    size_t capacity[DIM];                                                        \
  };                                                                             \
                                                                                 \
  void sl_tensor##DIM##_##NAME##_wipe(struct NAME t[restrict static 1]);         \
  void sl_tensor##DIM##_##NAME##_destroy(struct NAME t[restrict static 1]);      \
  void sl_tensor##DIM##_##NAME##_clear(struct NAME t[restrict static 1]);        \
  size_t sl_tensor##DIM##_##NAME##_size(const struct NAME t[restrict static 1]); \
  size_t sl_tensor##DIM##_##NAME##_offset(                                       \
      const struct NAME t[restrict static 1],                                    \
      const size_t idx[restrict static DIM]                                      \
  );                                                                             \
  T* sl_tensor##DIM##_##NAME##_get(                                              \
      struct NAME t[restrict static 1],                                          \
      const size_t idx[restrict static DIM]                                      \
  );                                                                             \
  void sl_tensor##DIM##_##NAME##_set(                                            \
      struct NAME t[restrict static 1],                                          \
      const size_t idx[restrict static DIM],                                     \
      T val                                                                      \
  );

#define SL_TENSOR_IMPLEMENT(T, NAME, DIM)                                         \
                                                                                  \
  void sl_tensor##DIM##_##NAME##_wipe(struct NAME t[restrict static 1]) {         \
    *t = (struct NAME){0};                                                        \
  }                                                                               \
                                                                                  \
  void sl_tensor##DIM##_##NAME##_destroy(struct NAME t[restrict static 1]) {      \
    if (t->data) {                                                                \
      free(t->data);                                                              \
    }                                                                             \
    sl_tensor##DIM##_##NAME##_wipe(t);                                            \
  }                                                                               \
                                                                                  \
  void sl_tensor##DIM##_##NAME##_clear(struct NAME t[restrict static 1]) {        \
    for (size_t d = 0; d < DIM; d++) {                                            \
      t->length[d] = 0;                                                           \
    }                                                                             \
  }                                                                               \
                                                                                  \
  size_t sl_tensor##DIM##_##NAME##_size(const struct NAME t[restrict static 1]) { \
    size_t n = 1;                                                                 \
    for (size_t d = 0; d < DIM; d++) {                                            \
      n *= t->length[d];                                                          \
    }                                                                             \
    return n;                                                                     \
  }                                                                               \
                                                                                  \
  size_t sl_tensor##DIM##_##NAME##_offset(                                        \
      const struct NAME t[restrict static 1],                                     \
      const size_t idx[restrict static DIM]                                       \
  ) {                                                                             \
    size_t offset = 0;                                                            \
    for (size_t d = 0; d < DIM; d++) {                                            \
      offset *= t->capacity[d];                                                   \
      offset += idx[d];                                                           \
    }                                                                             \
    return offset;                                                                \
  }                                                                               \
                                                                                  \
  T* sl_tensor##DIM##_##NAME##_get(                                               \
      struct NAME t[restrict static 1],                                           \
      const size_t idx[restrict static DIM]                                       \
  ) {                                                                             \
    return &t->data[sl_tensor##DIM##_##NAME##_offset(t, idx)];                    \
  }                                                                               \
                                                                                  \
  void sl_tensor##DIM##_##NAME##_set(                                             \
      struct NAME t[restrict static 1],                                           \
      const size_t idx[restrict static DIM],                                      \
      T val                                                                       \
  ) {                                                                             \
    t->data[sl_tensor##DIM##_##NAME##_offset(t, idx)] = val;                      \
  }

#define SL_VECTOR_IMPLEMENT(T, NAME) SL_TENSOR_IMPLEMENT(T, NAME, 1)
#define SL_VECTOR_DECLARE(T, NAME)                                                       \
  SL_TENSOR_DECLARE(T, NAME, 1)                                                          \
                                                                                         \
  static inline void NAME##_wipe(struct NAME t[restrict static 1]) {                     \
    sl_tensor1_##NAME##_wipe(t);                                                         \
  }                                                                                      \
  static inline void NAME##_destroy(struct NAME t[restrict static 1]) {                  \
    sl_tensor1_##NAME##_destroy(t);                                                      \
  }                                                                                      \
  static inline void NAME##_clear(struct NAME t[restrict static 1]) {                    \
    sl_tensor1_##NAME##_clear(t);                                                        \
  }                                                                                      \
  static inline size_t NAME##_size(const struct NAME t[restrict static 1]) {             \
    return t->length[0];                                                                 \
  }                                                                                      \
  static inline size_t NAME##_offset(const struct NAME t[restrict static 1], size_t i) { \
    return sl_tensor1_##NAME##_offset(t, (size_t[1]){i});                                \
  }                                                                                      \
  static inline T* NAME##_get(struct NAME t[restrict static 1], size_t i) {              \
    return sl_tensor1_##NAME##_get(t, (size_t[1]){i});                                   \
  }                                                                                      \
  static inline void NAME##_set(struct NAME t[restrict static 1], size_t i, T val) {     \
    sl_tensor1_##NAME##_set(t, (size_t[1]){i}, val);                                     \
  }

#define SL_MATRIX_IMPLEMENT(T, NAME) SL_TENSOR_IMPLEMENT(T, NAME, 2)
#define SL_MATRIX_DECLARE(T, NAME)                                                                 \
  SL_TENSOR_DECLARE(T, NAME, 2)                                                                    \
                                                                                                   \
  static inline void NAME##_wipe(struct NAME t[restrict static 1]) {                               \
    sl_tensor2_##NAME##_wipe(t);                                                                   \
  }                                                                                                \
  static inline void NAME##_destroy(struct NAME t[restrict static 1]) {                            \
    sl_tensor2_##NAME##_destroy(t);                                                                \
  }                                                                                                \
  static inline void NAME##_clear(struct NAME t[restrict static 1]) {                              \
    sl_tensor2_##NAME##_clear(t);                                                                  \
  }                                                                                                \
  static inline size_t NAME##_size(const struct NAME t[restrict static 1]) {                       \
    return sl_tensor2_##NAME##_size(t);                                                            \
  }                                                                                                \
  static inline size_t NAME##_num_rows(const struct NAME t[restrict static 1]) {                   \
    return t->length[0];                                                                           \
  }                                                                                                \
  static inline size_t NAME##_num_cols(const struct NAME t[restrict static 1]) {                   \
    return t->length[1];                                                                           \
  }                                                                                                \
  static inline size_t NAME##_offset(                                                              \
      const struct NAME t[restrict static 1],                                                      \
      size_t row,                                                                                  \
      size_t col                                                                                   \
  ) {                                                                                              \
    return sl_tensor2_##NAME##_offset(t, (size_t[2]){row, col});                                   \
  }                                                                                                \
  static inline T* NAME##_get(struct NAME t[restrict static 1], size_t row, size_t col) {          \
    return sl_tensor2_##NAME##_get(t, (size_t[2]){row, col});                                      \
  }                                                                                                \
  static inline void NAME##_set(struct NAME t[restrict static 1], size_t row, size_t col, T val) { \
    sl_tensor2_##NAME##_set(t, (size_t[2]){row, col}, val);                                        \
  }

#define SL_TENSOR3_IMPLEMENT(T, NAME) SL_TENSOR_IMPLEMENT(T, NAME, 3)
#define SL_TENSOR3_DECLARE(T, NAME)                                                             \
  SL_TENSOR_DECLARE(T, NAME, 3)                                                                 \
                                                                                                \
  static inline void NAME##_wipe(struct NAME t[restrict static 1]) {                            \
    sl_tensor3_##NAME##_wipe(t);                                                                \
  }                                                                                             \
  static inline void NAME##_destroy(struct NAME t[restrict static 1]) {                         \
    sl_tensor3_##NAME##_destroy(t);                                                             \
  }                                                                                             \
  static inline void NAME##_clear(struct NAME t[restrict static 1]) {                           \
    sl_tensor3_##NAME##_clear(t);                                                               \
  }                                                                                             \
  static inline size_t NAME##_size(const struct NAME t[restrict static 1]) {                    \
    return sl_tensor3_##NAME##_size(t);                                                         \
  }                                                                                             \
  static inline size_t                                                                          \
  NAME##_offset(const struct NAME t[restrict static 1], size_t i, size_t j, size_t k) {         \
    return sl_tensor3_##NAME##_offset(t, (size_t[3]){i, j, k});                                 \
  }                                                                                             \
  static inline T* NAME##_get(struct NAME t[restrict static 1], size_t i, size_t j, size_t k) { \
    return sl_tensor3_##NAME##_get(t, (size_t[3]){i, j, k});                                    \
  }                                                                                             \
  static inline void                                                                            \
  NAME##_set(struct NAME t[restrict static 1], size_t i, size_t j, size_t k, T val) {           \
    sl_tensor3_##NAME##_set(t, (size_t[3]){i, j, k}, val);                                      \
  }

#endif /* SL_TENSOR_H_INCLUDED */
