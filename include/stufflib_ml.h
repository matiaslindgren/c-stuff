#ifndef SL_ML_H_INCLUDED
#define SL_ML_H_INCLUDED

#include <assert.h>
#include <math.h>
#include <string.h>

#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"
#include "stufflib_rand.h"

void sl_ml_random_train_test_split(struct sl_la_matrix data[const static 1],
                                   struct sl_la_matrix train[const static 1],
                                   struct sl_la_matrix test[const static 1],
                                   uint16_t classes[const static 1],
                                   uint16_t train_classes[const static 1],
                                   uint16_t test_classes[const static 1]) {
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
                           sizeof(uint16_t),
                           (size_t)data->rows);
  test->data = data->data;
  train->data = data->data + test->rows * test->cols;
  const size_t n_test = (size_t)test->rows;
  memcpy(test_classes, classes, sizeof(uint16_t) * n_test);
  memcpy(train_classes,
         classes + n_test,
         sizeof(uint16_t) * (size_t)train->rows);
}

// https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)
// 2024-06-16
struct sl_ml_minmax_scaler {
  struct sl_la_vector lo;
  struct sl_la_vector hi;
};

void sl_ml_minmax_fit(struct sl_ml_minmax_scaler scaler[const static 1],
                      struct sl_la_matrix m[const static 1]) {
  assert(scaler->lo.size == m->cols && scaler->hi.size == m->cols);
  for (int row = 0; row < m->rows; ++row) {
    for (int col = 0; col < m->cols; ++col) {
      const float value = *sl_la_matrix_get(m, row, col);
      scaler->lo.data[col] = fminf(scaler->lo.data[col], value);
      scaler->hi.data[col] = fmaxf(scaler->hi.data[col], value);
    }
  }
}

void sl_ml_minmax_apply(struct sl_ml_minmax_scaler scaler[const static 1],
                        struct sl_la_matrix m[const static 1],
                        const float a,
                        const float b) {
  sl_la_matrix_sub_axis0(m, &(scaler->lo));
  sl_la_matrix_mul_axis2(m, b - a);
  sl_la_matrix_diffdiv_axis0(m, &(scaler->hi), &(scaler->lo));
  sl_la_matrix_add_axis2(m, a);
}

// https://en.wikipedia.org/wiki/F-score#Diagnostic_testing
// 2024-06-22
struct sl_ml_classification {
  int tp;
  int tn;
  int fp;
  int fn;
};

void sl_ml_classification_update(
    struct sl_ml_classification cls[const static 1],
    // TODO don't assume true/false
    const uint16_t pred_class,
    const uint16_t real_class) {
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

static inline double sl_ml_classification_recall(
    struct sl_ml_classification cls[const static 1]) {
  return (double)cls->tp / (cls->tp + cls->fn);
}

static inline double sl_ml_classification_f1_score(
    struct sl_ml_classification cls[const static 1]) {
  return (double)(2 * cls->tp) / (2 * cls->tp + cls->fp + cls->fn);
}

void sl_ml_classification_print(
    FILE stream[const static 1],
    struct sl_ml_classification cls[const static 1]) {
  fprintf(stream, "{");
  fprintf(stream, "\"tp\":%d,", cls->tp);
  fprintf(stream, "\"tn\":%d,", cls->tn);
  fprintf(stream, "\"fp\":%d,", cls->fp);
  fprintf(stream, "\"fn\":%d,", cls->fn);
  fprintf(stream, "\"accuracy\":%.3f,", sl_ml_classification_accuracy(cls));
  fprintf(stream, "\"precision\":%.3f,", sl_ml_classification_precision(cls));
  fprintf(stream, "\"recall\":%.3f,", sl_ml_classification_recall(cls));
  fprintf(stream, "\"f1_score\":%.3f", sl_ml_classification_f1_score(cls));
  fprintf(stream, "}\n");
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
// https://courses.edx.org/assets/courseware/v1/16f13f7ac37ae86ebe0372f2410bcec4/asset-v1:MITx+6.86x+1T2021+type@asset+block/resources_pegasos.pdf
// 2024-06-22
//
// 3.
// Smola, A.J., Schölkopf, B. A tutorial on support vector regression.
// Statistics and Computing 14, 199–222 (2004).
// https://alex.smola.org/papers/2004/SmoSch04.pdf
// 2024-06-22
//
// 4.
// Thorsten Joachims. Training Linear SVMs in Linear Time.
// https://www.cs.cornell.edu/people/tj/publications/joachims_06a.pdf
// 2024-06-22
struct sl_ml_svm {
  struct sl_la_vector w;
  int batch_size;
  int n_epochs;
  float learning_rate;
};

uint8_t sl_ml_svm_binary_predict(struct sl_ml_svm svm[const static 1],
                                 struct sl_la_vector x[const static 1]) {
  return (sl_la_vector_dot(&(svm->w), x) > 0) ? 1 : 0;
}

// implements mini-batch pegasos by Shalev-Shwartz et al. (2011)
void sl_ml_svm_linear_fit(struct sl_ml_svm svm[const static 1],
                          struct sl_la_matrix data[const static 1],
                          const uint16_t classes[const static 1]) {
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

#define SL_ML_MINMAX_SCALER_CREATE(name, n_features)                 \
  struct sl_ml_minmax_scaler name = {                                \
      .lo = (struct sl_la_vector){.size = (n_features),              \
                                  .data = (float[(n_features)]){0}}, \
      .hi = (struct sl_la_vector){.size = (n_features),              \
                                  .data = (float[(n_features)]){0}}, \
  }

#define SL_ML_MINMAX_RESCALE(n_features, dataset, a, b)         \
  do {                                                          \
    SL_ML_MINMAX_SCALER_CREATE(sl_minmax_scaler, (n_features)); \
    sl_ml_minmax_fit(&sl_minmax_scaler, (dataset));             \
    sl_ml_minmax_apply(&sl_minmax_scaler, (dataset), (a), (b)); \
  } while (false)

#endif  // SL_ML_H_INCLUDED
