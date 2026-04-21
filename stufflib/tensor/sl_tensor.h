#ifndef SL_TENSOR_H_INCLUDED
#define SL_TENSOR_H_INCLUDED

#include <stddef.h>
#include <stdlib.h>  // NOLINT(misc-include-cleaner)

#include <stufflib/context/context.h>

#define SL_TENSOR_DECLARE(T, NAME, ORDER)                                          \
                                                                                   \
  struct NAME {                                                                    \
    T* restrict data;                                                              \
    size_t length[(ORDER)];                                                        \
    size_t capacity[(ORDER)];                                                      \
  };                                                                               \
                                                                                   \
  void sl_tensor##ORDER##_##NAME##_wipe(struct NAME t[restrict static 1]);         \
  void sl_tensor##ORDER##_##NAME##_destroy(struct NAME t[restrict static 1]);      \
  void sl_tensor##ORDER##_##NAME##_clear(struct NAME t[restrict static 1]);        \
  size_t sl_tensor##ORDER##_##NAME##_size(const struct NAME t[restrict static 1]); \
  bool sl_tensor##ORDER##_##NAME##_create(                                         \
      struct sl_context ctx[restrict static 1],                                    \
      struct NAME t[restrict static 1],                                            \
      const size_t length[restrict static(ORDER)]                                  \
  );                                                                               \
  size_t sl_tensor##ORDER##_##NAME##_offset(                                       \
      const struct NAME t[restrict static 1],                                      \
      const size_t idx[restrict static(ORDER)]                                     \
  );                                                                               \
  T* sl_tensor##ORDER##_##NAME##_get(                                              \
      struct NAME t[restrict static 1],                                            \
      const size_t idx[restrict static(ORDER)]                                     \
  );                                                                               \
  void sl_tensor##ORDER##_##NAME##_set(                                            \
      struct NAME t[restrict static 1],                                            \
      const size_t idx[restrict static(ORDER)],                                    \
      T val                                                                        \
  );

#define SL_TENSOR_IMPLEMENT(T, NAME, ORDER)                                              \
                                                                                         \
  void sl_tensor##ORDER##_##NAME##_wipe(struct NAME t[restrict static 1]) {              \
    *t = (struct NAME){0};                                                               \
  }                                                                                      \
                                                                                         \
  void sl_tensor##ORDER##_##NAME##_destroy(struct NAME t[restrict static 1]) {           \
    if (t->data) {                                                                       \
      free(t->data);                                                                     \
    }                                                                                    \
    sl_tensor##ORDER##_##NAME##_wipe(t);                                                 \
  }                                                                                      \
                                                                                         \
  void sl_tensor##ORDER##_##NAME##_clear(struct NAME t[restrict static 1]) {             \
    for (size_t ord = 0; ord < (ORDER); ord++) {                                         \
      t->length[ord] = 0;                                                                \
    }                                                                                    \
  }                                                                                      \
                                                                                         \
  size_t sl_tensor##ORDER##_##NAME##_size(const struct NAME t[restrict static 1]) {      \
    size_t n = 1;                                                                        \
    for (size_t ord = 0; ord < (ORDER); ord++) {                                         \
      n *= t->length[ord];                                                               \
    }                                                                                    \
    return n;                                                                            \
  }                                                                                      \
                                                                                         \
  bool sl_tensor##ORDER##_##NAME##_create(                                               \
      struct sl_context ctx[restrict static 1],                                          \
      struct NAME t[restrict static 1],                                                  \
      const size_t length[restrict static(ORDER)]                                        \
  ) {                                                                                    \
    size_t total_size = 1;                                                               \
    for (size_t ord = 0; ord < (ORDER); ord++) {                                         \
      t->length[ord]   = length[ord];                                                    \
      t->capacity[ord] = length[ord];                                                    \
      total_size *= length[ord];                                                         \
    }                                                                                    \
    if (!total_size) {                                                                   \
      SL_ERROR(ctx, "will not allocate " #NAME " of length 0");                          \
      goto fail;                                                                         \
    }                                                                                    \
    if (!(t->data = calloc(total_size, sizeof(T)))) {                                    \
      SL_ERROR(                                                                          \
          ctx,                                                                           \
          "failed allocating " #NAME " of length %zu (type=" #T ", sizeof(" #T ")=%zu)", \
          total_size,                                                                    \
          sizeof(T)                                                                      \
      );                                                                                 \
      goto fail;                                                                         \
    }                                                                                    \
    return true;                                                                         \
  fail:                                                                                  \
    sl_tensor##ORDER##_##NAME##_wipe(t);                                                 \
    return false;                                                                        \
  }                                                                                      \
                                                                                         \
  size_t sl_tensor##ORDER##_##NAME##_offset(                                             \
      const struct NAME t[restrict static 1],                                            \
      const size_t idx[restrict static(ORDER)]                                           \
  ) {                                                                                    \
    size_t offset = 0;                                                                   \
    for (size_t ord = 0; ord < (ORDER); ord++) {                                         \
      offset *= t->capacity[ord];                                                        \
      offset += idx[ord];                                                                \
    }                                                                                    \
    return offset;                                                                       \
  }                                                                                      \
                                                                                         \
  T* sl_tensor##ORDER##_##NAME##_get(                                                    \
      struct NAME t[restrict static 1],                                                  \
      const size_t idx[restrict static(ORDER)]                                           \
  ) {                                                                                    \
    return &t->data[sl_tensor##ORDER##_##NAME##_offset(t, idx)];                         \
  }                                                                                      \
                                                                                         \
  void sl_tensor##ORDER##_##NAME##_set(                                                  \
      struct NAME t[restrict static 1],                                                  \
      const size_t idx[restrict static(ORDER)],                                          \
      T val                                                                              \
  ) {                                                                                    \
    t->data[sl_tensor##ORDER##_##NAME##_offset(t, idx)] = val;                           \
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
  static inline bool NAME##_create(                                                      \
      struct sl_context ctx[restrict static 1],                                          \
      struct NAME t[restrict static 1],                                                  \
      size_t length                                                                      \
  ) {                                                                                    \
    return sl_tensor1_##NAME##_create(ctx, t, (size_t[1]){length});                      \
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
  static inline bool NAME##_create(                                                                \
      struct sl_context ctx[restrict static 1],                                                    \
      struct NAME t[restrict static 1],                                                            \
      size_t rows,                                                                                 \
      size_t cols                                                                                  \
  ) {                                                                                              \
    return sl_tensor2_##NAME##_create(ctx, t, (size_t[2]){rows, cols});                            \
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
#define SL_TENSOR3_DECLARE(T, NAME)                                                            \
  SL_TENSOR_DECLARE(T, NAME, 3)                                                                \
                                                                                               \
  static inline void NAME##_wipe(struct NAME t[restrict static 1]) {                           \
    sl_tensor3_##NAME##_wipe(t);                                                               \
  }                                                                                            \
  static inline void NAME##_destroy(struct NAME t[restrict static 1]) {                        \
    sl_tensor3_##NAME##_destroy(t);                                                            \
  }                                                                                            \
  static inline void NAME##_clear(struct NAME t[restrict static 1]) {                          \
    sl_tensor3_##NAME##_clear(t);                                                              \
  }                                                                                            \
  static inline size_t NAME##_size(const struct NAME t[restrict static 1]) {                   \
    return sl_tensor3_##NAME##_size(t);                                                        \
  }                                                                                            \
  static inline bool NAME##_create(                                                            \
      struct sl_context ctx[restrict static 1],                                                \
      struct NAME t[restrict static 1],                                                        \
      size_t pages,                                                                            \
      size_t rows,                                                                             \
      size_t cols                                                                              \
  ) {                                                                                          \
    return sl_tensor3_##NAME##_create(ctx, t, (size_t[3]){pages, rows, cols});                 \
  }                                                                                            \
  static inline size_t                                                                         \
  NAME##_offset(const struct NAME t[restrict static 1], size_t page, size_t row, size_t col) { \
    return sl_tensor3_##NAME##_offset(t, (size_t[3]){page, row, col});                         \
  }                                                                                            \
  static inline T*                                                                             \
  NAME##_get(struct NAME t[restrict static 1], size_t page, size_t row, size_t col) {          \
    return sl_tensor3_##NAME##_get(t, (size_t[3]){page, row, col});                            \
  }                                                                                            \
  static inline void                                                                           \
  NAME##_set(struct NAME t[restrict static 1], size_t page, size_t row, size_t col, T val) {   \
    sl_tensor3_##NAME##_set(t, (size_t[3]){page, row, col}, val);                              \
  }

#endif /* SL_TENSOR_H_INCLUDED */
