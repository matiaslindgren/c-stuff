#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_string.h"
#include "stufflib_test.h"

bool test_1(const bool verbose) {
  const char* utf8_files[] = {
      "./test-data/txt/wikipedia/water_ar.txt",
      "./test-data/txt/wikipedia/water_de.txt",
      "./test-data/txt/wikipedia/water_el.txt",
      "./test-data/txt/wikipedia/water_fi.txt",
      "./test-data/txt/wikipedia/water_fr.txt",
      "./test-data/txt/wikipedia/water_he.txt",
      "./test-data/txt/wikipedia/water_hi.txt",
      "./test-data/txt/wikipedia/water_is.txt",
      "./test-data/txt/wikipedia/water_ja.txt",
      "./test-data/txt/wikipedia/water_ka.txt",
      "./test-data/txt/wikipedia/water_ko.txt",
      "./test-data/txt/wikipedia/water_pl.txt",
      "./test-data/txt/wikipedia/water_ru.txt",
      "./test-data/txt/wikipedia/water_ta.txt",
      "./test-data/txt/wikipedia/water_tr.txt",
      "./test-data/txt/wikipedia/water_vi.txt",
      "./test-data/txt/wikipedia/water_zh.txt",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(utf8_files); ++i) {
    const char* filename = utf8_files[i];
    if (verbose) {
      printf("%s\n", filename);
    }
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
      STUFFLIB_PRINT_ERROR("cannot open %s", filename);
      continue;
    }
    unsigned char buf[10000] = {0};
    size_t size = fread(buf, 1, 10000, fp);
    fclose(fp);
    assert(size > 0);
    assert(stufflib_string_is_utf8(size, buf));
  }

  return true;
}

STUFFLIB_TEST_MAIN(test_1)
