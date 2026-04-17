#ifndef SL_TENSOR_H_INCLUDED
#define SL_TENSOR_H_INCLUDED

#include <stddef.h>
#include <stdlib.h>

#define SL_TENSOR_DECLARE(T, NAME, DIM)                                  \
                                                                         \
  struct NAME {                                                          \
    T* restrict data;                                                    \
    size_t length[DIM];                                                  \
    size_t capacity[DIM];                                                \
  };                                                                     \
                                                                         \
  void sl_tensor##DIM##_wipe(struct NAME t[restrict static 1]);          \
  void sl_tensor##DIM##_destroy(struct NAME t[restrict static 1]);       \
  void sl_tensor##DIM##_clear(struct NAME t[restrict static 1]);         \
  size_t sl_tensor##DIM##_count(const struct NAME t[restrict static 1]); \
  size_t sl_tensor##DIM##_offset(                                        \
      const struct NAME t[restrict static 1],                            \
      const size_t idx[restrict static DIM]                              \
  );                                                                     \
  T* sl_tensor##DIM##_get(                                               \
      struct NAME t[restrict static 1],                                  \
      const size_t idx[restrict static DIM]                              \
  );                                                                     \
  void sl_tensor##DIM##_set(                                             \
      struct NAME t[restrict static 1],                                  \
      const size_t idx[restrict static DIM],                             \
      T val                                                              \
  );

#define SL_TENSOR_IMPLEMENT(T, NAME, DIM)                                 \
                                                                          \
  void sl_tensor##DIM##_wipe(struct NAME t[restrict static 1]) {          \
    *t = (struct NAME){0};                                                \
  }                                                                       \
                                                                          \
  void sl_tensor##DIM##_destroy(struct NAME t[restrict static 1]) {       \
    if (t->data) {                                                        \
      free(t->data);                                                      \
    }                                                                     \
    sl_tensor##DIM##_wipe(t);                                             \
  }                                                                       \
                                                                          \
  void sl_tensor##DIM##_clear(struct NAME t[restrict static 1]) {         \
    for (size_t d = 0; d < DIM; d++) {                                    \
      t->length[d] = 0;                                                   \
    }                                                                     \
  }                                                                       \
                                                                          \
  size_t sl_tensor##DIM##_count(const struct NAME t[restrict static 1]) { \
    size_t n = 1;                                                         \
    for (size_t d = 0; d < DIM; d++) n *= t->length[d];                   \
    return n;                                                             \
  }                                                                       \
                                                                          \
  size_t sl_tensor##DIM##_offset(                                         \
      const struct NAME t[restrict static 1],                             \
      const size_t idx[restrict static DIM]                               \
  ) {                                                                     \
    size_t offset = 0;                                                    \
    for (size_t d = 0; d < DIM; d++) {                                    \
      offset *= t->capacity[d];                                           \
      offset += idx[d];                                                   \
    }                                                                     \
    return offset;                                                        \
  }                                                                       \
                                                                          \
  T* sl_tensor##DIM##_get(                                                \
      struct NAME t[restrict static 1],                                   \
      const size_t idx[restrict static DIM]                               \
  ) {                                                                     \
    return &t->data[sl_tensor##DIM##_offset(t, idx)];                     \
  }                                                                       \
                                                                          \
  void sl_tensor##DIM##_set(                                              \
      struct NAME t[restrict static 1],                                   \
      const size_t idx[restrict static DIM],                              \
      T val                                                               \
  ) {                                                                     \
    t->data[sl_tensor##DIM##_offset(t, idx)] = val;                       \
  }

#define SL_VECTOR_IMPLEMENT(T, NAME) SL_TENSOR_IMPLEMENT(T, NAME, 1)
#define SL_VECTOR_DECLARE(T, NAME)                                                       \
  SL_TENSOR_DECLARE(T, NAME, 1)                                                          \
                                                                                         \
  static inline void NAME##_wipe(struct NAME t[restrict static 1]) {                     \
    sl_tensor1_wipe(t);                                                                  \
  }                                                                                      \
  static inline void NAME##_destroy(struct NAME t[restrict static 1]) {                  \
    sl_tensor1_destroy(t);                                                               \
  }                                                                                      \
  static inline void NAME##_clear(struct NAME t[restrict static 1]) {                    \
    sl_tensor1_clear(t);                                                                 \
  }                                                                                      \
  static inline size_t NAME##_count(const struct NAME t[restrict static 1]) {            \
    return sl_tensor1_count(t);                                                          \
  }                                                                                      \
  static inline size_t NAME##_offset(const struct NAME t[restrict static 1], size_t i) { \
    return sl_tensor1_offset(t, (size_t[1]){i});                                         \
  }                                                                                      \
  static inline T* NAME##_get(struct NAME t[restrict static 1], size_t i) {              \
    return sl_tensor1_get(t, (size_t[1]){i});                                            \
  }                                                                                      \
  static inline void NAME##_set(struct NAME t[restrict static 1], size_t i, T val) {     \
    sl_tensor1_set(t, (size_t[1]){i}, val);                                              \
  }

#define SL_MATRIX_IMPLEMENT(T, NAME) SL_TENSOR_IMPLEMENT(T, NAME, 2)
#define SL_MATRIX_DECLARE(T, NAME)                                                                 \
  SL_TENSOR_DECLARE(T, NAME, 2)                                                                    \
                                                                                                   \
  static inline void NAME##_wipe(struct NAME t[restrict static 1]) {                               \
    sl_tensor2_wipe(t);                                                                            \
  }                                                                                                \
  static inline void NAME##_destroy(struct NAME t[restrict static 1]) {                            \
    sl_tensor2_destroy(t);                                                                         \
  }                                                                                                \
  static inline void NAME##_clear(struct NAME t[restrict static 1]) {                              \
    sl_tensor2_clear(t);                                                                           \
  }                                                                                                \
  static inline size_t NAME##_count(const struct NAME t[restrict static 1]) {                      \
    return sl_tensor2_count(t);                                                                    \
  }                                                                                                \
  static inline size_t NAME##_offset(                                                              \
      const struct NAME t[restrict static 1],                                                      \
      size_t row,                                                                                  \
      size_t col                                                                                   \
  ) {                                                                                              \
    return sl_tensor2_offset(t, (size_t[2]){row, col});                                            \
  }                                                                                                \
  static inline T* NAME##_get(struct NAME t[restrict static 1], size_t row, size_t col) {          \
    return sl_tensor2_get(t, (size_t[2]){row, col});                                               \
  }                                                                                                \
  static inline void NAME##_set(struct NAME t[restrict static 1], size_t row, size_t col, T val) { \
    sl_tensor2_set(t, (size_t[2]){row, col}, val);                                                 \
  }

#endif /* SL_TENSOR_H_INCLUDED */
