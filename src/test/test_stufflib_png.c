#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_misc.h"
#include "stufflib_png.h"

int _test_read_single_pixel_header(const int verbose) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/ff0000-1x1-rgb-nocomp.png",
        "./test-data/00ff00-1x1-rgb-nocomp.png",
        "./test-data/0000ff-1x1-rgb-nocomp.png",
    }[i];
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_header pixel = stufflib_png_read_header(png_path);
    if (!pixel.width) {
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_header(stdout, pixel);
    }
    assert(pixel.width == 1);
    assert(pixel.height == 1);
    assert(pixel.bit_depth == 8);
    assert(pixel.color_type == 2);
  }
  return 0;
}

int _test_read_img_header(const int verbose) {
  {
    const char* png_path = "./test-data/white-square-rgba-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_header white_square = stufflib_png_read_header(png_path);
    if (!white_square.width) {
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_header(stdout, white_square);
    }
    assert(white_square.width == 26);
    assert(white_square.height == 28);
    assert(white_square.bit_depth == 8);
    assert(white_square.color_type == 6);
  }

  {
    const char* png_path = "./test-data/github-squares-rgb-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_header github_squares = stufflib_png_read_header(png_path);
    if (!github_squares.width) {
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_header(stdout, github_squares);
    }
    assert(github_squares.width == 81);
    assert(github_squares.height == 77);
    assert(github_squares.bit_depth == 8);
    assert(github_squares.color_type == 2);
  }

  {
    const char* png_path = "./test-data/github-profile-rgb-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_header github_profile = stufflib_png_read_header(png_path);
    if (!github_profile.width) {
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_header(stdout, github_profile);
    }
    assert(github_profile.width == 420);
    assert(github_profile.height == 420);
    assert(github_profile.bit_depth == 8);
    assert(github_profile.color_type == 2);
  }

  return 0;
}

static inline int _test_read_single_pixel(const char* png_path,
                                          const size_t on_index,
                                          const int verbose) {
  if (verbose) {
    printf("%s\n", png_path);
  }
  stufflib_png_image pixel = stufflib_png_read_image(png_path);
  if (!pixel.size) {
    return 1;
  }
  if (verbose) {
    stufflib_png_dump_img_meta(stdout, pixel);
  }
  assert(pixel.header.width == 1);
  assert(pixel.header.height == 1);
  assert(pixel.header.bit_depth == 8);
  assert(pixel.header.color_type == 2);

  assert(pixel.size == 4);
  unsigned char expected_data[4] = {0};
  expected_data[on_index] = 255;
  for (size_t c = 0; c < 4; ++c) {
    assert(pixel.data[c] == expected_data[c]);
  }

  stufflib_png_destroy(&pixel);
  return 0;
}

int _test_read_single_pixel_no_compression(const int verbose) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/ff0000-1x1-rgb-nocomp.png",
        "./test-data/00ff00-1x1-rgb-nocomp.png",
        "./test-data/0000ff-1x1-rgb-nocomp.png",
    }[i];
    const size_t on_pixel = i + 1;
    if (_test_read_single_pixel(png_path, on_pixel, verbose)) {
      return 1;
    }
  }
  return 0;
}

int _test_read_single_pixel_fixed_huffman_code_compression(const int verbose) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/ff0000-1x1-rgb-fixed.png",
        "./test-data/00ff00-1x1-rgb-fixed.png",
        "./test-data/0000ff-1x1-rgb-fixed.png",
    }[i];
    const size_t on_pixel = i + 1;
    if (_test_read_single_pixel(png_path, on_pixel, verbose)) {
      return 1;
    }
  }
  return 0;
}

typedef int test_function(const int);

int main(int argc, char* const argv[argc + 1]) {
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  test_function* tests[] = {
      _test_read_single_pixel_header,
      _test_read_img_header,
      _test_read_single_pixel_no_compression,
      _test_read_single_pixel_fixed_huffman_code_compression,
  };
  const char* test_names[] = {
      "read_single_pixel_header",
      "read_img_header",
      "read_single_pixel_no_compression",
      "read_single_pixel_fixed_huffman_code_compresssion",
  };

  for (size_t t = 0; t < (sizeof(tests) / sizeof(tests[0])); ++t) {
    if (verbose) {
      printf("\ntesting %s\n", test_names[t]);
    }
    if (tests[t](verbose)) {
      fprintf(stderr, "test %zu failed\n", t + 1);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
