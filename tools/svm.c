#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib/args/args.h"
#include "stufflib/dataset/dataset.h"
#include "stufflib/linalg/linalg.h"
#include "stufflib/macros/macros.h"
#include "stufflib/ml/ml.h"
#include "stufflib/record/reader.h"
#include "stufflib/record/record.h"

bool spambase(const struct sl_args args[const static 1]) {
  bool all_ok = false;

  const char* const dataset_dir = sl_args_get_positional(args, 1);
  const bool verbose = sl_args_parse_flag(args, "-v");

  if (verbose) {
    SL_LOG_INFO("training linear SVM on spambase dataset from '%s'",
                dataset_dir);
  }

  struct sl_record classes_record;
  uint16_t classes[SL_DATASET_SPAMBASE_SAMPLES] = {0};

  struct sl_record samples_record;
  struct sl_la_matrix samples =
      sl_la_matrix_create(SL_DATASET_SPAMBASE_SAMPLES,
                          SL_DATASET_SPAMBASE_FEATURES);

  if (!sl_record_read_metadata(&classes_record,
                               dataset_dir,
                               "spambase_classes")) {
    SL_LOG_ERROR("failed reading metadata of spambase classes");
    goto done;
  }

  if (!sl_record_read_metadata(&samples_record,
                               dataset_dir,
                               "spambase_samples")) {
    SL_LOG_ERROR("failed reading metadata of spambase samples");
    goto done;
  }

  if (!sl_record_read_all(&classes_record, sizeof(classes), (void*)classes)) {
    SL_LOG_ERROR("failed reading spambase classes");
    goto done;
  }

  if (!sl_record_read_all(
          &samples_record,
          sl_record_item_size(&samples_record) * sl_la_matrix_size(&samples),
          (void*)(samples.data))) {
    SL_LOG_ERROR("failed reading spambase samples");
    goto done;
  }

  struct sl_ml_svm svm = {
      .w = SL_LA_VECTOR_CREATE_INLINE(SL_DATASET_SPAMBASE_FEATURES),
      .s = SL_LA_VECTOR_CREATE_INLINE(SL_DATASET_SPAMBASE_FEATURES),
      .x = SL_LA_VECTOR_CREATE_INLINE(SL_DATASET_SPAMBASE_FEATURES),
      .shuffle_buffer = (size_t[SL_DATASET_SPAMBASE_SAMPLES]){0},
      .batch_size = 1,
      .n_epochs = 2,
      .learning_rate = 1e-9f,
  };

  struct sl_la_matrix test_data = {
      .rows = 2000,
      .cols = samples.cols,
  };
  uint16_t test_classes[2000] = {0};

  struct sl_la_matrix train_data = {
      .rows = samples.rows - test_data.rows,
      .cols = samples.cols,
  };
  uint16_t train_classes[SL_ARRAY_LEN(classes) - SL_ARRAY_LEN(test_classes)] = {
      0};

  sl_ml_random_train_test_split(&samples,
                                &train_data,
                                &test_data,
                                classes,
                                train_classes,
                                test_classes);

  struct sl_ml_minmax_scaler minmax_scaler =
      SL_ML_MINMAX_SCALER_CREATE_INLINE(SL_DATASET_SPAMBASE_FEATURES);
  sl_ml_minmax_fit(&minmax_scaler, &train_data);
  sl_ml_minmax_apply(&minmax_scaler, &train_data, -1, 1);
  sl_ml_minmax_apply(&minmax_scaler, &test_data, -1, 1);

  sl_ml_svm_linear_fit(&svm, &train_data, train_classes);

  {
    struct sl_ml_classification report = {0};
    for (int i = 0; i < train_data.rows; ++i) {
      struct sl_la_vector x = sl_la_matrix_row_view(&train_data, i);
      sl_ml_classification_update(&report,
                                  train_classes[i],
                                  sl_ml_svm_binary_predict(&svm, &x));
    }
    SL_LOG_INFO("spambase dataset, random train set, linear SVM");
    sl_ml_classification_print(stderr, &report);
  }
  {
    struct sl_ml_classification report = {0};
    for (int i = 0; i < test_data.rows; ++i) {
      struct sl_la_vector x = sl_la_matrix_row_view(&test_data, i);
      sl_ml_classification_update(&report,
                                  test_classes[i],
                                  sl_ml_svm_binary_predict(&svm, &x));
    }
    SL_LOG_INFO("spambase dataset, random test set, linear SVM");
    sl_ml_classification_print(stderr, &report);
  }

  all_ok = true;
done:
  sl_la_matrix_destroy(&samples);
  return all_ok;
}

#define SL_SVM_RCV1_BUFFER_LEN 10'000

bool rcv1(const struct sl_args args[const static 1]) {
  bool all_ok = false;

  const char* const dataset_dir = sl_args_get_positional(args, 1);
  const bool verbose = sl_args_parse_flag(args, "-v");
  const bool flip_train_test = sl_args_parse_flag(args, "--flip-train-test");

  if (verbose) {
    SL_LOG_INFO("RCV1: training linear SVM, reading dataset from '%s'",
                dataset_dir);
  }

  struct sl_record train_samples_record = {0};
  struct sl_file train_record_file = {0};
  struct sl_record_reader train_samples_reader = {
      .file = &train_record_file,
      .record = &train_samples_record,
  };

  struct sl_record train_classes_record = {0};
  uint8_t* train_classes = nullptr;

  struct sl_record test_samples_record = {0};
  struct sl_file test_record_file = {0};
  struct sl_record_reader test_samples_reader = {
      .file = &test_record_file,
      .record = &test_samples_record,
  };

  struct sl_record test_classes_record = {0};
  uint8_t* test_classes = nullptr;

  struct sl_la_matrix samples_batch =
      sl_la_matrix_create(SL_SVM_RCV1_BUFFER_LEN, SL_DATASET_RCV1_FEATURES);
  struct sl_span read_buffer =
      sl_span_view(sizeof(float) * sl_la_matrix_size(&samples_batch),
                   (void*)samples_batch.data);

  if (verbose) {
    SL_LOG_INFO("RCV1: reading metadata records");
  }

  if (!sl_record_read_metadata(
          &train_samples_record,
          dataset_dir,
          flip_train_test ? "rcv1_test_samples" : "rcv1_train_samples")) {
    SL_LOG_ERROR("failed reading metadata of RCV1 training set samples");
    goto done;
  }
  if (!sl_record_read_metadata(
          &test_samples_record,
          dataset_dir,
          flip_train_test ? "rcv1_train_samples" : "rcv1_test_samples")) {
    SL_LOG_ERROR("failed reading metadata of RCV1 testing set samples");
    goto done;
  }

  if (!sl_record_read_metadata(
          &train_classes_record,
          dataset_dir,
          flip_train_test ? "rcv1_test_classes" : "rcv1_train_classes")) {
    SL_LOG_ERROR("failed reading metadata of RCV1 training set classes");
    goto done;
  }
  if (!sl_record_read_metadata(
          &test_classes_record,
          dataset_dir,
          flip_train_test ? "rcv1_train_classes" : "rcv1_test_classes")) {
    SL_LOG_ERROR("failed reading metadata of RCV1 testing set classes");
    goto done;
  }

  const size_t class_size = sl_record_item_size(&train_classes_record);

  train_classes = sl_alloc(train_classes_record.size, class_size);
  test_classes = sl_alloc(test_classes_record.size, class_size);

  if (verbose) {
    SL_LOG_INFO("RCV1: reading classes records");
  }

  if (!sl_record_read_all(&train_classes_record,
                          train_classes_record.size * class_size,
                          (void*)train_classes)) {
    SL_LOG_ERROR("failed reading RCV1 training set classes");
    goto done;
  }
  for (size_t i = 0; i < train_classes_record.size; ++i) {
    if (train_classes[i] > 1) {
      SL_LOG_ERROR("invalid RCV1 training class %u", train_classes[i]);
      goto done;
    }
  }

  if (!sl_record_read_all(&test_classes_record,
                          test_classes_record.size * class_size,
                          (void*)test_classes)) {
    SL_LOG_ERROR("failed reading RCV1 testing set classes");
    goto done;
  }
  for (size_t i = 0; i < test_classes_record.size; ++i) {
    if (test_classes[i] > 1) {
      SL_LOG_ERROR("invalid RCV1 testing class %u", test_classes[i]);
      goto done;
    }
  }

  if (!sl_record_reader_open(&train_samples_reader)) {
    SL_LOG_ERROR("failed opening RCV1 training set samples record reader");
    goto done;
  }

  if (verbose) {
    SL_LOG_INFO("RCV1: fitting minmax scaler on training set");
  }

  struct sl_ml_minmax_scaler minmax_scaler =
      SL_ML_MINMAX_SCALER_CREATE_INLINE(SL_DATASET_RCV1_FEATURES);

  for (size_t batch_idx = 0; !sl_record_reader_is_done(&train_samples_reader);
       ++batch_idx) {
    if (verbose) {
      SL_LOG_INFO("RCV1 train batch %zu minmax: read batch", batch_idx);
    }
    sl_span_clear(&read_buffer);
    if (!sl_record_reader_read(&train_samples_reader, &read_buffer)) {
      SL_LOG_ERROR("RCV1 train batch %zu minmax: failed reading samples batch",
                   batch_idx);
      goto done;
    }
    for (int row = 0; row < samples_batch.rows; ++row) {
      for (int col = 0; col < samples_batch.cols; ++col) {
        const float value = *sl_la_matrix_get(&samples_batch, row, col);
        if (value < 0 || value > 1) {
          SL_LOG_ERROR(
              "invalid RCV1 training sample at (%d, %d): %g is not in [0, 1]",
              row,
              col,
              (double)value);
          goto done;
        }
      }
    }
    if (verbose) {
      SL_LOG_INFO("RCV1 train batch %zu minmax: fit", batch_idx);
    }
    sl_ml_minmax_fit(&minmax_scaler, &samples_batch);
  }

  sl_record_reader_close(&train_samples_reader);

  train_record_file = (struct sl_file){0};
  train_samples_reader = (struct sl_record_reader){
      .file = &train_record_file,
      .record = &train_samples_record,
  };

  if (!sl_record_reader_open(&train_samples_reader)) {
    SL_LOG_ERROR("failed opening RCV1 training set samples record reader");
    goto done;
  }
  if (!sl_record_reader_open(&test_samples_reader)) {
    SL_LOG_ERROR("failed opening RCV1 testing set samples record reader");
    goto done;
  }

  if (verbose) {
    SL_LOG_INFO("RCV1: fitting linear SVM on training set");
  }

  struct sl_ml_svm svm = {
      .w = SL_LA_VECTOR_CREATE_INLINE(SL_DATASET_RCV1_FEATURES),
      .s = SL_LA_VECTOR_CREATE_INLINE(SL_DATASET_RCV1_FEATURES),
      .x = SL_LA_VECTOR_CREATE_INLINE(SL_DATASET_RCV1_FEATURES),
      .shuffle_buffer = (size_t[SL_SVM_RCV1_BUFFER_LEN]){0},
      .batch_size = 100,
      .n_epochs = 1,
      // learning rate from Shalev-Shwartz et al. (2011)
      .learning_rate = 1e-4f,
  };

  uint16_t classes_buffer[SL_SVM_RCV1_BUFFER_LEN] = {0};
  for (size_t batch_idx = 0; !sl_record_reader_is_done(&train_samples_reader);
       ++batch_idx) {
    if (verbose) {
      SL_LOG_INFO("RCV1 train batch %zu: reading buffer", batch_idx);
    }
    sl_span_clear(&read_buffer);
    if (!sl_record_reader_read(&train_samples_reader, &read_buffer)) {
      SL_LOG_ERROR(
          "failed reading RCV1 training set samples batch during svm fit");
      goto done;
    }

    if (verbose) {
      SL_LOG_INFO("RCV1 train batch %zu: apply minmax", batch_idx);
    }
    sl_ml_minmax_apply(&minmax_scaler, &samples_batch, -1, 1);

    if (verbose) {
      SL_LOG_INFO("RCV1 train batch %zu: copy classes", batch_idx);
    }
    for (size_t i = 0; i < SL_SVM_RCV1_BUFFER_LEN; ++i) {
      const size_t class_idx = (size_t)samples_batch.rows * batch_idx + i;
      classes_buffer[i] =
          class_idx < train_classes_record.size ? train_classes[class_idx] : 0;
    }

    if (verbose) {
      SL_LOG_INFO("RCV1 train batch %zu: fit svm", batch_idx);
    }
    sl_ml_svm_linear_fit(&svm, &samples_batch, classes_buffer);

    if (!sl_la_vector_is_finite(&(svm.w))) {
      SL_LOG_ERROR("RCV1 train batch %zu: SVM weights has NaNs", batch_idx);
      goto done;
    }

    if (verbose) {
      SL_LOG_INFO("RCV1 train batch %zu: results", batch_idx);
    }
    struct sl_ml_classification report = {0};
    for (int i = 0; i < SL_SVM_RCV1_BUFFER_LEN; ++i) {
      struct sl_la_vector x = sl_la_matrix_row_view(&samples_batch, i);
      sl_ml_classification_update(&report,
                                  classes_buffer[i],
                                  sl_ml_svm_binary_predict(&svm, &x));
    }
    sl_ml_classification_print(stderr, &report);
  }

  for (size_t batch_idx = 0; !sl_record_reader_is_done(&test_samples_reader);
       ++batch_idx) {
    if (verbose) {
      SL_LOG_INFO("RCV1 test batch %zu: reading buffer", batch_idx);
    }
    sl_span_clear(&read_buffer);
    if (!sl_record_reader_read(&test_samples_reader, &read_buffer)) {
      SL_LOG_ERROR(
          "failed reading RCV1 testing set samples batch during svm "
          "evaluation");
      goto done;
    }

    if (verbose) {
      SL_LOG_INFO("RCV1 test batch %zu: apply minmax", batch_idx);
    }
    sl_ml_minmax_apply(&minmax_scaler, &samples_batch, -1, 1);

    if (verbose) {
      SL_LOG_INFO("RCV1 test batch %zu: copy classes", batch_idx);
    }
    for (size_t i = 0; i < SL_SVM_RCV1_BUFFER_LEN; ++i) {
      const size_t class_idx = (size_t)samples_batch.rows * batch_idx + i;
      classes_buffer[i] =
          class_idx < test_classes_record.size ? test_classes[class_idx] : 0;
    }

    if (verbose) {
      SL_LOG_INFO("RCV1 test batch %zu: results", batch_idx);
    }
    struct sl_ml_classification report = {0};
    for (int i = 0; i < SL_SVM_RCV1_BUFFER_LEN; ++i) {
      struct sl_la_vector x = sl_la_matrix_row_view(&samples_batch, i);
      sl_ml_classification_update(&report,
                                  classes_buffer[i],
                                  sl_ml_svm_binary_predict(&svm, &x));
    }
    sl_ml_classification_print(stderr, &report);
  }

  all_ok = true;
done:
  sl_free(train_classes);
  sl_free(test_classes);
  sl_record_reader_close(&train_samples_reader);
  sl_record_reader_close(&test_samples_reader);
  sl_la_matrix_destroy(&samples_batch);
  return all_ok;
}

void print_usage(const struct sl_args args[const static 1]) {
  SL_LOG_ERROR("usage: %s experiment dataset_dir [-v]", args->argv[0]);
}

int main(int argc, char* const argv[argc + 1]) {
  bool ok = false;
  struct sl_args args = {.argc = argc, .argv = argv};
  if (sl_args_count_positional(&args) < 2) {
    SL_LOG_ERROR("incorrect number of arguments");
  } else {
    const char* experiment = sl_args_get_positional(&args, 0);
    if (experiment) {
      if (strcmp(experiment, "spambase") == 0) {
        ok = spambase(&args);
      } else if (strcmp(experiment, "rcv1") == 0) {
        ok = rcv1(&args);
      } else {
        SL_LOG_ERROR("unknown experiment %s", experiment);
      }
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
