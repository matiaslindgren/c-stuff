#ifndef SL_ML_H_INCLUDED
#define SL_ML_H_INCLUDED

#include <math.h>
#include <string.h>

#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"
#include "stufflib_rand.h"

void sl_ml_random_train_test_split(struct sl_la_matrix data[const static 1],
                                   struct sl_la_matrix train[const static 1],
                                   struct sl_la_matrix test[const static 1],
                                   int classes[const static 1],
                                   int train_classes[const static 1],
                                   int test_classes[const static 1]) {
  if (train->rows + test->rows != data->rows) {
    SL_LOG_ERROR("train set size + test set size != data set size");
    return;
  }
  if (train->cols != test->cols || data->cols != train->cols) {
    SL_LOG_ERROR("number of features must be equal when doing a split");
    return;
  }
  sl_rand_shuffle_together(data->data,
                           classes,
                           sizeof(float) * (size_t)data->cols,
                           sizeof(int),
                           (size_t)data->rows);
  test->data = data->data;
  train->data = data->data + test->rows * test->cols;
  const size_t n_test = (size_t)test->rows;
  memcpy(test_classes, classes, sizeof(int) * n_test);
  memcpy(train_classes, classes + n_test, sizeof(int) * (size_t)train->rows);
}

// min-max rescaling
// https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)
// 2024-06-16
void sl_ml_rescale_features(struct sl_la_matrix m[const static 1],
                            const float a,
                            const float b) {
  // TODO maybe with doubles to avoid precision loss
  struct sl_la_vector v_min = sl_la_vector_create(m->cols);
  struct sl_la_vector v_max = sl_la_vector_create(m->cols);

  sl_la_matrix_copy_row(&v_min, m, 0);
  sl_la_matrix_copy_row(&v_max, m, 0);
  for (int row = 1; row < m->rows; ++row) {
    for (int col = 0; col < m->cols; ++col) {
      const float value = *sl_la_matrix_get(m, row, col);
      v_min.data[col] = fminf(v_min.data[col], value);
      v_max.data[col] = fmaxf(v_max.data[col], value);
    }
  }
  sl_la_vector_scale(&v_min, -1);
  sl_la_vector_add(&v_max, &v_min);
  sl_la_matrix_add_axis0(m, &v_min);
  sl_la_matrix_div_axis0(m, &v_max);
  sl_la_matrix_mul_axis2(m, b - a);
  sl_la_matrix_add_axis2(m, a);

  sl_la_vector_destroy(&v_max);
  sl_la_vector_destroy(&v_min);
}

struct sl_ml_classification {
  int tp;
  int tn;
  int fp;
  int fn;
};

void sl_ml_classification_update(
    struct sl_ml_classification cls[const static 1],
    const int pred_class,
    const int real_class) {
  cls->tp += real_class && pred_class;
  cls->tn += !real_class && !pred_class;
  cls->fp += !real_class && pred_class;
  cls->fn += real_class && !pred_class;
}

static inline double sl_ml_classification_accuracy(
    struct sl_ml_classification cls[const static 1]) {
  return (double)(cls->tp + cls->tn) / (cls->tp + cls->tn + cls->fp + cls->fn);
}

static inline double sl_ml_classification_precision(
    struct sl_ml_classification cls[const static 1]) {
  return (double)cls->tp / (cls->tp + cls->fp);
}

static inline double sl_ml_classification_f1_score(
    struct sl_ml_classification cls[const static 1]) {
  return (double)(2 * cls->tp) / (2 * cls->tp + cls->fp + cls->fn);
}

void sl_ml_classification_print(
    struct sl_ml_classification cls[const static 1]) {
  printf("{");
  printf("\"tp\":%d,", cls->tp);
  printf("\"tn\":%d,", cls->tn);
  printf("\"fp\":%d,", cls->fp);
  printf("\"fn\":%d,", cls->fn);
  printf("\"accuracy\":%.3f,", sl_ml_classification_accuracy(cls));
  printf("\"precision\":%.3f,", sl_ml_classification_precision(cls));
  printf("\"f1_score\":%.3f", sl_ml_classification_f1_score(cls));
  printf("}\n");
}

// Support Vector Machine
//
// references:
//
// 1.
// https://courses.engr.illinois.edu/cs441/sp2023/lectures/Lecture%2008%20-%20SVMs%20and%20SGD.pdf
// 2024-06-16
//
// 2.
// Shalev-Shwartz, S., Singer, Y., Srebro, N. et al.
// Pegasos: primal estimated sub-gradient solver for SVM. (2011).
// https://doi.org/10.1007/s10107-010-0420-4
// 2024-06-16
//
struct sl_ml_svm {
  struct sl_la_vector w;
  int batch_size;
  int n_epochs;
  float learning_rate;
};

int sl_ml_svm_predict(struct sl_ml_svm svm[const static 1],
                      struct sl_la_vector x[const static 1]) {
  return (sl_la_vector_dot(&(svm->w), x) > 0) ? 1 : 0;
}

void sl_ml_svm_linear_fit(struct sl_ml_svm svm[const static 1],
                          struct sl_la_matrix data[const static 1],
                          const int classes[const static 1]) {
  struct sl_la_vector x = sl_la_vector_create(data->cols);
  struct sl_la_vector s = sl_la_vector_create(data->cols);
  int* indices = sl_alloc((size_t)data->rows, sizeof(int));

  for (int i = 0; i < data->rows; ++i) {
    indices[i] = i;
  }

  const int k = svm->batch_size;
  const float lambda = svm->learning_rate;
  const int n_iterations = svm->n_epochs * data->rows / svm->batch_size;

  int batch_begin = data->rows;

  for (int t = 1; t <= n_iterations; ++t) {
    if (batch_begin + k >= data->rows) {
      sl_rand_shuffle(indices, sizeof(int), (size_t)data->rows);
      batch_begin = 0;
    }

    const float eta = 1.0f / (lambda * (float)t);
    sl_la_vector_clear(&s);

    for (int i = 0; i < k; ++i) {
      const int idx = indices[batch_begin + i];
      sl_la_matrix_copy_row(&x, data, idx);
      const float y = (classes[idx] == 1) ? 1 : -1;
      if (y * sl_la_vector_dot(&(svm->w), &x) < 1) {
        sl_la_vector_scale(&x, y);
        sl_la_vector_add(&s, &x);
      }
    }
    batch_begin += k;

    sl_la_vector_scale(&(svm->w), 1 - eta * lambda);
    sl_la_vector_scale(&s, eta / (float)k);
    sl_la_vector_add(&(svm->w), &s);
  }

  sl_la_vector_destroy(&s);
  sl_la_vector_destroy(&x);
  sl_free(indices);
}

#endif  // SL_ML_H_INCLUDED
