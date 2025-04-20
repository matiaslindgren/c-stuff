#include "stufflib/dataset/dataset.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib/args/args.h"
#include "stufflib/filesystem/filesystem.h"
#include "stufflib/io/io.h"
#include "stufflib/linalg/linalg.h"
#include "stufflib/macros/macros.h"
#include "stufflib/memory/memory.h"
#include "stufflib/misc/misc.h"
#include "stufflib/png/png.h"
#include "stufflib/record/record.h"
#include "stufflib/record/writer.h"
#include "stufflib/span/span.h"
#include "stufflib/string/string.h"
#include "stufflib/tokenizer/tokenizer.h"

static unsigned char reader_buffer_data[1024 << 6] = {0};
static struct sl_span reader_buffer = {0};

static bool cifar_to_png(const struct sl_args args[const static 1]) {
  // CIFAR dataset parser
  // Data format reference: https://www.cs.toronto.edu/~kriz/cifar.html
  // (accessed 2024-06-01)
  if (sl_args_count_positional(args) != 3) {
    SL_LOG_ERROR("too few arguments to CIFAR -> PNG extractor");
    return false;
  }

  bool all_ok = false;

  const char* const dataset_dir = sl_args_get_positional(args, 1);
  const char* const output_dir = sl_args_get_positional(args, 2);
  const bool verbose = sl_args_parse_flag(args, "-v");

  if (verbose) {
    SL_LOG_INFO("reading CIFAR batches from '%s', writing PNG data into '%s'",
                dataset_dir,
                output_dir);
  }

  char labels[10][256] = {0};

  {
    char filename[256] = {0};
    snprintf(filename,
             SL_ARRAY_LEN(filename),
             "%s/batches.meta.txt",
             dataset_dir);
    if (verbose) {
      SL_LOG_INFO("reading metadata '%s'", filename);
    }
    struct sl_string data = sl_fs_read_file_utf8(filename, &reader_buffer);
    // TODO create iterlines util
    struct sl_span newline = sl_span_view(1, (unsigned char[]){'\n'});
    struct sl_tokenizer newline_tokenizer =
        sl_tokenizer_create(&(data.utf8_data), &newline);
    size_t lineno = 0;
    for (struct sl_iterator iter = sl_tokenizer_iter(&newline_tokenizer);
         lineno < 10 && !sl_tokenizer_iter_is_done(&iter);
         sl_tokenizer_iter_advance(&iter)) {
      struct sl_string line = sl_string_from_utf8(sl_tokenizer_iter_get(&iter));
      if (line.length == 0 || !sl_string_is_ascii(&line)) {
        SL_LOG_ERROR("all metadata lines must be in ASCII");
        sl_string_destroy(&line);
        goto done;
      }
      sl_string_copy_ascii(labels[lineno], &line);
      if (verbose) {
        printf("label %zu: %s\n", lineno, labels[lineno]);
      }
      ++lineno;
      sl_string_destroy(&line);
    }
    sl_string_destroy(&data);
  }

  const char* batch_names[] = {
      "data_batch_1.bin",
      "data_batch_2.bin",
      "data_batch_3.bin",
      "data_batch_4.bin",
      "data_batch_5.bin",
      "test_batch.bin",
  };
  const size_t cifar_batch_size = 30'730'000;

  for (size_t batch_num = 0; batch_num < SL_ARRAY_LEN(batch_names);
       ++batch_num) {
    char filename[256] = {0};
    snprintf(filename,
             SL_ARRAY_LEN(filename),
             "%s/%s",
             dataset_dir,
             batch_names[batch_num]);
    struct sl_span data = sl_fs_read_file(filename, &reader_buffer);
    if (verbose) {
      SL_LOG_INFO("batch '%s' contains '%zu' bytes", filename, data.size);
    }
    if (data.size != cifar_batch_size) {
      SL_LOG_ERROR(
          "all CIFAR batches should contain exactly %zu bytes but batch '%s' "
          "contains %zu bytes",
          cifar_batch_size,
          filename,
          data.size);
      goto invalid_batch;
    }

    for (size_t sample_num = 0; sample_num < 10'000; ++sample_num) {
      struct sl_png_image img = sl_png_image_rgb_create(32, 32);
      const size_t sample_begin = sample_num * (1 + 3 * 1024);

      unsigned char label = data.data[sample_begin];
      if (label > 9) {
        SL_LOG_ERROR("labels of every CIFAR sample must be between 0 and 9");
        goto invalid_sample;
      }

      for (size_t row = 0; row < img.header.height; ++row) {
        for (size_t col = 0; col < img.header.width; ++col) {
          const size_t j = row * img.header.width + col;
          unsigned char px[] = {
              data.data[sample_begin + 1 + 0 * 1024 + j],
              data.data[sample_begin + 1 + 1 * 1024 + j],
              data.data[sample_begin + 1 + 2 * 1024 + j],
          };
          sl_png_image_set_pixel(&img, row + 1, col + 1, px);
        }
      }

      char outname[200] = {0};
      snprintf(outname,
               SL_ARRAY_LEN(outname),
               "%s/batch%zu_sample%04zu_%s.png",
               output_dir,
               batch_num,
               sample_num,
               labels[label]);
      if (verbose) {
        SL_LOG_INFO("writing sample %s", outname);
      }
      if (!sl_png_write_image(img, outname)) {
        goto invalid_sample;
      }

      sl_png_image_destroy(img);
      continue;
    invalid_sample:
      sl_png_image_destroy(img);
      goto invalid_batch;
    }

    sl_span_destroy(&data);
    continue;
  invalid_batch:
    sl_span_destroy(&data);
    goto done;
  }

  all_ok = true;

done:
  return all_ok;
}

bool spambase(const struct sl_args args[const static 1]) {
  // spambase dataset parser
  // Data format reference: https://archive.ics.uci.edu/dataset/94/spambase
  // (accessed 2024-06-09)
  if (sl_args_count_positional(args) != 3) {
    SL_LOG_ERROR("too few arguments to spambase extractor");
    return false;
  }

  bool all_ok = false;

  const char* const dataset_dir = sl_args_get_positional(args, 1);
  const char* const output_dir = sl_args_get_positional(args, 2);
  const bool verbose = sl_args_parse_flag(args, "-v");

  if (verbose) {
    SL_LOG_INFO("reading spambase dataset from '%s', writing dataset to '%s'",
                dataset_dir,
                output_dir);
  }

  struct sl_la_matrix data = sl_la_matrix_create(SL_DATASET_SPAMBASE_SAMPLES,
                                                 SL_DATASET_SPAMBASE_FEATURES);
  uint16_t classes[SL_DATASET_SPAMBASE_SAMPLES] = {0};

  {
    const char* filename = "spambase.data";
    char path[1024] = {0};
    if (!sl_file_format_path(SL_ARRAY_LEN(path),
                             path,
                             dataset_dir,
                             filename,
                             "")) {
      SL_LOG_ERROR("failed formatting path '%s/%s'", dataset_dir, filename);
      goto done;
    }

    if (verbose) {
      SL_LOG_INFO("reading csv file '%s'", path);
    }

    struct sl_string content = sl_fs_read_file_utf8(path, &reader_buffer);
    // TODO create iterlines util
    struct sl_span newline = sl_span_view(1, (unsigned char[]){'\n'});
    struct sl_tokenizer newline_tokenizer =
        sl_tokenizer_create(&(content.utf8_data), &newline);

    int lineno = 0;
    for (struct sl_iterator iter = sl_tokenizer_iter(&newline_tokenizer);
         lineno < data.rows && !sl_tokenizer_iter_is_done(&iter);
         sl_tokenizer_iter_advance(&iter)) {
      struct sl_span* line = sl_tokenizer_iter_get(&iter);
      if (line->size < 2) {
        // TODO proper generic iterlines util that skips CRLF
        continue;
      }

      int label = 0;
      // TODO maybe strof on char file reader buffer
      if (EOF == sscanf((const char*)line->data,
                        ("%f" SL_REPEAT_56(",%f") ",%d"),
                        sl_la_matrix_get(&data, lineno, 0),
                        sl_la_matrix_get(&data, lineno, 1),
                        sl_la_matrix_get(&data, lineno, 2),
                        sl_la_matrix_get(&data, lineno, 3),
                        sl_la_matrix_get(&data, lineno, 4),
                        sl_la_matrix_get(&data, lineno, 5),
                        sl_la_matrix_get(&data, lineno, 6),
                        sl_la_matrix_get(&data, lineno, 7),
                        sl_la_matrix_get(&data, lineno, 8),
                        sl_la_matrix_get(&data, lineno, 9),
                        sl_la_matrix_get(&data, lineno, 10),
                        sl_la_matrix_get(&data, lineno, 11),
                        sl_la_matrix_get(&data, lineno, 12),
                        sl_la_matrix_get(&data, lineno, 13),
                        sl_la_matrix_get(&data, lineno, 14),
                        sl_la_matrix_get(&data, lineno, 15),
                        sl_la_matrix_get(&data, lineno, 16),
                        sl_la_matrix_get(&data, lineno, 17),
                        sl_la_matrix_get(&data, lineno, 18),
                        sl_la_matrix_get(&data, lineno, 19),
                        sl_la_matrix_get(&data, lineno, 20),
                        sl_la_matrix_get(&data, lineno, 21),
                        sl_la_matrix_get(&data, lineno, 22),
                        sl_la_matrix_get(&data, lineno, 23),
                        sl_la_matrix_get(&data, lineno, 24),
                        sl_la_matrix_get(&data, lineno, 25),
                        sl_la_matrix_get(&data, lineno, 26),
                        sl_la_matrix_get(&data, lineno, 27),
                        sl_la_matrix_get(&data, lineno, 28),
                        sl_la_matrix_get(&data, lineno, 29),
                        sl_la_matrix_get(&data, lineno, 30),
                        sl_la_matrix_get(&data, lineno, 31),
                        sl_la_matrix_get(&data, lineno, 32),
                        sl_la_matrix_get(&data, lineno, 33),
                        sl_la_matrix_get(&data, lineno, 34),
                        sl_la_matrix_get(&data, lineno, 35),
                        sl_la_matrix_get(&data, lineno, 36),
                        sl_la_matrix_get(&data, lineno, 37),
                        sl_la_matrix_get(&data, lineno, 38),
                        sl_la_matrix_get(&data, lineno, 39),
                        sl_la_matrix_get(&data, lineno, 40),
                        sl_la_matrix_get(&data, lineno, 41),
                        sl_la_matrix_get(&data, lineno, 42),
                        sl_la_matrix_get(&data, lineno, 43),
                        sl_la_matrix_get(&data, lineno, 44),
                        sl_la_matrix_get(&data, lineno, 45),
                        sl_la_matrix_get(&data, lineno, 46),
                        sl_la_matrix_get(&data, lineno, 47),
                        sl_la_matrix_get(&data, lineno, 48),
                        sl_la_matrix_get(&data, lineno, 49),
                        sl_la_matrix_get(&data, lineno, 50),
                        sl_la_matrix_get(&data, lineno, 51),
                        sl_la_matrix_get(&data, lineno, 52),
                        sl_la_matrix_get(&data, lineno, 53),
                        sl_la_matrix_get(&data, lineno, 54),
                        sl_la_matrix_get(&data, lineno, 55),
                        sl_la_matrix_get(&data, lineno, 56),
                        &label)) {
        SL_LOG_ERROR("failed parsing CSV line %d", lineno);
        break;
      }
      classes[lineno] = label > 0;
      ++lineno;
    }

    sl_string_destroy(&content);

    if (lineno != data.rows) {
      SL_LOG_ERROR("expected %d samples but parsed %d", data.rows, lineno);
      goto done;
    }
  }

  const size_t n_rows = (size_t)data.rows;
  const size_t n_cols = (size_t)data.cols;

  struct sl_record record_samples = {
      .layout = "sparse",
      .type = "float32",
      .name = "spambase_samples",
      .size = sl_misc_count_nonzero(sizeof(float),
                                    n_rows * n_cols,
                                    (void*)data.data),
      .n_dims = 2,
      .dim_size = {n_rows, n_cols},
  };
  strcpy(record_samples.path, output_dir);
  if (!(sl_record_write_metadata(&record_samples) &&
        sl_record_write_all(&record_samples,
                            sizeof(float) * n_rows * n_cols,
                            (void*)(data.data)))) {
    SL_LOG_ERROR("failed writing spambase dataset samples to '%s'", output_dir);
    goto done;
  }

  struct sl_record record_classes = {
      .layout = "dense",
      .type = "uint16",
      .name = "spambase_classes",
      .size = SL_ARRAY_LEN(classes),
      .n_dims = 1,
      .dim_size = {n_rows},
  };
  strcpy(record_classes.path, output_dir);
  if (!(sl_record_write_metadata(&record_classes) &&
        sl_record_write_all(&record_classes,
                            sizeof(classes),
                            (void*)classes))) {
    SL_LOG_ERROR("failed writing spambase dataset classes to '%s'", output_dir);
    goto done;
  }

  all_ok = true;
done:
  sl_la_matrix_destroy(&data);
  return all_ok;
}

// RCV1: A New Benchmark Collection for Text Categorization Research.
// David D. Lewis et al.
// https://jmlr.csail.mit.edu/papers/volume5/lewis04a/lewis04a.pdf
// 2024-06-22
//
// http://www.ai.mit.edu/projects/jmlr/papers/volume5/lewis04a/lyrl2004_rcv1v2_README.htm
// 2024-06-23
bool rcv1(const struct sl_args args[const static 1]) {
  if (sl_args_count_positional(args) != 3) {
    SL_LOG_ERROR("too few arguments to RCV1 extractor");
    return false;
  }

  // TODO way too much code
  bool all_ok = false;

  const char* const dataset_dir = sl_args_get_positional(args, 1);
  const char* const output_dir = sl_args_get_positional(args, 2);
  const bool verbose = sl_args_parse_flag(args, "-v");

  if (verbose) {
    SL_LOG_INFO(
        "reading RCV1 dataset with dimensions %d x %d from '%s', writing "
        "dataset to '%s'",
        SL_DATASET_RCV1_SAMPLES,
        SL_DATASET_RCV1_FEATURES,
        dataset_dir,
        output_dir);
  }

  struct sl_rcv1_metadata {
    size_t index;
    bool in_trainset;
    bool is_ccat_class;
    bool has_value;
  };

  const size_t max_document_id = 2 * SL_DATASET_RCV1_SAMPLES;

  FILE* fp = nullptr;

  struct sl_rcv1_metadata* metadata =
      sl_alloc(max_document_id + 1, sizeof(struct sl_rcv1_metadata));

  struct sl_la_vector batch =
      SL_LA_VECTOR_CREATE_INLINE(SL_DATASET_RCV1_FEATURES);

  struct sl_record train_record = {0};
  struct sl_file train_record_file = {0};
  struct sl_record_writer trainset_writer = {0};

  struct sl_record test_record = {0};
  struct sl_file test_record_file = {0};
  struct sl_record_writer testset_writer = {0};

  {
    const char* filename = "rcv1v2-ids.dat";
    char path[1024] = {0};
    if (!sl_file_format_path(SL_ARRAY_LEN(path),
                             path,
                             dataset_dir,
                             filename,
                             "")) {
      SL_LOG_ERROR("failed formatting path '%s/%s'", dataset_dir, filename);
      goto done;
    }

    fp = fopen(path, "r");
    if (!fp) {
      SL_LOG_ERROR("cannot open file '%s' for reading", path);
      goto done;
    }

    if (verbose) {
      SL_LOG_INFO("reading RCV1 document IDs from '%s'", path);
    }

    for (size_t id = 0, idx = 0; EOF != fscanf(fp, "%zu ", &id); ++idx) {
      if (id >= max_document_id) {
        SL_LOG_ERROR("unexpectedly large RCV1 document ID %zu at index %zu",
                     id,
                     idx);
        goto done;
      }
      if (metadata[id].has_value) {
        SL_LOG_ERROR("duplicate RCV1 document ID %zu at index %zu", id, idx);
        goto done;
      }
      metadata[id] = (struct sl_rcv1_metadata){
          .index = idx,
          .has_value = true,
      };
    }

    fclose(fp);
    fp = nullptr;
  }

  {
    const char* filename = "rcv1-v2.topics.qrels";
    char path[1024] = {0};
    if (!sl_file_format_path(SL_ARRAY_LEN(path),
                             path,
                             dataset_dir,
                             filename,
                             "")) {
      SL_LOG_ERROR("failed formatting path '%s/%s'", dataset_dir, filename);
      goto done;
    }

    fp = fopen(path, "r");
    if (!fp) {
      SL_LOG_ERROR("cannot open file '%s' for reading", path);
      goto done;
    }

    if (verbose) {
      SL_LOG_INFO("reading RCV1 topics from '%s'", path);
    }

    {
      char category[16] = {0};
      size_t id = 0;
      while (EOF != fscanf(fp, "%15s %zu %*d ", category, &id)) {
        if (id >= max_document_id) {
          SL_LOG_ERROR(
              "unexpectedly large RCV1 document ID %zu while parsing topic "
              "category",
              id);
          goto done;
        }
        metadata[id].is_ccat_class = SL_STR_EQ(category, "CCAT");
      }
    }

    fclose(fp);
    fp = nullptr;
  }

  train_record = (struct sl_record){
      .layout = "sparse",
      .type = "float32",
      .name = "rcv1_train_samples",
      .n_dims = 2,
      .dim_size = {0, SL_DATASET_RCV1_FEATURES},
  };
  strcpy(train_record.path, output_dir);
  train_record_file = (struct sl_file){0};
  trainset_writer = (struct sl_record_writer){
      .file = &train_record_file,
      .record = &train_record,
  };

  test_record = (struct sl_record){
      .layout = "sparse",
      .type = "float32",
      .name = "rcv1_test_samples",
      .n_dims = 2,
      .dim_size = {0, SL_DATASET_RCV1_FEATURES},
  };
  strcpy(test_record.path, output_dir);
  test_record_file = (struct sl_file){0};
  testset_writer = (struct sl_record_writer){
      .file = &test_record_file,
      .record = &test_record,
  };

  if (!sl_record_writer_open(&trainset_writer)) {
    SL_LOG_ERROR("cannot open training set samples record writer");
    goto done;
  }
  if (!sl_record_writer_open(&testset_writer)) {
    SL_LOG_ERROR("cannot open test set samples record writer");
    goto done;
  }

  const char* batch_names[] = {
      "lyrl2004_vectors_test_pt0",
      "lyrl2004_vectors_test_pt1",
      "lyrl2004_vectors_test_pt2",
      "lyrl2004_vectors_test_pt3",
      "lyrl2004_vectors_train",
  };

  for (size_t batch_idx = 0; batch_idx < SL_ARRAY_LEN(batch_names);
       ++batch_idx) {
    const char* batch_name = batch_names[batch_idx];
    char path[1024] = {0};
    if (!sl_file_format_path(SL_ARRAY_LEN(path),
                             path,
                             dataset_dir,
                             batch_name,
                             ".dat")) {
      SL_LOG_ERROR("failed formatting path '%s/%s.dat'",
                   dataset_dir,
                   batch_name);
      goto done;
    }

    fp = fopen(path, "r");
    if (!fp) {
      SL_LOG_ERROR("cannot open file '%s' for reading", path);
      goto done;
    }

    if (verbose) {
      SL_LOG_INFO("reading RCV1 vectors from '%s'", path);
    }

    size_t lineno = 0;

    for (char line[32768] = {0}, newline = 0;
         EOF != fscanf(fp, "%32767[^\n]%c", line, &newline);
         ++lineno, newline = 0) {
      if (newline && newline != '\n') {
        SL_LOG_ERROR(
            "RCV1 file '%s' lineno %zu: too long line for line buffer of "
            "length %zu",
            path,
            lineno,
            SL_ARRAY_LEN(line));
        goto done;
      }

      char* lhs = line;
      char* rhs = nullptr;

      unsigned long id = strtoul(lhs, &rhs, 10);
      if (id >= max_document_id || !metadata[id].has_value) {
        SL_LOG_ERROR("RCV1 file '%s' lineno %zu: unknown document with ID %zu",
                     path,
                     lineno,
                     id);
        goto done;
      }

      metadata[id].in_trainset =
          SL_STR_EQ(batch_name, "lyrl2004_vectors_train");

      if (verbose && lineno % 10'000 == 0) {
        SL_LOG_INFO(
            "RCV1 file '%s' lineno %zu: document ID %zu sample index %zu",
            path,
            lineno,
            id,
            metadata[id].index);
      }

      sl_la_vector_clear(&batch);

      for (lhs = rhs; lhs && lhs[0];) {
        unsigned long feature_id = strtoul(lhs, &rhs, 10);
        if (lhs == rhs || errno == ERANGE || feature_id == 0 ||
            feature_id > SL_DATASET_RCV1_FEATURES) {
          SL_LOG_ERROR(
              "RCV1 file '%s' lineno %zu: cannot parse invalid feature ID",
              path,
              lineno);
          errno = 0;
          goto done;
        }

        lhs = rhs;
        if (lhs[0] != ':') {
          SL_LOG_ERROR(
              "RCV1 file '%s' lineno %zu: expected ':' after feature ID %lu",
              path,
              lineno,
              feature_id);
          goto done;
        }
        ++lhs;

        float value = strtof(lhs, &rhs);
        if (lhs == rhs || errno == ERANGE) {
          SL_LOG_ERROR(
              "RCV1 file '%s' lineno %zu feature %lu: cannot parse float",
              path,
              lineno,
              feature_id);
          errno = 0;
          goto done;
        }
        if (value < 0 || value > 1) {
          SL_LOG_ERROR(
              "RCV1 file '%s' lineno %zu feature %lu value %g not in [0, 1]",
              path,
              lineno,
              feature_id,
              (double)value);
          errno = 0;
          goto done;
        }
        batch.data[feature_id - 1] = value;

        lhs = rhs;
      }

      struct sl_span write_buffer =
          sl_span_view(sizeof(float) * (size_t)(batch.size),
                       (void*)(batch.data));

      if (metadata[id].in_trainset) {
        // TODO len N buffer instead of len 1
        train_record.dim_size[0] += 1;
        if (!sl_record_writer_write(&trainset_writer, &write_buffer)) {
          SL_LOG_ERROR(
              "RCV1 file '%s' lineno %zu: failed training set writing sample",
              path,
              lineno);
          goto done;
        }
      } else {
        test_record.dim_size[0] += 1;
        if (!sl_record_writer_write(&testset_writer, &write_buffer)) {
          SL_LOG_ERROR(
              "RCV1 file '%s' lineno %zu: failed writing test set sample",
              path,
              lineno);
          goto done;
        }
      }
    }

    fclose(fp);
    fp = nullptr;
  }

  train_record.size = trainset_writer.n_written;
  test_record.size = testset_writer.n_written;

  if (!sl_record_write_metadata(&train_record)) {
    SL_LOG_ERROR("failed writing RCV1 metadata for training set samples");
    goto done;
  }
  if (!sl_record_write_metadata(&test_record)) {
    SL_LOG_ERROR("failed writing RCV1 metadata for test set samples");
    goto done;
  }

  sl_record_writer_close(&trainset_writer);
  sl_record_writer_close(&testset_writer);

  train_record = (struct sl_record){
      .layout = "dense",
      .type = "uint8",
      .name = "rcv1_train_classes",
      .size = 1,
      .n_dims = 1,
      .dim_size = {0},
  };
  strcpy(train_record.path, output_dir);
  train_record_file = (struct sl_file){0};
  trainset_writer = (struct sl_record_writer){
      .file = &train_record_file,
      .record = &train_record,
  };

  test_record = (struct sl_record){
      .layout = "dense",
      .type = "uint8",
      .name = "rcv1_test_classes",
      .size = 1,
      .n_dims = 1,
      .dim_size = {0},
  };
  strcpy(test_record.path, output_dir);
  test_record_file = (struct sl_file){0};
  testset_writer = (struct sl_record_writer){
      .file = &test_record_file,
      .record = &test_record,
  };

  if (!sl_record_writer_open(&trainset_writer)) {
    SL_LOG_ERROR("cannot open training set classes record writer");
    goto done;
  }
  if (!sl_record_writer_open(&testset_writer)) {
    SL_LOG_ERROR("cannot open test set classes record writer");
    goto done;
  }

  struct sl_span class_buffer = {.size = 1, .data = (uint8_t[1]){0}};
  for (size_t id = 0; id <= max_document_id; ++id) {
    if (metadata[id].has_value) {
      class_buffer.data[0] = metadata[id].is_ccat_class;
      if (metadata[id].in_trainset) {
        if (!sl_record_writer_write(&trainset_writer, &class_buffer)) {
          SL_LOG_ERROR("RCV1 document %zu: failed writing training set class",
                       id);
          goto done;
        }
      } else {
        if (!sl_record_writer_write(&testset_writer, &class_buffer)) {
          SL_LOG_ERROR("RCV1 document %zu: failed writing test set class", id);
          goto done;
        }
      }
    }
  }

  train_record.size = train_record.dim_size[0] = trainset_writer.n_written;
  test_record.size = test_record.dim_size[0] = testset_writer.n_written;

  if (!sl_record_write_metadata(&train_record)) {
    SL_LOG_ERROR("failed writing RCV1 metadata for training set classes");
    goto done;
  }
  if (!sl_record_write_metadata(&test_record)) {
    SL_LOG_ERROR("failed writing RCV1 metadata for test set classes");
    goto done;
  }

  all_ok = true;
done:
  if (fp) {
    fclose(fp);
  }
  sl_free(metadata);
  sl_record_writer_close(&trainset_writer);
  sl_record_writer_close(&testset_writer);
  return all_ok;
}

void print_usage(const struct sl_args args[const static 1]) {
  SL_LOG_ERROR(("usage: %s cifar_to_png dataset_path output_path [-v]"
                "usage: %s spambase dataset_path output_path [-v]"
                "usage: %s rcv1 dataset_path output_path [-v]"),
               args->argv[0],
               args->argv[0],
               args->argv[0]);
}

int main(int argc, char* const argv[argc + 1]) {
  struct sl_args args = {.argc = argc, .argv = argv};
  reader_buffer =
      sl_span_view(SL_ARRAY_LEN(reader_buffer_data), reader_buffer_data);
  bool ok = false;
  const char* command = sl_args_get_positional(&args, 0);
  if (command) {
    if (strcmp(command, "cifar_to_png") == 0) {
      ok = cifar_to_png(&args);
    } else if (strcmp(command, "spambase") == 0) {
      ok = spambase(&args);
    } else if (strcmp(command, "rcv1") == 0) {
      ok = rcv1(&args);
    } else {
      SL_LOG_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
