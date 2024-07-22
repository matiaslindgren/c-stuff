#ifndef SL_LINALG_H_INCLUDED
#define SL_LINALG_H_INCLUDED

#ifndef SL_LA_FLOAT_EQ_TOL
  #define SL_LA_FLOAT_EQ_TOL 1e-12
#endif

#ifndef SL_LA_FLOAT_FORMAT
  #define SL_LA_FLOAT_FORMAT "%+15.12e"
#endif

// TODO size_t everywhere, cast LAPACK_INT only at cblas interface
void sl_la_vec_add(const int count,
                   float lhs[restrict count],
                   const float rhs[restrict count]);

void sl_la_vec_sub(const int count,
                   float lhs[restrict count],
                   const float rhs[restrict count]);

void sl_la_vec_mul(const int count,
                   float lhs[restrict count],
                   const float rhs[restrict count]);

void sl_la_vec_min(const int count,
                   float lhs[restrict count],
                   const float rhs[restrict count]);

void sl_la_vec_max(const int count,
                   float lhs[restrict count],
                   const float rhs[restrict count]);

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

struct sl_la_vector sl_la_vector_create(const int size);

#define SL_LA_VECTOR_CREATE_INLINE(length)            \
  (struct sl_la_vector) {                             \
    .size = (length), .data = (float[(length)]) { 0 } \
  }

void sl_la_vector_destroy(struct sl_la_vector v[const static 1]);

struct sl_la_matrix sl_la_matrix_create(const int rows, const int cols);

void sl_la_matrix_destroy(struct sl_la_matrix a[const static 1]);

float* sl_la_matrix_get(struct sl_la_matrix a[const static 1],
                        const int row,
                        const int col);

float* sl_la_matrix_get_row(struct sl_la_matrix a[const static 1],
                            const int row);

size_t sl_la_matrix_size(struct sl_la_matrix a[const static 1]);

struct sl_la_vector sl_la_matrix_row_view(struct sl_la_matrix a[const static 1],
                                          const int row);

bool sl_la_vector_is_finite(struct sl_la_vector v[const static 1]);

void sl_la_vector_scale(struct sl_la_vector v[const static 1],
                        const float alpha);

void sl_la_vector_clear(struct sl_la_vector v[const static 1]);

float sl_la_vector_dot(const struct sl_la_vector a[const static 1],
                       const struct sl_la_vector b[const static 1]);

void sl_la_vector_add(const struct sl_la_vector a[const static 1],
                      const struct sl_la_vector b[const static 1]);

void sl_la_vector_sub(const struct sl_la_vector a[const static 1],
                      const struct sl_la_vector b[const static 1]);

void sl_la_vector_mul(struct sl_la_vector lhs[const static 1],
                      const struct sl_la_vector rhs[const static 1]);

void sl_la_vector_copy(struct sl_la_vector dst[const static 1],
                       struct sl_la_vector src[const static 1]);

void sl_la_vector_print(FILE stream[const static 1],
                        const struct sl_la_vector v[const static 1]);

void sl_la_matrix_print(FILE stream[const static 1],
                        struct sl_la_matrix a[const static 1]);

void sl_la_matrix_multiply(const struct sl_la_matrix a[const static 1],
                           const struct sl_la_matrix b[const static 1],
                           struct sl_la_matrix c[const static 1]);

double sl_la_matrix_trace(struct sl_la_matrix a[const static 1]);

double sl_la_matrix_frobenius_norm(struct sl_la_matrix a[const static 1]);

void sl_la_matrix_copy_row(struct sl_la_vector dst[const static 1],
                           struct sl_la_matrix src[const static 1],
                           const int row);

void sl_la_matrix_saxpy_axis0(struct sl_la_matrix m[const static 1],
                              struct sl_la_vector v[const static 1],
                              const float alpha);

void sl_la_matrix_add_axis0(struct sl_la_matrix m[const static 1],
                            struct sl_la_vector v[const static 1]);

void sl_la_matrix_sub_axis0(struct sl_la_matrix m[const static 1],
                            struct sl_la_vector v[const static 1]);

void sl_la_matrix_mul_axis0(struct sl_la_matrix m[const static 1],
                            struct sl_la_vector v[const static 1]);

void sl_la_matrix_diffdiv_axis0(struct sl_la_matrix m[const static 1],
                                struct sl_la_vector lhs[const static 1],
                                struct sl_la_vector rhs[const static 1]);

void sl_la_matrix_add_axis2(struct sl_la_matrix m[const static 1],
                            const float x);

void sl_la_matrix_mul_axis2(struct sl_la_matrix m[const static 1],
                            const float x);

bool sl_la_vector_equal(struct sl_la_vector lhs[const static 1],
                        struct sl_la_vector rhs[const static 1]);

bool sl_la_matrix_equal(struct sl_la_matrix lhs[const static 1],
                        struct sl_la_matrix rhs[const static 1]);

#endif  // SL_LINALG_H_INCLUDED
