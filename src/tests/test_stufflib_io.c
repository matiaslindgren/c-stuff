#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_args.h"
#include "stufflib_io.h"
#include "stufflib_iterator.h"
#include "stufflib_macros.h"

bool test_create_file_iter_from_file(const bool verbose) {
  const char* files[] = {
      "./test-data/txt/empty",
      "./test-data/txt/hello.txt",
      "./test-data/txt/numbers.txt",
      "./test-data/txt/one.txt",
      "./test-data/png/asan.png",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(files); ++i) {
    struct sl_iterator iter = sl_file_iter_open(files[i]);
    assert(iter.data);
    struct sl_file_buffer* buffer = iter.data;
    assert(buffer);
    assert(buffer->filename);
    assert(strcmp(buffer->filename, files[i]) == 0);
    assert(buffer->capacity > 0);
    assert(buffer->data.data);
    assert(buffer->file);
    assert(!ferror(buffer->file));
    sl_file_iter_close(&iter);
  }
  return true;
}

bool test_read_single_char(const bool verbose) {
  struct sl_iterator iter = sl_file_iter_open("./test-data/txt/one.txt");
  assert(iter.index == 0);
  assert(iter.pos == 0);
  assert(!sl_file_iter_is_done(&iter));

  struct sl_span* data = sl_file_iter_get(&iter);
  assert(data);
  assert(data->size == 1);
  assert(data->data[0] == '1');

  sl_file_iter_advance(&iter);
  assert(sl_file_iter_is_done(&iter));

  sl_file_iter_close(&iter);
  return true;
}

bool test_read_empty_file(const bool verbose) {
  struct sl_iterator iter = sl_file_iter_open("./test-data/txt/empty");
  assert(iter.index == 0);
  assert(iter.pos == 0);
  assert(sl_file_iter_is_done(&iter));
  sl_file_iter_close(&iter);
  return true;
}

bool test_read_nonexisting_file(const bool verbose) {
  struct sl_iterator iter =
      sl_file_iter_open("./test-data/txt/missing/file.txt");
  assert(iter.index == 0);
  assert(iter.pos == 0);
  assert(!iter.data);
  sl_file_iter_close(&iter);
  return true;
}

bool test_read_read_entire_file(const bool verbose) {
  const char* files[] = {
      "./test-data/png/asan.png",
      "./test-data/png/ff0000-1x1-rgb-fixed.png",
      "./test-data/txt/empty",
      "./test-data/txt/hello.txt",
      "./test-data/txt/numbers.txt",
      "./test-data/txt/one.txt",
      "./test-data/txt/wikipedia/water_ar.txt",
      "./test-data/txt/wikipedia/water_ar_codepoints.txt",
      "./test-data/txt/wikipedia/water_zh.txt",
      "./test-data/txt/wikipedia/water_zh_codepoints.txt",

  };
  const size_t file_sizes[] = {
      24'733,
      69,
      0,
      11,
      292,
      1,
      922,
      2'345,
      190,
      383,
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(files); ++i) {
    struct sl_iterator iter = sl_file_iter_open(files[i]);
    struct sl_file_buffer* buffer = iter.data;
    size_t total_size = 0;
    while (!sl_file_iter_is_done(&iter)) {
      total_size += buffer->data.size;
      sl_file_iter_advance(&iter);
    }
    assert(!ferror(buffer->file));
    assert(feof(buffer->file));
    assert(buffer->data.size == 0);
    if (verbose) {
      printf("computed size %zu, expecting %zu, file '%s'\n",
             total_size,
             file_sizes[i],
             files[i]);
    }
    assert(total_size == file_sizes[i]);
    sl_file_iter_close(&iter);
  }
  return true;
}

SL_TEST_MAIN(test_create_file_iter_from_file,
             test_read_single_char,
             test_read_empty_file,
             test_read_nonexisting_file,
             test_read_read_entire_file)
