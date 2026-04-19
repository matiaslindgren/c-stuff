#ifndef SL_LINALG_H_INCLUDED
#define SL_LINALG_H_INCLUDED

#include <stddef.h>
#include <stdio.h>

#include <stufflib/context/context.h>
#include <stufflib/math/math.h>
#include <stufflib/matrix/sl_matrix_f32.h>
#include <stufflib/vector/sl_vector_f32.h>

#define SL_LA_VECTOR_CREATE_INLINE(n)                            \
  (struct sl_vector_f32) {                                       \
    .data = (float[(n)]){0}, .length = {(n)}, .capacity = {(n)}, \
  }

#ifndef SL_LA_FLOAT_EQ_TOL
  #define SL_LA_FLOAT_EQ_TOL 1e-12
#endif

#ifndef SL_LA_FLOAT_FORMAT
  #define SL_LA_FLOAT_FORMAT "%+15.12e"
#endif

bool sl_la_vector_create(
    struct sl_context ctx[static 1],
    size_t size,
    struct sl_vector_f32 out[static 1]
);
void sl_la_vector_destroy(struct sl_vector_f32 v[const static 1]);
bool sl_la_matrix_create(
    struct sl_context ctx[static 1],
    size_t rows,
    size_t cols,
    struct sl_matrix_f32 out[static 1]
);
void sl_la_matrix_destroy(struct sl_matrix_f32 a[const static 1]);
struct sl_vector_f32 sl_la_matrix_row_view(struct sl_matrix_f32 a[const static 1], size_t row);
void sl_la_vec_add(size_t count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_sub(size_t count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_mul(size_t count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_min(size_t count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_max(size_t count, float lhs[restrict count], const float rhs[restrict count]);
bool sl_la_vector_is_finite(struct sl_vector_f32 v[const static 1]);
void sl_la_vector_scale(struct sl_vector_f32 v[const static 1], float alpha);
void sl_la_vector_clear(struct sl_vector_f32 v[const static 1]);
float sl_la_vector_dot(
    const struct sl_vector_f32 a[const static 1],
    const struct sl_vector_f32 b[const static 1]
);
void sl_la_vector_add(
    const struct sl_vector_f32 a[const static 1],
    const struct sl_vector_f32 b[const static 1]
);
void sl_la_vector_sub(
    const struct sl_vector_f32 a[const static 1],
    const struct sl_vector_f32 b[const static 1]
);
void sl_la_vector_mul(
    struct sl_vector_f32 lhs[const static 1],
    const struct sl_vector_f32 rhs[const static 1]
);
void sl_la_vector_copy(
    struct sl_vector_f32 dst[const static 1],
    struct sl_vector_f32 src[const static 1]
);
bool sl_la_vector_print(
    struct sl_context ctx[static 1],
    FILE stream[const static 1],
    const struct sl_vector_f32 v[const static 1]
);
bool sl_la_matrix_print(
    struct sl_context ctx[static 1],
    FILE stream[const static 1],
    struct sl_matrix_f32 a[const static 1]
);
void sl_la_matrix_multiply(
    const struct sl_matrix_f32 a[const static 1],
    const struct sl_matrix_f32 b[const static 1],
    struct sl_matrix_f32 c[const static 1]
);
double sl_la_matrix_trace(struct sl_matrix_f32 a[const static 1]);
double sl_la_matrix_frobenius_norm(struct sl_matrix_f32 a[const static 1]);
void sl_la_matrix_copy_row(
    struct sl_vector_f32 dst[const static 1],
    struct sl_matrix_f32 src[const static 1],
    size_t row
);
void sl_la_matrix_saxpy_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1],
    float alpha
);
void sl_la_matrix_add_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1]
);
void sl_la_matrix_sub_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1]
);
void sl_la_matrix_mul_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 v[const static 1]
);
void sl_la_matrix_diffdiv_axis0(
    struct sl_matrix_f32 m[const static 1],
    struct sl_vector_f32 lhs[const static 1],
    struct sl_vector_f32 rhs[const static 1]
);
void sl_la_matrix_add_axis2(struct sl_matrix_f32 m[const static 1], float x);
void sl_la_matrix_mul_axis2(struct sl_matrix_f32 m[const static 1], float x);

static inline bool sl_la_vector_equal(
    struct sl_vector_f32 lhs[const static 1],
    struct sl_vector_f32 rhs[const static 1]
) {
  if (sl_vector_f32_size(lhs) != sl_vector_f32_size(rhs)) {
    return false;
  }
  for (size_t i = 0; i < sl_vector_f32_size(lhs); ++i) {
    if (!sl_math_double_almost((double)lhs->data[i], (double)rhs->data[i], SL_LA_FLOAT_EQ_TOL)) {
      return false;
    }
  }
  return true;
}

static inline bool sl_la_matrix_equal(
    struct sl_matrix_f32 lhs[const static 1],
    struct sl_matrix_f32 rhs[const static 1]
) {
  if (sl_matrix_f32_num_rows(lhs) != sl_matrix_f32_num_rows(rhs)
      || sl_matrix_f32_num_cols(lhs) != sl_matrix_f32_num_cols(rhs)) {
    return false;
  }
  const size_t rows = sl_matrix_f32_num_rows(lhs);
  const size_t cols = sl_matrix_f32_num_cols(lhs);
  for (size_t row = 0; row < rows; ++row) {
    for (size_t col = 0; col < cols; ++col) {
      if (!sl_math_double_almost(
              (double)*sl_matrix_f32_get(lhs, row, col),
              (double)*sl_matrix_f32_get(rhs, row, col),
              SL_LA_FLOAT_EQ_TOL
          )) {
        return false;
      }
    }
  }
  return true;
}

#endif  // SL_LINALG_H_INCLUDED
