#ifndef SL_ML_H_INCLUDED
#define SL_ML_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <stufflib/context/context.h>
#include <stufflib/linalg/linalg.h>

// https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)
// 2024-06-16
struct sl_ml_minmax_scaler {
  struct sl_vector_f32 lo;
  struct sl_vector_f32 hi;
  struct sl_vector_f32 scale;
  struct sl_vector_f32 offset;
  bool is_cached;
};

// https://en.wikipedia.org/wiki/F-score#Diagnostic_testing
// 2024-06-22
struct sl_ml_classification {
  int tp;
  int tn;
  int fp;
  int fn;
};

// Support Vector Machine
struct sl_ml_svm {
  struct sl_vector_f32 w;
  struct sl_vector_f32 x;
  struct sl_vector_f32 s;
  size_t* shuffle_buffer;
  int batch_size;
  int n_epochs;
  float learning_rate;
};

void sl_ml_random_train_test_split(
    struct sl_context ctx[static 1],
    uint64_t prng[static 1],
    struct sl_matrix_f32 data[const static 1],
    struct sl_matrix_f32 train[const static 1],
    struct sl_matrix_f32 test[const static 1],
    uint16_t classes[const static 1],
    uint16_t train_classes[const static 1],
    uint16_t test_classes[const static 1]
);
void sl_ml_minmax_fit(
    struct sl_ml_minmax_scaler scaler[const static 1],
    struct sl_matrix_f32 m[const static 1]
);
void sl_ml_minmax_apply(
    struct sl_ml_minmax_scaler scaler[const static 1],
    struct sl_matrix_f32 m[const static 1],
    float a,
    float b
);
void sl_ml_classification_update(
    struct sl_ml_classification cls[const static 1],
    uint16_t pred_class,
    uint16_t real_class
);
double sl_ml_classification_accuracy(struct sl_ml_classification cls[const static 1]);
double sl_ml_classification_precision(struct sl_ml_classification cls[const static 1]);
double sl_ml_classification_recall(struct sl_ml_classification cls[const static 1]);
double sl_ml_classification_f1_score(struct sl_ml_classification cls[const static 1]);
void sl_ml_classification_print(
    FILE stream[const static 1],
    struct sl_ml_classification cls[const static 1]
);
uint8_t sl_ml_svm_binary_predict(
    struct sl_ml_svm svm[const static 1],
    struct sl_vector_f32 x[const static 1]
);
void sl_ml_svm_linear_fit(
    uint64_t prng[static 1],
    struct sl_ml_svm svm[const static 1],
    struct sl_matrix_f32 data[const static 1],
    const uint16_t classes[const static 1]
);

#define SL_ML_MINMAX_SCALER_CREATE_INLINE(n_features) \
  (struct sl_ml_minmax_scaler) {                      \
    .lo                                               \
        = {.data     = (float[(n_features)]){0},      \
           .length   = {(n_features)},                \
           .capacity = {(n_features)}},               \
        .hi                                           \
        = {.data     = (float[(n_features)]){0},      \
           .length   = {(n_features)},                \
           .capacity = {(n_features)}},               \
        .scale                                        \
        = {.data     = (float[(n_features)]){0},      \
           .length   = {(n_features)},                \
           .capacity = {(n_features)}},               \
        .offset                                       \
        = {.data     = (float[(n_features)]){0},      \
           .length   = {(n_features)},                \
           .capacity = {(n_features)}},               \
  }

#define SL_ML_MINMAX_RESCALE(n_features, dataset, a, b)                                            \
  do {                                                                                             \
    struct sl_ml_minmax_scaler sl_minmax_scaler = SL_ML_MINMAX_SCALER_CREATE_INLINE((n_features)); \
    sl_ml_minmax_fit(&sl_minmax_scaler, (dataset));                                                \
    sl_ml_minmax_apply(&sl_minmax_scaler, (dataset), (a), (b));                                    \
  } while (false)

#endif  // SL_ML_H_INCLUDED
