#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/io/io.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>

#include "./test_data.h"

static bool test_format_path(struct sl_context ctx[static 1], const bool) {
  (void)ctx;
  char buffer[1024]    = {0};
  const size_t bufsize = SL_ARRAY_LEN(buffer);

  assert(sl_file_format_path(bufsize, buffer, "/a/bbb/c1234", "d", ".txt"));
  SL_ASSERT_EQ_STR(buffer, "/a/bbb/c1234/d.txt");
  assert(!sl_file_format_path(bufsize, buffer, "", "b", ".c"));
  SL_ASSERT_EQ_STR(buffer, "");
  assert(!sl_file_format_path(bufsize, buffer, "a", "", ".c"));
  SL_ASSERT_EQ_STR(buffer, "");
  assert(sl_file_format_path(bufsize, buffer, "a", "b", ""));
  SL_ASSERT_EQ_STR(buffer, "a/b");
  assert(sl_file_format_path(bufsize, buffer, "a", "b", ".c"));
  SL_ASSERT_EQ_STR(buffer, "a/b.c");

  return true;
}

static bool test_open_file(struct sl_context ctx[static 1], const bool) {
  const char* files[] = {
      "./test-data/txt/empty",
      "./test-data/txt/hello.txt",
      "./test-data/txt/numbers.txt",
      "./test-data/txt/one.txt",
      "./test-data/png/asan.png",
  };

  for (size_t i = 0; i < SL_ARRAY_LEN(files); ++i) {
    struct sl_file file = {0};
    assert(sl_file_open(ctx, &file, files[i], "rb"));
    assert(file.file);
    assert(sl_file_can_read(&file));
    SL_ASSERT_EQ_STR(file.path, files[i]);
    sl_file_close(&file);
    assert(!sl_file_can_read(&file));
    assert(!file.file);
  }

  return true;
}

static bool test_read_single_char(struct sl_context ctx[static 1], const bool) {
  struct sl_file file = {0};
  assert(sl_file_open(ctx, &file, "./test-data/txt/one.txt", "rb"));
  assert(sl_file_can_read(&file));

  unsigned char buf[8]  = {0};
  struct sl_span buffer = sl_span_view(SL_ARRAY_LEN(buf), buf);

  SL_ASSERT_EQ_LL(sl_file_read(ctx, &file, &buffer), 1);
  assert(!ferror(file.file));
  assert(feof(file.file));

  SL_ASSERT_EQ_CHAR(buffer.data[0], '1');
  SL_ASSERT_EQ_CHAR(buf[0], '1');

  assert(!sl_file_can_read(&file));
  sl_file_close(&file);
  return true;
}

static bool test_read_empty_file(struct sl_context ctx[static 1], const bool) {
  struct sl_file file = {0};
  assert(sl_file_open(ctx, &file, "./test-data/txt/empty", "rb"));
  assert(!ferror(file.file));

  unsigned char buf[1]  = {0};
  struct sl_span buffer = sl_span_view(SL_ARRAY_LEN(buf), buf);
  assert(sl_file_can_read(&file));
  SL_ASSERT_EQ_LL(sl_file_read(ctx, &file, &buffer), 0);
  assert(!sl_file_can_read(&file));

  sl_file_close(&file);
  return true;
}

static bool test_read_entire_file(struct sl_context ctx[static 1], const bool) {
  unsigned char buf[1024] = {0};
  struct sl_span buffer   = sl_span_view(SL_ARRAY_LEN(buf), buf);

  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_file_paths); ++i) {
    struct sl_file file = {0};
    assert(sl_file_open(ctx, &file, sl_test_data_file_paths[i], "rb"));
    assert(sl_file_can_read(&file));
    size_t total_size = 0;
    while (sl_file_can_read(&file)) {
      total_size += sl_file_read(ctx, &file, &buffer);
    }
    assert(!ferror(file.file));
    assert(feof(file.file));
    SL_ASSERT_EQ_LL(total_size, sl_test_data_file_sizes[i]);
    sl_file_close(&file);
    assert(!sl_file_can_read(&file));
  }
  return true;
}

static bool test_parse_numbers(struct sl_context ctx[static 1], const bool) {
  struct sl_file file = {0};
  assert(sl_file_open(ctx, &file, "./test-data/txt/numbers.txt", "rb"));
  assert(sl_file_can_read(&file));

  int64_t numbers[100] = {0};
  const size_t len     = SL_ARRAY_LEN(numbers);
  SL_ASSERT_EQ_LL(sl_file_parse_int64(ctx, &file, len, numbers), len);
  for (size_t i = 0; i < len; ++i) {
    SL_ASSERT_EQ_LL(numbers[i], i - 50);
  }

  sl_file_close(&file);
  return true;
}

static bool test_io_read_one_byte(struct sl_context ctx[static 1], const bool) {
  unsigned char buf[1] = {0};
  assert(sl_io_read(ctx, "./test-data/txt/one.txt", buf, 1));
  assert(!sl_error_occurred(&ctx->errors));
  SL_ASSERT_EQ_CHAR(buf[0], '1');
  return true;
}

static bool test_io_read_file(struct sl_context ctx[static 1], const bool) {
  uint64_t buf = 0;
  assert(sl_io_read(ctx, "./test-data/txt/8bytes.txt", (unsigned char*)&buf, 8));
  assert(!sl_error_occurred(&ctx->errors));
  // lil-endian 12345678 ascii digits
  assert(buf == 0x3837363534333231UL);
  return true;
}

static bool test_io_read_missing_file(struct sl_context ctx[static 1], const bool) {
  unsigned char buf[1] = {0};
  sl_io_read(ctx, "./test-data/txt/does_not_exist", buf, 1);
  assert(sl_error_occurred(&ctx->errors));
  sl_error_clear(&ctx->errors);
  return true;
}

static bool test_io_read_too_many_bytes(struct sl_context ctx[static 1], const bool) {
  unsigned char buf[8] = {0};
  sl_io_read(ctx, "./test-data/txt/one.txt", buf, sizeof(buf));
  assert(sl_error_occurred(&ctx->errors));
  sl_error_clear(&ctx->errors);
  return true;
}

SL_TEST_MAIN(
    test_format_path,
    test_open_file,
    test_read_single_char,
    test_read_empty_file,
    test_read_entire_file,
    test_parse_numbers,
    test_io_read_one_byte,
    test_io_read_file,
    test_io_read_missing_file,
    test_io_read_too_many_bytes
)
