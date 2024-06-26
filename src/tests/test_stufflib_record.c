#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_misc.h"
#include "stufflib_record.h"

bool contains(const char dir[static const 1],
              const char name[static const 1],
              const size_t count,
              unsigned char expected[static const 1]) {
  char path[200] = {0};
  assert(0 < snprintf(path, SL_ARRAY_LEN(path), "%s/%s", dir, name));
  FILE* fp = fopen(path, "rb");
  assert(fp);
  unsigned char content[4096] = {0};
  const size_t n_read = fread(content, 1, SL_ARRAY_LEN(content), fp);
  fclose(fp);
  return n_read == count && memcmp(content, expected, count) == 0;
}

bool contains_str(const char dir[static const 1],
                  const char name[static const 1],
                  char expected[static const 1]) {
  return contains(dir, name, strlen(expected), (unsigned char*)expected);
}

bool test_write_metadata(const bool) {
  {
    struct sl_record record = {
        .layout = "dense",
        .type = "float",
        .name = "small1",
        .size = 3,
        .n_dims = 1,
        .dim_size = {3},
    };
    strcpy(record.path, sl_misc_tmpdir());
    assert(sl_record_write_metadata(&record));
    assert(contains_str(sl_misc_tmpdir(),
                        "small1.sl_record_meta",
                        ("name: small1\n"
                         "type: float\n"
                         "layout: dense\n"
                         "size: 3\n"
                         "dims: 1\n"
                         "dim0: 3\n")));
  }
  {
    struct sl_record record = {
        .layout = "dense",
        .type = "float",
        .name = "small2",
        .size = 3,
        .n_dims = 3,
        .dim_size = {1, 1, 1},
    };
    strcpy(record.path, sl_misc_tmpdir());
    assert(sl_record_write_metadata(&record));
    assert(contains_str(sl_misc_tmpdir(),
                        "small2.sl_record_meta",
                        ("name: small2\n"
                         "type: float\n"
                         "layout: dense\n"
                         "size: 3\n"
                         "dims: 3\n"
                         "dim0: 1\n"
                         "dim1: 1\n"
                         "dim2: 1\n")));
  }
  {
    struct sl_record record = {
        .layout = "dense",
        .type = "float",
        .name = "large1",
        .size = 10'000'000,
        .n_dims = 4,
        .dim_size = {10, 15625, 3125, 64},
    };
    strcpy(record.path, sl_misc_tmpdir());
    assert(sl_record_write_metadata(&record));
    assert(contains_str(sl_misc_tmpdir(),
                        "large1.sl_record_meta",
                        ("name: large1\n"
                         "type: float\n"
                         "layout: dense\n"
                         "size: 10000000\n"
                         "dims: 4\n"
                         "dim0: 10\n"
                         "dim1: 15625\n"
                         "dim2: 3125\n"
                         "dim3: 64\n")));
  }
  return true;
}

bool test_read_metadata(const bool) {
  {
    struct sl_record record;
    assert(sl_record_read_metadata(&record, "./test-data/record", "small1"));
    assert(record.path);
    assert(record.name);
    assert(record.type);
    assert(record.layout);
    assert(strcmp(record.path, "./test-data/record") == 0);
    assert(strcmp(record.name, "small1") == 0);
    assert(strcmp(record.type, "float") == 0);
    assert(strcmp(record.layout, "dense") == 0);
    assert(record.size == 3);
    assert(record.n_dims == 1);
    assert(record.dim_size);
    assert(record.dim_size[0] == 3);
  }
  {
    struct sl_record record;
    assert(sl_record_read_metadata(&record, "./test-data/record", "small2"));
    assert(record.path);
    assert(record.name);
    assert(record.type);
    assert(record.layout);
    assert(strcmp(record.path, "./test-data/record") == 0);
    assert(strcmp(record.name, "small2") == 0);
    assert(strcmp(record.type, "float") == 0);
    assert(strcmp(record.layout, "dense") == 0);
    assert(record.size == 3);
    assert(record.n_dims == 3);
    assert(record.dim_size);
    assert(record.dim_size + 1);
    assert(record.dim_size + 2);
    assert(record.dim_size[0] == 1);
    assert(record.dim_size[1] == 1);
    assert(record.dim_size[2] == 1);
  }
  {
    struct sl_record record;
    assert(sl_record_read_metadata(&record, "./test-data/record", "large1"));
    assert(record.path);
    assert(record.name);
    assert(record.type);
    assert(record.layout);
    assert(strcmp(record.path, "./test-data/record") == 0);
    assert(strcmp(record.name, "large1") == 0);
    assert(strcmp(record.type, "float") == 0);
    assert(strcmp(record.layout, "dense") == 0);
    assert(record.size == 10'000'000);
    assert(record.n_dims == 4);
    assert(record.dim_size);
    assert(record.dim_size + 1);
    assert(record.dim_size + 2);
    assert(record.dim_size + 3);
    assert(record.dim_size[0] == 10);
    assert(record.dim_size[1] == 15625);
    assert(record.dim_size[2] == 3125);
    assert(record.dim_size[3] == 64);
  }
  {
    struct sl_record record;
    assert(sl_record_read_metadata(&record, "./test-data/record", "large2"));
    assert(record.path);
    assert(record.name);
    assert(record.type);
    assert(record.layout);
    assert(strcmp(record.path, "./test-data/record") == 0);
    assert(strcmp(record.name, "large2") == 0);
    assert(strcmp(record.type, "float") == 0);
    assert(strcmp(record.layout, "sparse") == 0);
    assert(record.size == 10);
    assert(record.n_dims == 3);
    assert(record.dim_size);
    assert(record.dim_size + 1);
    assert(record.dim_size + 2);
    assert(record.dim_size[0] == 1000);
    assert(record.dim_size[1] == 2000);
    assert(record.dim_size[2] == 3000);
  }
  return true;
}

bool test_append_data(const bool) {
  {
    struct sl_record record = {
        .layout = "dense",
        .type = "float",
        .name = "small3",
        .size = 12,
        .n_dims = 2,
        .dim_size = {4, 3},
    };
    strcpy(record.path, sl_misc_tmpdir());

    struct sl_la_matrix data = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){2, -7, 3, 7, 1, -5, -3, 9, -5, -1, 6, -1},
    };
    assert(sl_record_append_data(&record, data.data, sl_la_matrix_size(&data)));
    assert(record.pos == 12);

    unsigned char raw[] = {
        0, 0, 0,    0x40, 0, 0, 0xe0, 0xc0, 0, 0, 0x40, 0x40, 0, 0, 0xe0, 0x40,
        0, 0, 0x80, 0x3f, 0, 0, 0xa0, 0xc0, 0, 0, 0x40, 0xc0, 0, 0, 0x10, 0x41,
        0, 0, 0xa0, 0xc0, 0, 0, 0x80, 0xbf, 0, 0, 0xc0, 0x40, 0, 0, 0x80, 0xbf};

    assert(sl_record_write_metadata(&record));
    assert(contains(sl_misc_tmpdir(),
                    "small3.sl_record_data",
                    SL_ARRAY_LEN(raw),
                    raw));
  }

  {
    struct sl_record record = {
        .layout = "sparse",
        .type = "float",
        .name = "small4",
        .n_dims = 2,
        .dim_size = {4, 3},
    };
    strcpy(record.path, sl_misc_tmpdir());

    struct sl_la_matrix data = {
        .rows = 4,
        .cols = 3,
        .data = (float[]){0, 0, 0, 5, 0, 0, 0, 0, 0, 0, -10, 0},
    };

    assert(sl_record_append_data(&record, data.data, sl_la_matrix_size(&data)));
    assert(record.size == 2);
    assert(record.pos == 12);

    unsigned char raw_data[] = {0, 0, 0xa0, 0x40, 0, 0, 0x20, 0xc1};
    unsigned char raw_sparse_index[] = {0x03, 0, 0, 0, 0x07, 0, 0, 0};

    assert(sl_record_write_metadata(&record));
    assert(contains(sl_misc_tmpdir(),
                    "small4.sl_record_sparse_index",
                    SL_ARRAY_LEN(raw_sparse_index),
                    raw_sparse_index));
    assert(contains(sl_misc_tmpdir(),
                    "small4.sl_record_data",
                    SL_ARRAY_LEN(raw_data),
                    raw_data));
  }

  return true;
}

bool test_read_data(const bool) {
  {
    struct sl_record record = {
        .layout = "dense",
        .type = "float",
        .name = "small3",
        .path = "./test-data/record",
        .size = 2,
        .n_dims = 1,
        .dim_size = {2},
    };
    float data[2] = {0};
    assert(sl_record_read_data(&record, data));
    assert(sl_math_double_almost(data[0], 5, 1e-9));
    assert(sl_math_double_almost(data[1], -10, 1e-9));
  }
  {
    struct sl_record record = {
        .layout = "sparse",
        .type = "float",
        .name = "small3",
        .path = "./test-data/record",
        .size = 2,
        .n_dims = 2,
        .dim_size = {4, 3},
    };
    float expected[12] = {0, 0, 0, 5, 0, 0, 0, 0, 0, 0, -10, 0};
    float data[12] = {0};
    assert(sl_record_read_data(&record, data));
    for (size_t i = 0; i < record.size; ++i) {
      assert(sl_math_double_almost(data[i], expected[i], 1e-9));
    }
  }
  return true;
}

SL_TEST_MAIN(test_write_metadata,
             test_read_metadata,
             test_append_data,
             test_read_data)
