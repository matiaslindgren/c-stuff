#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SL_FILE_BUFFER_CAPACITY (1024 << 10)

#include "stufflib_args.h"
#include "stufflib_dataset.h"
#include "stufflib_filesystem.h"
#include "stufflib_io.h"
#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_ml.h"
#include "stufflib_png.h"

bool cifar_to_png(const struct sl_args args[const static 1]) {
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
    struct sl_string data = sl_fs_read_file_utf8(filename);
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
        sl_string_delete(&line);
        goto done;
      }
      sl_string_copy_ascii(labels[lineno], &line);
      if (verbose) {
        printf("label %zu: %s\n", lineno, labels[lineno]);
      }
      ++lineno;
      sl_string_delete(&line);
    }
    sl_string_delete(&data);
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
    struct sl_span data = sl_fs_read_file(filename);
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

    sl_span_delete(&data);
    continue;
  invalid_batch:
    sl_span_delete(&data);
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

  struct sl_la_matrix data = sl_la_matrix_create(4601, 57);
  int classes[4601] = {0};

  {
    char filename[256] = {0};
    snprintf(filename, SL_ARRAY_LEN(filename), "%s/spambase.data", dataset_dir);
    if (verbose) {
      SL_LOG_INFO("reading csv file '%s'", filename);
    }

    struct sl_string content = sl_fs_read_file_utf8(filename);
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

    sl_string_delete(&content);

    if (lineno != data.rows) {
      SL_LOG_ERROR("expected %d samples but parsed %d", data.rows, lineno);
      goto done;
    }
  }

  const size_t n_rows = (size_t)data.rows;
  const size_t n_cols = (size_t)data.cols;

  struct sl_ds_dataset dataset = {
      .type = "sparse",
      .name = "spambase",
      .n_dims = 2,
  };
  strcpy(dataset.path, output_dir);
  dataset.dim_size[0] = n_rows;
  dataset.dim_size[1] = n_cols;

  if (!sl_ds_append(&dataset, &data) || !sl_ds_finalize(&dataset)) {
    SL_LOG_ERROR("failed writing spambase dataset to %s", output_dir);
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

  bool all_ok = false;

  const char* const dataset_dir = sl_args_get_positional(args, 1);
  const char* const output_dir = sl_args_get_positional(args, 2);
  const bool verbose = sl_args_parse_flag(args, "-v");

  if (verbose) {
    SL_LOG_INFO("reading RCV1 dataset from '%s', writing dataset to '%s'",
                dataset_dir,
                output_dir);
  }

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
