#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/context/context.h>
#include <stufflib/error/error.h>
#include <stufflib/io/io.h>
#include <stufflib/macros/macros.h>
#include <stufflib/span/span.h>
#include <stufflib/testing/testing.h>

#include "./test_data.h"

SL_TEST(test_format_path) {
  (void)ctx;
  char buffer[1024]    = {0};
  const size_t bufsize = SL_ARRAY_LEN(buffer);

  SL_ASSERT_TRUE(sl_file_format_path(bufsize, buffer, "/a/bbb/c1234", "d", ".txt"));
  SL_ASSERT_EQ_STR(buffer, "/a/bbb/c1234/d.txt");
  SL_ASSERT_TRUE(!sl_file_format_path(bufsize, buffer, "", "b", ".c"));
  SL_ASSERT_EQ_STR(buffer, "");
  SL_ASSERT_TRUE(!sl_file_format_path(bufsize, buffer, "a", "", ".c"));
  SL_ASSERT_EQ_STR(buffer, "");
  SL_ASSERT_TRUE(sl_file_format_path(bufsize, buffer, "a", "b", ""));
  SL_ASSERT_EQ_STR(buffer, "a/b");
  SL_ASSERT_TRUE(sl_file_format_path(bufsize, buffer, "a", "b", ".c"));
  SL_ASSERT_EQ_STR(buffer, "a/b.c");

  return true;
}

SL_TEST(test_open_file) {
  const char* files[] = {
      "./test-data/txt/empty",
      "./test-data/txt/hello.txt",
      "./test-data/txt/numbers.txt",
      "./test-data/txt/one.txt",
      "./test-data/png/asan.png",
  };

  for (size_t i = 0; i < SL_ARRAY_LEN(files); ++i) {
    struct sl_file file = {0};
    SL_ASSERT_TRUE(sl_file_open(ctx, &file, files[i], "rb"));
    SL_ASSERT_TRUE(file.file);
    SL_ASSERT_TRUE(sl_file_can_read(&file));
    SL_ASSERT_EQ_STR(file.path, files[i]);
    sl_file_close(&file);
    SL_ASSERT_TRUE(!sl_file_can_read(&file));
    SL_ASSERT_TRUE(!file.file);
  }

  return true;
}

SL_TEST(test_read_single_char) {
  struct sl_file file = {0};
  SL_ASSERT_TRUE(sl_file_open(ctx, &file, "./test-data/txt/one.txt", "rb"));
  SL_ASSERT_TRUE(sl_file_can_read(&file));

  unsigned char buf[8]  = {0};
  struct sl_span buffer = sl_span_view(SL_ARRAY_LEN(buf), buf);

  SL_ASSERT_EQ_LL(sl_file_read(ctx, &file, &buffer), 1);
  SL_ASSERT_TRUE(!ferror(file.file));
  SL_ASSERT_TRUE(feof(file.file));

  SL_ASSERT_EQ_CHAR(buffer.data[0], '1');
  SL_ASSERT_EQ_CHAR(buf[0], '1');

  SL_ASSERT_TRUE(!sl_file_can_read(&file));
  sl_file_close(&file);
  return true;
}

SL_TEST(test_read_empty_file) {
  struct sl_file file = {0};
  SL_ASSERT_TRUE(sl_file_open(ctx, &file, "./test-data/txt/empty", "rb"));
  SL_ASSERT_TRUE(!ferror(file.file));

  unsigned char buf[1]  = {0};
  struct sl_span buffer = sl_span_view(SL_ARRAY_LEN(buf), buf);
  SL_ASSERT_TRUE(sl_file_can_read(&file));
  SL_ASSERT_EQ_LL(sl_file_read(ctx, &file, &buffer), 0);
  SL_ASSERT_TRUE(!sl_file_can_read(&file));

  sl_file_close(&file);
  return true;
}

SL_TEST(test_read_entire_file) {
  unsigned char buf[1024] = {0};
  struct sl_span buffer   = sl_span_view(SL_ARRAY_LEN(buf), buf);

  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_file_paths); ++i) {
    struct sl_file file = {0};
    SL_ASSERT_TRUE(sl_file_open(ctx, &file, sl_test_data_file_paths[i], "rb"));
    SL_ASSERT_TRUE(sl_file_can_read(&file));
    size_t total_size = 0;
    while (sl_file_can_read(&file)) {
      total_size += sl_file_read(ctx, &file, &buffer);
    }
    SL_ASSERT_TRUE(!ferror(file.file));
    SL_ASSERT_TRUE(feof(file.file));
    SL_ASSERT_EQ_LL(total_size, sl_test_data_file_sizes[i]);
    sl_file_close(&file);
    SL_ASSERT_TRUE(!sl_file_can_read(&file));
  }
  return true;
}

SL_TEST(test_parse_numbers) {
  struct sl_file file = {0};
  SL_ASSERT_TRUE(sl_file_open(ctx, &file, "./test-data/txt/numbers.txt", "rb"));
  SL_ASSERT_TRUE(sl_file_can_read(&file));

  int64_t numbers[100] = {0};
  const size_t len     = SL_ARRAY_LEN(numbers);
  SL_ASSERT_EQ_LL(sl_file_parse_int64(ctx, &file, len, numbers), len);
  for (size_t i = 0; i < len; ++i) {
    SL_ASSERT_EQ_LL(numbers[i], i - 50);
  }

  sl_file_close(&file);
  return true;
}

SL_TEST(test_io_read_one_byte) {
  unsigned char buf[1] = {0};
  SL_ASSERT_TRUE(sl_io_read(ctx, "./test-data/txt/one.txt", buf, 1));
  SL_ASSERT_TRUE(!sl_error_occurred(&ctx->errors));
  SL_ASSERT_EQ_CHAR(buf[0], '1');
  return true;
}

SL_TEST(test_io_read_file) {
  uint64_t buf = 0;
  SL_ASSERT_TRUE(sl_io_read(ctx, "./test-data/txt/8bytes.txt", (unsigned char*)&buf, 8));
  SL_ASSERT_TRUE(!sl_error_occurred(&ctx->errors));
  // lil-endian 12345678 ascii digits
  SL_ASSERT_TRUE(buf == 0x3837363534333231UL);
  return true;
}

SL_TEST(test_io_read_missing_file) {
  unsigned char buf[1] = {0};
  sl_io_read(ctx, "./test-data/txt/does_not_exist", buf, 1);
  SL_ASSERT_TRUE(sl_error_occurred(&ctx->errors));
  sl_error_clear(&ctx->errors);
  return true;
}

SL_TEST(test_io_read_too_many_bytes) {
  unsigned char buf[8] = {0};
  sl_io_read(ctx, "./test-data/txt/one.txt", buf, sizeof(buf));
  SL_ASSERT_TRUE(sl_error_occurred(&ctx->errors));
  sl_error_clear(&ctx->errors);
  return true;
}

SL_TEST_MAIN()
