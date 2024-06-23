// TODO
#if 0
bool spambase(const struct sl_args args[const static 1]) {
  struct sl_ds_dataset dataset = {
      .type = "dense",
      .name = "spambase",
      .size = (size_t)data.rows * (size_t)data.cols,
      .n_dims = 3,
  };
  strcpy(dataset.path, output_dir);
  dataset.dim_size[0] = 1;
  dataset.dim_size[1] = (size_t)data.rows;
  dataset.dim_size[2] = (size_t)data.cols;

  struct sl_la_matrix data = sl_la_matrix_create(4601, 57);
  int classes[4601] = {0};

  struct sl_ml_svm svm = {
      .w = (struct sl_la_vector){.size = data.cols, .data = (float[57]){0}},
      .batch_size = 1,
      .n_epochs = 2,
      .learning_rate = 1e-9f,
  };

  struct sl_la_matrix test_data = {
      .rows = 2000,
      .cols = data.cols,
  };
  int test_classes[2000] = {0};

  struct sl_la_matrix train_data = {
      .rows = data.rows - test_data.rows,
      .cols = data.cols,
  };
  int train_classes[SL_ARRAY_LEN(classes) - SL_ARRAY_LEN(test_classes)] = {0};

  sl_ml_random_train_test_split(&data,
                                &train_data,
                                &test_data,
                                classes,
                                train_classes,
                                test_classes);

  sl_ml_minmax_rescale(&train_data, -1, 1);
  sl_ml_svm_linear_fit(&svm, &train_data, train_classes);

  {
    struct sl_ml_classification report = {0};
    for (int i = 0; i < train_data.rows; ++i) {
      struct sl_la_vector x = sl_la_matrix_row_view(&train_data, i);
      sl_ml_classification_update(&report,
                                  train_classes[i],
                                  sl_ml_svm_predict(&svm, &x));
    }
    SL_LOG_INFO("spambase dataset, random train set, linear SVM");
    sl_ml_classification_print(&report);
  }
  {
    struct sl_ml_classification report = {0};
    for (int i = 0; i < test_data.rows; ++i) {
      struct sl_la_vector x = sl_la_matrix_row_view(&test_data, i);
      sl_ml_classification_update(&report,
                                  test_classes[i],
                                  sl_ml_svm_predict(&svm, &x));
    }
    SL_LOG_INFO("spambase dataset, random test set, linear SVM");
    sl_ml_classification_print(&report);
  }

  sl_la_matrix_destroy(&data);

  return all_ok;
}

#endif

int main() { return 0; }
