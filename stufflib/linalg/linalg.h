#ifndef SL_LINALG_H_INCLUDED
#define SL_LINALG_H_INCLUDED

#include <stddef.h>
#include <stdio.h>

#include <stufflib/context/context.h>
#include <stufflib/math/math.h>

#ifndef SL_LA_FLOAT_EQ_TOL
  #define SL_LA_FLOAT_EQ_TOL 1e-12
#endif

#ifndef SL_LA_FLOAT_FORMAT
  #define SL_LA_FLOAT_FORMAT "%+15.12e"
#endif

// TODO generic n dim row-major tensor?
// vector and matrix as views instead of alloc

struct sl_la_vector {
  int size;
  float* data;
};

struct sl_la_matrix {
  int rows;
  int cols;
  float* data;
};

#define SL_LA_VECTOR_CREATE_INLINE(length)         \
  (struct sl_la_vector) {                          \
    .size = (length), .data = (float[(length)]){0} \
  }

struct sl_la_vector sl_la_vector_create(struct sl_context ctx[static 1], int size);
void sl_la_vector_destroy(struct sl_la_vector v[const static 1]);
struct sl_la_matrix sl_la_matrix_create(struct sl_context ctx[static 1], int rows, int cols);
void sl_la_matrix_destroy(struct sl_la_matrix a[const static 1]);
float* sl_la_matrix_get(struct sl_la_matrix a[const static 1], int row, int col);
float* sl_la_matrix_get_row(struct sl_la_matrix a[const static 1], int row);
size_t sl_la_matrix_size(struct sl_la_matrix a[const static 1]);
struct sl_la_vector sl_la_matrix_row_view(struct sl_la_matrix a[const static 1], int row);
void sl_la_vec_add(int count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_sub(int count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_mul(int count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_min(int count, float lhs[restrict count], const float rhs[restrict count]);
void sl_la_vec_max(int count, float lhs[restrict count], const float rhs[restrict count]);
bool sl_la_vector_is_finite(struct sl_la_vector v[const static 1]);
void sl_la_vector_scale(struct sl_la_vector v[const static 1], float alpha);
void sl_la_vector_clear(struct sl_la_vector v[const static 1]);
float sl_la_vector_dot(
    const struct sl_la_vector a[const static 1],
    const struct sl_la_vector b[const static 1]
);
void sl_la_vector_add(
    const struct sl_la_vector a[const static 1],
    const struct sl_la_vector b[const static 1]
);
void sl_la_vector_sub(
    const struct sl_la_vector a[const static 1],
    const struct sl_la_vector b[const static 1]
);
void sl_la_vector_mul(
    struct sl_la_vector lhs[const static 1],
    const struct sl_la_vector rhs[const static 1]
);
void sl_la_vector_copy(
    struct sl_la_vector dst[const static 1],
    struct sl_la_vector src[const static 1]
);
void sl_la_vector_print(FILE stream[const static 1], const struct sl_la_vector v[const static 1]);
void sl_la_matrix_print(FILE stream[const static 1], struct sl_la_matrix a[const static 1]);
void sl_la_matrix_multiply(
    const struct sl_la_matrix a[const static 1],
    const struct sl_la_matrix b[const static 1],
    struct sl_la_matrix c[const static 1]
);
double sl_la_matrix_trace(struct sl_la_matrix a[const static 1]);
double sl_la_matrix_frobenius_norm(struct sl_la_matrix a[const static 1]);
void sl_la_matrix_copy_row(
    struct sl_la_vector dst[const static 1],
    struct sl_la_matrix src[const static 1],
    int row
);
void sl_la_matrix_saxpy_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1],
    float alpha
);
void sl_la_matrix_add_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1]
);
void sl_la_matrix_sub_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1]
);
void sl_la_matrix_mul_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector v[const static 1]
);
void sl_la_matrix_diffdiv_axis0(
    struct sl_la_matrix m[const static 1],
    struct sl_la_vector lhs[const static 1],
    struct sl_la_vector rhs[const static 1]
);
void sl_la_matrix_add_axis2(struct sl_la_matrix m[const static 1], float x);
void sl_la_matrix_mul_axis2(struct sl_la_matrix m[const static 1], float x);

static inline bool sl_la_vector_equal(
    struct sl_la_vector lhs[const static 1],
    struct sl_la_vector rhs[const static 1]
) {
  if (lhs->size != rhs->size) {
    return false;
  }
  for (int i = 0; i < lhs->size; ++i) {
    if (!sl_math_double_almost((double)lhs->data[i], (double)rhs->data[i], SL_LA_FLOAT_EQ_TOL)) {
      return false;
    }
  }
  return true;
}

static inline bool sl_la_matrix_equal(
    struct sl_la_matrix lhs[const static 1],
    struct sl_la_matrix rhs[const static 1]
) {
  if (lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
    return false;
  }
  const int rows = lhs->rows;
  const int cols = lhs->cols;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      if (!sl_math_double_almost(
              (double)*sl_la_matrix_get(lhs, row, col),
              (double)*sl_la_matrix_get(rhs, row, col),
              SL_LA_FLOAT_EQ_TOL
          )) {
        return false;
      }
    }
  }
  return true;
}

#endif  // SL_LINALG_H_INCLUDED
