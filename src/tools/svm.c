#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_dataset.h"
#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_ml.h"
#include "stufflib_record.h"
#include "stufflib_record_reader.h"

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
      .w = (struct sl_la_vector){.size = samples.cols,
                                 .data =
                                     (float[SL_DATASET_SPAMBASE_FEATURES]){0}},
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

  SL_ML_MINMAX_SCALER_CREATE(sl_minmax_scaler, SL_DATASET_SPAMBASE_FEATURES);
  sl_ml_minmax_fit(&sl_minmax_scaler, &train_data);
  sl_ml_minmax_apply(&sl_minmax_scaler, &train_data, -1, 1);
  sl_ml_minmax_apply(&sl_minmax_scaler, &test_data, -1, 1);

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
