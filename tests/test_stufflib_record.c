#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/io/io.h>
#include <stufflib/linalg/linalg.h>
#include <stufflib/macros/macros.h>
#include <stufflib/math/math.h>
#include <stufflib/memory/memory.h>
#include <stufflib/misc/misc.h>
#include <stufflib/record/reader.h>
#include <stufflib/record/record.h>
#include <stufflib/record/writer.h>
#include <stufflib/span/span.h>
#include <stufflib/testing/testing.h>

bool contains(
    const char dir[static const 1],
    const char name[static const 1],
    const size_t count,
    unsigned char expected[static const 1]
) {
  char path[200] = {0};
  {
    int ok = snprintf(path, SL_ARRAY_LEN(path), "%s/%s", dir, name);
    SL_ASSERT_TRUE(ok > 0);
  }

  FILE* fp = fopen(path, "rb");
  SL_ASSERT_TRUE(fp != nullptr);

  struct sl_span buffer = {
      .size = 1'024,
      .data = (unsigned char[1'024]){0},
  };

  bool yes = false;

  for (size_t i = 0; i < (count + buffer.size - 1) / buffer.size; ++i) {
    const size_t n_left = SL_MIN(buffer.size, count - i * buffer.size);
    if (n_left) {
      const size_t n_read = fread(buffer.data, 1, n_left, fp);
      if (n_left != n_read || ferror(fp) != 0
          || memcmp(buffer.data, expected + i * buffer.size, n_read) != 0) {
        goto done;
      }
    }
  }

  yes = true;
done:
  fclose(fp);
  return yes;
}

bool contains_str(
    const char dir[static const 1],
    const char name[static const 1],
    char expected[static const 1]
) {
  return contains(dir, name, strlen(expected), (unsigned char*)expected);
}

SL_TEST(test_write_metadata) {
  {
    struct sl_record record = {
        .layout   = "dense",
        .type     = "float32",
        .name     = "small1",
        .size     = 3,
        .n_dims   = 1,
        .dim_size = {3},
    };
    strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
    record.path[sizeof(record.path) - 1] = '\0';
    SL_ASSERT_TRUE(sl_record_write_metadata(ctx, &record));
    SL_ASSERT_TRUE(contains_str(
        sl_misc_tmpdir(),
        "small1.sl_record_meta",
        ("name: small1\n"
         "type: float32\n"
         "layout: dense\n"
         "size: 3\n"
         "dims: 1\n"
         "dim0: 3\n")
    ));
  }
  {
    struct sl_record record = {
        .layout   = "dense",
        .type     = "float32",
        .name     = "small2",
        .size     = 3,
        .n_dims   = 3,
        .dim_size = {1, 1, 1},
    };
    strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
    record.path[sizeof(record.path) - 1] = '\0';
    SL_ASSERT_TRUE(sl_record_write_metadata(ctx, &record));
    SL_ASSERT_TRUE(contains_str(
        sl_misc_tmpdir(),
        "small2.sl_record_meta",
        ("name: small2\n"
         "type: float32\n"
         "layout: dense\n"
         "size: 3\n"
         "dims: 3\n"
         "dim0: 1\n"
         "dim1: 1\n"
         "dim2: 1\n")
    ));
  }
  {
    struct sl_record record = {
        .layout   = "dense",
        .type     = "float32",
        .name     = "large1",
        .size     = 10'000'000,
        .n_dims   = 4,
        .dim_size = {10, 15'625, 3'125, 64},
    };
    strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
    record.path[sizeof(record.path) - 1] = '\0';
    SL_ASSERT_TRUE(sl_record_write_metadata(ctx, &record));
    SL_ASSERT_TRUE(contains_str(
        sl_misc_tmpdir(),
        "large1.sl_record_meta",
        ("name: large1\n"
         "type: float32\n"
         "layout: dense\n"
         "size: 10000000\n"
         "dims: 4\n"
         "dim0: 10\n"
         "dim1: 15625\n"
         "dim2: 3125\n"
         "dim3: 64\n")
    ));
  }
  return true;
}

SL_TEST(test_read_metadata) {
  {
    struct sl_record record;
    SL_ASSERT_TRUE(sl_record_read_metadata(ctx, &record, "./test-data/record", "small1"));
    SL_ASSERT_TRUE(record.path);
    SL_ASSERT_TRUE(record.name);
    SL_ASSERT_TRUE(record.type);
    SL_ASSERT_TRUE(record.layout);
    SL_ASSERT_EQ_STR(record.path, "./test-data/record");
    SL_ASSERT_EQ_STR(record.name, "small1");
    SL_ASSERT_EQ_STR(record.type, "float32");
    SL_ASSERT_EQ_STR(record.layout, "dense");
    SL_ASSERT_EQ_LL(record.size, 3);
    SL_ASSERT_EQ_LL(record.n_dims, 1);
    SL_ASSERT_TRUE(record.dim_size);
    SL_ASSERT_EQ_LL(record.dim_size[0], 3);
  }
  {
    struct sl_record record;
    SL_ASSERT_TRUE(sl_record_read_metadata(ctx, &record, "./test-data/record", "small2"));
    SL_ASSERT_TRUE(record.path);
    SL_ASSERT_TRUE(record.name);
    SL_ASSERT_TRUE(record.type);
    SL_ASSERT_TRUE(record.layout);
    SL_ASSERT_EQ_STR(record.path, "./test-data/record");
    SL_ASSERT_EQ_STR(record.name, "small2");
    SL_ASSERT_EQ_STR(record.type, "float32");
    SL_ASSERT_EQ_STR(record.layout, "dense");
    SL_ASSERT_EQ_LL(record.size, 3);
    SL_ASSERT_EQ_LL(record.n_dims, 3);
    SL_ASSERT_TRUE(record.dim_size);
    SL_ASSERT_TRUE(record.dim_size + 1);
    SL_ASSERT_TRUE(record.dim_size + 2);
    SL_ASSERT_EQ_LL(record.dim_size[0], 1);
    SL_ASSERT_EQ_LL(record.dim_size[1], 1);
    SL_ASSERT_EQ_LL(record.dim_size[2], 1);
  }
  {
    struct sl_record record;
    SL_ASSERT_TRUE(sl_record_read_metadata(ctx, &record, "./test-data/record", "large1"));
    SL_ASSERT_TRUE(record.path);
    SL_ASSERT_TRUE(record.name);
    SL_ASSERT_TRUE(record.type);
    SL_ASSERT_TRUE(record.layout);
    SL_ASSERT_EQ_STR(record.path, "./test-data/record");
    SL_ASSERT_EQ_STR(record.name, "large1");
    SL_ASSERT_EQ_STR(record.type, "float32");
    SL_ASSERT_EQ_STR(record.layout, "dense");
    SL_ASSERT_EQ_LL(record.size, 10'000'000);
    SL_ASSERT_EQ_LL(record.n_dims, 4);
    SL_ASSERT_TRUE(record.dim_size);
    SL_ASSERT_TRUE(record.dim_size + 1);
    SL_ASSERT_TRUE(record.dim_size + 2);
    SL_ASSERT_TRUE(record.dim_size + 3);
    SL_ASSERT_EQ_LL(record.dim_size[0], 10);
    SL_ASSERT_EQ_LL(record.dim_size[1], 15'625);
    SL_ASSERT_EQ_LL(record.dim_size[2], 3'125);
    SL_ASSERT_EQ_LL(record.dim_size[3], 64);
  }
  {
    struct sl_record record;
    SL_ASSERT_TRUE(sl_record_read_metadata(ctx, &record, "./test-data/record", "large2"));
    SL_ASSERT_TRUE(record.path);
    SL_ASSERT_TRUE(record.name);
    SL_ASSERT_TRUE(record.type);
    SL_ASSERT_TRUE(record.layout);
    SL_ASSERT_EQ_STR(record.path, "./test-data/record");
    SL_ASSERT_EQ_STR(record.name, "large2");
    SL_ASSERT_EQ_STR(record.type, "float32");
    SL_ASSERT_EQ_STR(record.layout, "sparse");
    SL_ASSERT_EQ_LL(record.size, 10);
    SL_ASSERT_EQ_LL(record.n_dims, 3);
    SL_ASSERT_TRUE(record.dim_size);
    SL_ASSERT_TRUE(record.dim_size + 1);
    SL_ASSERT_TRUE(record.dim_size + 2);
    SL_ASSERT_EQ_LL(record.dim_size[0], 1'000);
    SL_ASSERT_EQ_LL(record.dim_size[1], 2'000);
    SL_ASSERT_EQ_LL(record.dim_size[2], 3'000);
  }
  return true;
}

SL_TEST(test_dense_data_reader) {
  struct sl_record record = {0};
  SL_ASSERT_TRUE(sl_record_read_metadata(ctx, &record, "./test-data/record", "large3"));

  const size_t batches = record.dim_size[0];
  const size_t rows    = record.dim_size[1];
  const size_t cols    = record.dim_size[2];
  SL_ASSERT_EQ_LL(batches, 512);
  SL_ASSERT_EQ_LL(rows, 32);
  SL_ASSERT_EQ_LL(cols, 4);
  SL_ASSERT_EQ_STR(record.type, "float32");
  SL_ASSERT_EQ_STR(record.layout, "dense");

  struct sl_file file            = {0};
  struct sl_record_reader reader = {
      .file   = &file,
      .record = &record,
  };

  SL_ASSERT_TRUE(sl_record_reader_open(ctx, &reader));

  struct sl_matrix_f32 batch = {
      .length   = {rows, cols},
      .capacity = {rows, cols},
      .data     = (float[32 * 4]){0},
  };
  const size_t buf_size = sizeof(batch.data[0]) * rows * cols;
  // TODO combine matrix and span with generic tensor
  struct sl_span buffer = sl_span_view(buf_size, (void*)(batch.data));

  for (size_t i = 0; i < batches; ++i) {
    SL_ASSERT_TRUE(!sl_record_reader_is_done(ctx, &reader));

    SL_ASSERT_EQ_LL(sl_record_reader_ftell(&reader), i * buf_size);
    SL_ASSERT_TRUE(sl_record_reader_read(ctx, &reader, &buffer));
    SL_ASSERT_EQ_LL(sl_record_reader_ftell(&reader), (i + 1) * buf_size);

    SL_ASSERT_EQ_LL(buffer.size, buf_size);
    SL_ASSERT_EQ_PTR(buffer.data, batch.data);

    for (size_t j = 0; j < rows; ++j) {
      for (size_t k = 0; k < cols; ++k) {
        const size_t idx      = i * rows * cols + j * cols + k;
        const double expected = sqrt((double)idx + 1);
        const double result   = (double)*sl_matrix_f32_get(&batch, j, k);
        SL_ASSERT_EQ_DOUBLE(result, expected, 1e-5);
      }
    }
  }

  SL_ASSERT_TRUE(sl_record_reader_is_done(ctx, &reader));
  SL_ASSERT_EQ_LL(sl_record_reader_ftell(&reader), sizeof(batch.data[0]) * record.size);

  sl_record_reader_close(&reader);
  return true;
}

SL_TEST(test_sparse_data_reader) {
  int64_t nonzero_index[100]  = {0};
  int64_t nonzero_values[100] = {0};
  const size_t nonzero_count  = SL_ARRAY_LEN(nonzero_index);
  {
    struct sl_file file = {0};
    SL_ASSERT_TRUE(sl_file_open(ctx, &file, "./test-data/record/large4.index.txt", "rb"));
    SL_ASSERT_EQ_LL(sl_file_parse_int64(ctx, &file, nonzero_count, nonzero_index), nonzero_count);
    sl_file_close(&file);
  }
  {
    struct sl_file file = {0};
    SL_ASSERT_TRUE(sl_file_open(ctx, &file, "./test-data/record/large4.values.txt", "rb"));
    SL_ASSERT_EQ_LL(sl_file_parse_int64(ctx, &file, nonzero_count, nonzero_values), nonzero_count);
    sl_file_close(&file);
  }

  struct sl_record record = {0};
  SL_ASSERT_TRUE(sl_record_read_metadata(ctx, &record, "./test-data/record", "large4"));

  SL_ASSERT_EQ_STR(record.type, "int64");
  SL_ASSERT_EQ_STR(record.layout, "sparse");
  SL_ASSERT_EQ_LL(record.size, nonzero_count);
  SL_ASSERT_EQ_LL(record.n_dims, 1);
  const size_t dense_size = record.dim_size[0];
  SL_ASSERT_EQ_LL(dense_size, ((size_t)1'024) << 24);
  SL_ASSERT_EQ_LL(sl_record_item_size(&record), 8);

  struct sl_file file            = {0};
  struct sl_record_reader reader = {
      .file   = &file,
      .record = &record,
  };

  SL_ASSERT_TRUE(sl_record_reader_open(ctx, &reader));

  int64_t batch[1'024 << 6] = {0};
  const size_t batch_size   = SL_ARRAY_LEN(batch);
  // TODO combine matrix and span with generic tensor
  struct sl_span buffer     = sl_span_view(sizeof(batch[0]) * batch_size, (void*)batch);

  size_t seen_items = 0;

  for (size_t i = 0; seen_items < nonzero_count;) {
    SL_ASSERT_TRUE(!sl_record_reader_is_done(ctx, &reader));

    if (reader.sparse_offset > 3 * batch_size) {
      const size_t n_skip = reader.sparse_offset / (3 * batch_size);
      reader.sparse_offset -= batch_size * n_skip;
      reader.index += batch_size * n_skip;
      i += n_skip;
      continue;
    }

    SL_ASSERT_TRUE(sl_record_reader_read(ctx, &reader, &buffer));

    SL_ASSERT_EQ_LL(buffer.size, buffer.size);
    SL_ASSERT_EQ_PTR(buffer.data, batch);

    for (size_t j = 0; j < batch_size; ++j) {
      const int64_t idx = (int64_t)(i * batch_size + j);
      if (seen_items < nonzero_count && idx == nonzero_index[seen_items]) {
        SL_ASSERT_EQ_LL(batch[j], nonzero_values[seen_items]);
        ++seen_items;
      } else {
        SL_ASSERT_EQ_LL(batch[j], 0);
      }
    }

    ++i;
  }

  SL_ASSERT_EQ_LL(seen_items, nonzero_count);
  SL_ASSERT_TRUE(sl_record_reader_is_done(ctx, &reader));

  sl_record_reader_close(&reader);
  return true;
}

SL_TEST(test_read_data) {
  {
    struct sl_record record = {
        .layout   = "dense",
        .type     = "float32",
        .name     = "small4",
        .path     = "./test-data/record",
        .size     = 2,
        .n_dims   = 1,
        .dim_size = {2},
    };
    float data[2] = {0};
    SL_ASSERT_TRUE(sl_record_read_all(ctx, &record, sizeof(data), data));
    SL_ASSERT_TRUE(sl_math_double_almost((double)data[0], 5, 1e-5));
    SL_ASSERT_TRUE(sl_math_double_almost((double)data[1], -10, 1e-5));
  }
  {
    struct sl_record record = {
        .layout   = "sparse",
        .type     = "float32",
        .name     = "small3",
        .path     = "./test-data/record",
        .size     = 3,
        .n_dims   = 2,
        .dim_size = {4, 3},
    };
    const float expected[] = {0, 0, 1.2f, 0, 0, 0, 0, -3.4f, 56.78f, 0, 0, 0, 0};
    float data[12]         = {0};
    SL_ASSERT_TRUE(sl_record_read_all(ctx, &record, sizeof(data), data));
    for (size_t i = 0; i < record.size; ++i) {
      SL_ASSERT_TRUE(sl_math_double_almost((double)data[i], (double)expected[i], 1e-5));
    }
  }
  return true;
}

SL_TEST(test_dense_data_writer) {
  struct sl_record record = {
      .layout   = "dense",
      .type     = "float32",
      .name     = "large5",
      .size     = 50'000,
      .n_dims   = 2,
      .dim_size = {2'000, 25},
  };
  strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
  record.path[sizeof(record.path) - 1] = '\0';

  float* dataset = sl_alloc(ctx, record.size, sizeof(float));
  for (size_t i = 0; i < record.size; ++i) {
    dataset[i] = (float)sqrt((double)(i + 1));
  }

  struct sl_file file            = {0};
  struct sl_record_writer writer = {
      .file   = &file,
      .record = &record,
  };

  SL_ASSERT_TRUE(sl_record_writer_open(ctx, &writer));

  const size_t batch_size = 10;
  const size_t rows       = record.dim_size[0];
  const size_t cols       = record.dim_size[1];

  struct sl_matrix_f32 batch = {
      .length   = {batch_size, cols},
      .capacity = {batch_size, cols},
      .data     = (float[10 * 25]){0},
  };
  const size_t buf_size = sizeof(batch.data[0]) * batch_size * cols;
  // TODO combine matrix and span with generic tensor
  struct sl_span buffer = sl_span_view(buf_size, (void*)(batch.data));

  for (size_t i = 0; i < rows / batch_size; ++i) {
    for (size_t j = 0; j < batch_size; ++j) {
      for (size_t k = 0; k < cols; ++k) {
        batch.data[j * cols + k] = dataset[(i * batch_size + j) * cols + k];
      }
    }
    SL_ASSERT_TRUE(sl_record_writer_write(ctx, &writer, &buffer));
    SL_ASSERT_EQ_LL(buffer.size, buf_size);
    SL_ASSERT_EQ_PTR(buffer.data, batch.data);
  }
  sl_record_writer_close(&writer);

  SL_ASSERT_TRUE(contains(
      sl_misc_tmpdir(),
      "large5.sl_record_data",
      sizeof(float) * record.size,
      (void*)dataset
  ));

  sl_free(dataset);

  return true;
}

SL_TEST(test_sparse_data_writer) {
  size_t nonzero_index[]     = {0, 1, 2, 100, 105, 9'012, 12'303, 12'304, 25'000, 90'123};
  int32_t nonzero_values[]   = {-5, -3, -2, 10, 100, 12'345, -2'003, 12'345, -20, 1'093};
  const size_t nonzero_count = SL_ARRAY_LEN(nonzero_index);

  struct sl_record record = {
      .layout   = "sparse",
      .type     = "int32",
      .name     = "large6",
      .size     = nonzero_count,
      .n_dims   = 1,
      .dim_size = {nonzero_index[nonzero_count - 1]},
  };
  strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
  record.path[sizeof(record.path) - 1] = '\0';

  struct sl_file file            = {0};
  struct sl_record_writer writer = {
      .file   = &file,
      .record = &record,
  };

  SL_ASSERT_TRUE(sl_record_writer_open(ctx, &writer));
  {
    int32_t buffer[1'024] = {0};
    const size_t buf_size = SL_ARRAY_LEN(buffer);
    struct sl_span batch  = sl_span_view(sizeof(buffer), (void*)buffer);
    size_t seen_items     = 0;
    for (size_t i = 0; i < (record.dim_size[0] + buf_size - 1) / buf_size; ++i) {
      for (size_t j = 0; j < buf_size; ++j) {
        size_t idx = i * buf_size + j;
        if (seen_items < nonzero_count && idx == nonzero_index[seen_items]) {
          buffer[j] = nonzero_values[seen_items];
          ++seen_items;
        } else {
          buffer[j] = 0;
        }
      }
      SL_ASSERT_TRUE(sl_record_writer_write(ctx, &writer, &batch));
    }
    SL_ASSERT_EQ_LL(seen_items, nonzero_count);
  }
  sl_record_writer_close(&writer);

  {
    char datapath[1'024] = {0};
    {
      int ok = snprintf(
          datapath,
          SL_ARRAY_LEN(datapath),
          "%s/%s",
          sl_misc_tmpdir(),
          "large6.sl_record_data"
      );
      SL_ASSERT_TRUE(ok > 0);
    }
    FILE* fp = fopen(datapath, "rb");
    SL_ASSERT_TRUE(fp);
    size_t idx = 0;
    for (size_t i = 0; i < nonzero_count; ++i) {
      int64_t offset = 0;
      SL_ASSERT_EQ_LL(fread(&offset, sizeof(offset), 1, fp), 1);
      SL_ASSERT_EQ_LL(ferror(fp), 0);
      int32_t value = 0;
      SL_ASSERT_EQ_LL(fread(&value, sizeof(value), 1, fp), 1);
      SL_ASSERT_EQ_LL(ferror(fp), 0);
      idx += (size_t)offset;
      SL_ASSERT_EQ_LL(idx, nonzero_index[i]);
      SL_ASSERT_EQ_LL(value, nonzero_values[i]);
    }
    fclose(fp);
  }

  return true;
}

SL_TEST(test_write_data) {
  {
    struct sl_record record = {
        .layout   = "dense",
        .type     = "float32",
        .name     = "small3",
        .size     = 12,
        .n_dims   = 2,
        .dim_size = {4, 3},
    };
    strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
    record.path[sizeof(record.path) - 1] = '\0';

    struct sl_matrix_f32 data = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    SL_ASSERT_TRUE(
        sl_record_write_all(ctx, &record, sizeof(float) * sl_matrix_f32_size(&data), data.data)
    );

    unsigned char raw[] = {0, 0, 0,    0x40, 0, 0, 0xe0, 0xc0, 0, 0, 0x40, 0x40, 0, 0, 0xe0, 0x40,
                           0, 0, 0x80, 0x3f, 0, 0, 0xa0, 0xc0, 0, 0, 0x40, 0xc0, 0, 0, 0x10, 0x41,
                           0, 0, 0xa0, 0xc0, 0, 0, 0x80, 0xbf, 0, 0, 0xc0, 0x40, 0, 0, 0x80, 0xbf};

    SL_ASSERT_TRUE(sl_record_write_metadata(ctx, &record));
    SL_ASSERT_TRUE(contains(sl_misc_tmpdir(), "small3.sl_record_data", SL_ARRAY_LEN(raw), raw));
  }

  {
    struct sl_record record = {
        .layout   = "sparse",
        .type     = "float32",
        .name     = "small4",
        .size     = 2,
        .n_dims   = 2,
        .dim_size = {4, 3},
    };
    strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
    record.path[sizeof(record.path) - 1] = '\0';

    struct sl_matrix_f32 data = {
        .length   = {4, 3},
        .capacity = {4, 3},
        .data     = (float[]){0, 0, 0, 5, 0, 0, 0, 0, 0, 0, -10, 0},
    };

    SL_ASSERT_TRUE(
        sl_record_write_all(ctx, &record, sizeof(float) * sl_matrix_f32_size(&data), data.data)
    );

    unsigned char raw_data[] = {0x03, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xa0, 0x40,
                                0x07, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x20, 0xc1};

    SL_ASSERT_TRUE(sl_record_write_metadata(ctx, &record));
    SL_ASSERT_TRUE(
        contains(sl_misc_tmpdir(), "small4.sl_record_data", SL_ARRAY_LEN(raw_data), raw_data)
    );
  }

  return true;
}

SL_TEST(test_sparse_write_and_read) {
  {
    struct sl_record record = {
        .layout   = "sparse",
        .type     = "float32",
        .name     = "empty",
        .size     = 0,
        .n_dims   = 2,
        .dim_size = {1'024 << 10, 1'024 << 10},
    };
    strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
    record.path[sizeof(record.path) - 1] = '\0';

    float data1[1] = {0};
    SL_ASSERT_TRUE(sl_record_write_all(ctx, &record, sizeof(data1[0]), (void*)data1));

    float data2[1] = {0};
    SL_ASSERT_TRUE(sl_record_read_all(ctx, &record, sizeof(data2), data2));
    SL_ASSERT_EQ_LL(data1[0], 0);
    SL_ASSERT_EQ_LL(data2[0], 0);
  }
  {
    struct sl_record record = {
        .layout   = "sparse",
        .type     = "float32",
        .name     = "small5",
        .size     = 16,
        .n_dims   = 2,
        .dim_size = {20, 5},
    };
    strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
    record.path[sizeof(record.path) - 1] = '\0';

    struct sl_matrix_f32 data1 = {
        .length   = {20, 5},
        .capacity = {20, 5},
        .data
        = (float[]){1, 0, 0,  0, 0, 0, -2, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  4, 0, 0, 0, 0, 0, -5,
                     0, 0, 0,  1, 0, 0, 0,  -2, 0,  0, 0, 0, 0, 0, 0, 4, 0, 0,  0, 0, 0, 5, 0, 0, 0,
                     0, 0, -1, 0, 0, 0, 0,  0,  -2, 0, 0, 0, 0, 0, 3, 0, 0, 0,  4, 0, 0, 0, 0, 0, 0,
                     0, 6, 0,  0, 0, 7, 0,  0,  0,  0, 0, 8, 0, 0, 0, 0, 0, -9, 0, 0, 0, 0, 0, 0, 0},
    };
    SL_ASSERT_TRUE(sl_record_write_all(
        ctx,
        &record,
        sizeof(data1.data[0]) * sl_matrix_f32_size(&data1),
        (void*)data1.data
    ));

    float data2[20 * 5] = {0};
    SL_ASSERT_TRUE(sl_record_read_all(ctx, &record, sizeof(data2), data2));
    SL_ASSERT_EQ_LL(memcmp(data1.data, data2, sizeof(data2)), 0);
  }
  return true;
}

SL_TEST(test_sparse_batch_write_and_read) {
  struct sl_record record = {
      .layout   = "sparse",
      .type     = "float32",
      .name     = "small5",
      .size     = 16,
      .n_dims   = 2,
      .dim_size = {20, 5},
  };
  strncpy(record.path, sl_misc_tmpdir(), sizeof(record.path) - 1);
  record.path[sizeof(record.path) - 1] = '\0';

  struct sl_matrix_f32 data1 = {
      .length   = {20, 5},
      .capacity = {20, 5},
      .data
      = (float[]){1, 0, 0,  0, 0, 0, -2, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  4, 0, 0, 0, 0, 0, -5,
                   0, 0, 0,  1, 0, 0, 0,  -2, 0,  0, 0, 0, 0, 0, 0, 4, 0, 0,  0, 0, 0, 5, 0, 0, 0,
                   0, 0, -1, 0, 0, 0, 0,  0,  -2, 0, 0, 0, 0, 0, 3, 0, 0, 0,  4, 0, 0, 0, 0, 0, 0,
                   0, 6, 0,  0, 0, 7, 0,  0,  0,  0, 0, 8, 0, 0, 0, 0, 0, -9, 0, 0, 0, 0, 0, 0, 0},
  };

  const size_t batch_size  = 5;
  const size_t buffer_size = batch_size * record.dim_size[1];
  const size_t batch_count = (record.dim_size[0] + batch_size - 1) / batch_size;

  {
    struct sl_file file            = {0};
    struct sl_record_writer writer = {
        .file   = &file,
        .record = &record,
    };
    SL_ASSERT_TRUE(sl_record_writer_open(ctx, &writer));
    for (size_t batch_idx = 0; batch_idx < batch_count; ++batch_idx) {
      SL_ASSERT_TRUE(sl_record_writer_write(
          ctx,
          &writer,
          &((struct sl_span){
              .size = sizeof(float) * buffer_size,
              .data = (void*)(data1.data + batch_idx * buffer_size),
          })
      ));
    }
    sl_record_writer_close(&writer);
  }

  float data2[20 * 5] = {0};
  {
    struct sl_file file            = {0};
    struct sl_record_reader reader = {
        .file   = &file,
        .record = &record,
    };
    SL_ASSERT_TRUE(sl_record_reader_open(ctx, &reader));
    for (size_t batch_idx = 0; batch_idx < batch_count; ++batch_idx) {
      SL_ASSERT_TRUE(!sl_record_reader_is_done(ctx, &reader));
      SL_ASSERT_TRUE(sl_record_reader_read(
          ctx,
          &reader,
          &((struct sl_span){
              .size = sizeof(float) * buffer_size,
              .data = (void*)(data2 + batch_idx * buffer_size),
          })
      ));
    }
    SL_ASSERT_TRUE(sl_record_reader_is_done(ctx, &reader));
    sl_record_reader_close(&reader);
  }
  SL_ASSERT_EQ_LL(memcmp(data1.data, data2, sizeof(data2)), 0);

  return true;
}

SL_TEST_MAIN()
