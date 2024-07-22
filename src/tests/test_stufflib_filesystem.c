#include <assert.h>
#include <stdlib.h>

#include "./test_data.h"
#include "stufflib/args/args.h"
#include "stufflib/filesystem/filesystem.h"
#include "stufflib/iterator/iterator.h"
#include "stufflib/macros/macros.h"
#include "stufflib/span/span.h"
#include "stufflib/string/string.h"

static bool test_read_file(const bool verbose) {
  unsigned char buf[128] = {0};
  struct sl_span buffer = sl_span_view(SL_ARRAY_LEN(buf), buf);

  for (size_t i = 0; i < SL_ARRAY_LEN(sl_test_data_file_paths); ++i) {
    if (verbose) {
      printf("reading test file '%s'\n", sl_test_data_file_paths[i]);
    }
    struct sl_span data = sl_fs_read_file(sl_test_data_file_paths[i], &buffer);
    assert(data.owned);
    SL_ASSERT_EQ_LL(data.size, sl_test_data_file_sizes[i]);
    if (data.size > 0) {
      assert(data.data != nullptr);
    }
    sl_span_destroy(&data);
  }
  return true;
}

static bool test_read_file_utf8(const bool verbose) {
  unsigned char buf[128] = {0};
  struct sl_span buffer = sl_span_view(SL_ARRAY_LEN(buf), buf);

  const char* languages[] = {
      "ar", "bg",  "cs", "de",  "el", "fa", "fi", "fr",  "he",  "hi", "is",
      "ja", "ka",  "ki", "ko",  "ku", "lt", "lv", "nah", "nqo", "pl", "pt",
      "ru", "shi", "sl", "szl", "ta", "tr", "uk", "vep", "vi",  "zh",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(languages); ++i) {
    char input_path[200] = {0};
    snprintf(input_path,
             SL_ARRAY_LEN(input_path),
             "./test-data/txt/wikipedia/water_%s.txt",
             languages[i]);
    if (verbose) {
      printf("%s\n", input_path);
    }

    char length_path[200] = {0};
    snprintf(length_path,
             SL_ARRAY_LEN(length_path),
             "./test-data/txt/wikipedia/water_%s_length.txt",
             languages[i]);

    FILE* fp = fopen(length_path, "rb");
    assert(fp);
    char tmp[200] = {0};
    assert(fread(tmp, 1, 200, fp));
    fclose(fp);
    const size_t expected_str_length = strtoull(tmp, 0, 10);

    struct sl_string str = sl_fs_read_file_utf8(input_path, &buffer);
    assert(str.length == expected_str_length);
    sl_string_destroy(&str);
  }
  return true;
}

static bool test_read_lines(const bool verbose) {
  unsigned char buf[128] = {0};
  struct sl_span buffer = sl_span_view(SL_ARRAY_LEN(buf), buf);

  const char* expected[] = {
      u8"# test-data/txt/wikipedia/water_fi.txt",
      u8"Vesi on huoneenlämmössä",
      u8"",
      u8"",
      u8" nesteenä esiintyvä",
      u8"",
      u8"vedyn ja hapen",
      u8"  muodostama epäorgaaninen",
      u8"",
      u8"kemiallinen yhdiste ja vedyn palamistuote.",
      u8"",
  };

  struct sl_string str =
      sl_fs_read_file_utf8("./test-data/txt/lines.txt", &buffer);
  {
    // TODO create iterlines util
    struct sl_span newline = sl_span_view(1, (unsigned char[]){'\n'});
    struct sl_tokenizer newline_tokenizer =
        sl_tokenizer_create(&(str.utf8_data), &newline);
    size_t lineno = 0;
    for (struct sl_iterator iter = sl_tokenizer_iter(&newline_tokenizer);
         !sl_tokenizer_iter_is_done(&iter);
         sl_tokenizer_iter_advance(&iter)) {
      struct sl_string line = sl_string_from_utf8(sl_tokenizer_iter_get(&iter));
      if (verbose) {
        printf("line %zu:", lineno);
        bool print_ok = sl_string_fprint(stdout, &line);
        printf("\n");
        assert(print_ok);
      }
      assert(lineno < SL_ARRAY_LEN(expected));
      const size_t line_len = strlen(expected[lineno]);
      assert(memcmp(line.utf8_data.data,
                    expected[lineno],
                    SL_MIN(line_len, line.utf8_data.size)) == 0);
      sl_string_destroy(&line);
      ++lineno;
    }
  }
  sl_string_destroy(&str);

  return true;
}

SL_TEST_MAIN(test_read_file, test_read_file_utf8, test_read_lines)
