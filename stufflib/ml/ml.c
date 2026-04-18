#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <stufflib/context/context.h>
#include <stufflib/linalg/linalg.h>
#include <stufflib/ml/ml.h>
#include <stufflib/random/random.h>

void sl_ml_random_train_test_split(
    struct sl_context ctx[static 1],
    uint64_t prng[static 1],
    struct sl_matrix_f32 data[const static 1],
    struct sl_matrix_f32 train[const static 1],
    struct sl_matrix_f32 test[const static 1],
    uint16_t classes[const static 1],
    uint16_t train_classes[const static 1],
    uint16_t test_classes[const static 1]
) {
  if (sl_matrix_f32_num_rows(train) + sl_matrix_f32_num_rows(test)
      != sl_matrix_f32_num_rows(data)) {
    SL_ERROR(ctx, "train set size + test set size != data set size");
    return;
  }
  if (sl_matrix_f32_num_cols(train) != sl_matrix_f32_num_cols(test)
      || sl_matrix_f32_num_cols(data) != sl_matrix_f32_num_cols(train)) {
    SL_ERROR(ctx, "number of features must be equal when doing a split");
    return;
  }
  sl_random_shuffle_together(
      prng,
      data->data,
      classes,
      sizeof(float) * sl_matrix_f32_num_cols(data),
      sizeof(uint16_t),
      sl_matrix_f32_num_rows(data)
  );
  test->data          = data->data;
  train->data         = data->data + (sl_matrix_f32_num_rows(test) * sl_matrix_f32_num_cols(test));
  const size_t n_test = sl_matrix_f32_num_rows(test);
  memcpy(test_classes, classes, sizeof(uint16_t) * n_test);
  memcpy(train_classes, classes + n_test, sizeof(uint16_t) * sl_matrix_f32_num_rows(train));
}

void sl_ml_minmax_fit(
    struct sl_ml_minmax_scaler scaler[const static 1],
    struct sl_matrix_f32 m[const static 1]
) {
  assert(
      sl_vector_f32_size(&scaler->lo) == sl_matrix_f32_num_cols(m)
      && sl_vector_f32_size(&scaler->hi) == sl_matrix_f32_num_cols(m)
  );
  scaler->is_cached = false;
  for (size_t row = 0; row < sl_matrix_f32_num_rows(m); ++row) {
    sl_la_vec_min(sl_matrix_f32_num_cols(m), scaler->lo.data, sl_matrix_f32_get(m, row, 0));
    sl_la_vec_max(sl_matrix_f32_num_cols(m), scaler->hi.data, sl_matrix_f32_get(m, row, 0));
  }
}

void sl_ml_minmax_apply(
    struct sl_ml_minmax_scaler scaler[const static 1],
    struct sl_matrix_f32 m[const static 1],
    const float a,
    const float b
) {
  struct sl_vector_f32* scale  = &(scaler->scale);
  struct sl_vector_f32* offset = &(scaler->offset);
  if (!scaler->is_cached) {
    // TODO BLAS
    for (size_t i = 0; i < sl_vector_f32_size(scale); ++i) {
      const float s   = (b - a) / (scaler->hi.data[i] - scaler->lo.data[i]);
      scale->data[i]  = s;
      offset->data[i] = a - (s * (scaler->lo.data[i]));
    }
    scaler->is_cached = true;
  }
  sl_la_matrix_mul_axis0(m, scale);
  sl_la_matrix_add_axis0(m, offset);
}

void sl_ml_classification_update(
    struct sl_ml_classification cls[const static 1],
    // TODO don't assume true/false
    const uint16_t pred_class,
    const uint16_t real_class
) {
  cls->tp += real_class && pred_class;
  cls->tn += !real_class && !pred_class;
  cls->fp += !real_class && pred_class;
  cls->fn += real_class && !pred_class;
}

double sl_ml_classification_accuracy(struct sl_ml_classification cls[const static 1]) {
  return (double)(cls->tp + cls->tn) / (cls->tp + cls->tn + cls->fp + cls->fn);
}

double sl_ml_classification_precision(struct sl_ml_classification cls[const static 1]) {
  return (double)cls->tp / (cls->tp + cls->fp);
}

double sl_ml_classification_recall(struct sl_ml_classification cls[const static 1]) {
  return (double)cls->tp / (cls->tp + cls->fn);
}

double sl_ml_classification_f1_score(struct sl_ml_classification cls[const static 1]) {
  return (double)(2 * cls->tp) / (2 * cls->tp + cls->fp + cls->fn);
}

void sl_ml_classification_print(
    FILE stream[const static 1],
    struct sl_ml_classification cls[const static 1]
) {
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

uint8_t sl_ml_svm_binary_predict(
    struct sl_ml_svm svm[const static 1],
    struct sl_vector_f32 x[const static 1]
) {
  return (sl_la_vector_dot(&(svm->w), x) > 0) ? 1 : 0;
}

// implements mini-batch pegasos by Shalev-Shwartz et al. (2011)
void sl_ml_svm_linear_fit(
    uint64_t prng[static 1],
    struct sl_ml_svm svm[const static 1],
    struct sl_matrix_f32 data[const static 1],
    const uint16_t classes[const static 1]
) {
  for (size_t i = 0; i < sl_matrix_f32_num_rows(data); ++i) {
    svm->shuffle_buffer[i] = i;
  }

  const int k            = svm->batch_size;
  const float lambda     = svm->learning_rate;
  const int n_iterations = svm->n_epochs * (int)sl_matrix_f32_num_rows(data) / svm->batch_size;

  int batch_begin = (int)sl_matrix_f32_num_rows(data);

  for (int t = 1; t <= n_iterations; ++t) {
    if (batch_begin + k >= (int)sl_matrix_f32_num_rows(data)) {
      sl_random_shuffle(prng, svm->shuffle_buffer, sizeof(size_t), sl_matrix_f32_num_rows(data));
      batch_begin = 0;
    }

    const float eta = 1.0F / (lambda * (float)t);
    sl_la_vector_clear(&(svm->s));

    for (int i = 0; i < k; ++i) {
      const size_t idx = svm->shuffle_buffer[batch_begin + i];
      sl_la_matrix_copy_row(&(svm->x), data, idx);
      const float y = (classes[idx] == 1) ? 1 : -1;
      if (y * sl_la_vector_dot(&(svm->w), &(svm->x)) < 1) {
        sl_la_vector_scale(&(svm->x), y);
        sl_la_vector_add(&(svm->s), &(svm->x));
      }
    }
    batch_begin += k;

    sl_la_vector_scale(&(svm->w), 1 - (eta * lambda));
    sl_la_vector_scale(&(svm->s), eta / (float)k);
    sl_la_vector_add(&(svm->w), &(svm->s));
  }
}
