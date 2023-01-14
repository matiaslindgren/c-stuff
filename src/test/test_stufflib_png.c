#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_misc.h"
#include "stufflib_png.h"

int _test_read_single_pixel_chunks(const int verbose) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/ff0000-1x1-rgb-nocomp.png",
        "./test-data/00ff00-1x1-rgb-nocomp.png",
        "./test-data/0000ff-1x1-rgb-nocomp.png",
    }[i];
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_chunks chunks = stufflib_png_read_chunks(png_path);
    assert(chunks.count == 3);
    assert(chunks.chunks[0].type == stufflib_png_IHDR);
    assert(chunks.chunks[0].data.size == 13);
    assert(chunks.chunks[1].type == stufflib_png_IDAT);
    assert(chunks.chunks[1].data.size == 15);
    assert(chunks.chunks[2].type == stufflib_png_IEND);
    assert(chunks.chunks[2].data.size == 0);
    stufflib_png_chunks_destroy(chunks);
  }
  return 0;
}

int _test_read_large_image_with_many_chunks(const int verbose) {
  const char* png_path = "./test-data/asan.png";
  if (verbose) {
    printf("%s\n", png_path);
  }
  stufflib_png_chunks chunks = stufflib_png_read_chunks(png_path);
  assert(chunks.count == 6);
  assert(chunks.chunks[0].type == stufflib_png_IHDR);
  assert(chunks.chunks[0].data.size == 13);
  size_t i = 1;
  for (; i < 4; ++i) {
    assert(chunks.chunks[i].type == stufflib_png_IDAT);
    assert(chunks.chunks[i].data.size == (1 << 13));
  }
  assert(chunks.chunks[i].type == stufflib_png_IDAT);
  assert(chunks.chunks[i].data.size == 64);
  assert(chunks.chunks[i + 1].type == stufflib_png_IEND);
  assert(chunks.chunks[i + 1].data.size == 0);
  stufflib_png_chunks_destroy(chunks);
  return 0;
}

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
    assert(pixel.color_type == stufflib_png_rgb);
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
    assert(white_square.color_type == stufflib_png_rgba);
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
    assert(github_squares.color_type == stufflib_png_rgb);
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
    assert(github_profile.color_type == stufflib_png_rgb);
  }

  {
    const char* png_path = "./test-data/asan.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_header asan = stufflib_png_read_header(png_path);
    if (!asan.width) {
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_header(stdout, asan);
    }
    assert(asan.width == 958);
    assert(asan.height == 458);
    assert(asan.bit_depth == 8);
    assert(asan.color_type == stufflib_png_rgb);
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
  if (!pixel.data.size) {
    fprintf(stderr, "failed reading PNG image %s\n", png_path);
    return 1;
  }
  if (verbose) {
    stufflib_png_dump_img_meta(stdout, pixel);
  }
  assert(pixel.header.width == 1);
  assert(pixel.header.height == 1);
  assert(pixel.header.bit_depth == 8);
  assert(pixel.header.color_type == stufflib_png_rgb);

  assert(pixel.data.size == 4);
  unsigned char expected_data[4] = {0};
  expected_data[on_index] = 255;
  for (size_t c = 0; c < 4; ++c) {
    assert(pixel.data.data[c] == expected_data[c]);
  }

  stufflib_png_image_destroy(pixel);
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

int _test_read_single_pixel_with_fixed_compression(const int verbose) {
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

int _test_read_small_images_with_dynamic_compression(const int verbose) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/0099ee-80x160-rgb-dynamic.png",
        "./test-data/cc1177-80x160-rgb-dynamic.png",
        "./test-data/ffaa55-80x160-rgb-dynamic.png",
    }[i];
    const size_t rgb = (size_t[]){
        0x0099ee,
        0xcc1177,
        0xffaa55,
    }[i];
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_image img = stufflib_png_read_image(png_path);
    if (!img.data.size) {
      fprintf(stderr, "failed reading PNG image %s\n", png_path);
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, img);
    }
    const size_t width = 80;
    const size_t height = 160;
    assert(img.header.width == width);
    assert(img.header.height == height);
    assert(img.header.bit_depth == 8);
    assert(img.header.color_type == stufflib_png_rgb);

    assert(img.data.size == height + width * height * 3);
    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        const size_t i = y + 1 + 3 * (y * width + x);
        const unsigned r = img.data.data[i + 0];
        const unsigned g = img.data.data[i + 1];
        const unsigned b = img.data.data[i + 2];
        printf("y%zu x%zu i%zu r%u g%u b%u %zu\n", y, x, i, r, g, b, rgb);
        assert(r == ((rgb & 0xff0000) >> 16));
        assert(g == ((rgb & 0x00ff00) >> 8));
        assert(b == ((rgb & 0x0000ff) >> 0));
      }
    }
    stufflib_png_image_destroy(img);
  }
  return 0;
}

int _test_read_large_images_with_dynamic_compression(const int verbose) {
  {
    const char* png_path = "./test-data/aabbcc-1600x1600-rgb-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_image square = stufflib_png_read_image(png_path);
    if (!square.data.size) {
      fprintf(stderr, "failed reading PNG image %s\n", png_path);
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, square);
    }
    const size_t size = 1600;
    assert(square.header.width == size);
    assert(square.header.height == size);
    assert(square.header.bit_depth == 8);
    assert(square.header.color_type == stufflib_png_rgb);
    assert(square.data.size == size + size * size * 3);
    for (size_t y = 0; y < size; ++y) {
      for (size_t x = 0; x < size; ++x) {
        const size_t i = y + 1 + y * size * 3 + x * 3;
        assert(square.data.data[i + 0] == 0xaa);
        assert(square.data.data[i + 1] == 0xbb);
        assert(square.data.data[i + 2] == 0xcc);
      }
    }
    stufflib_png_image_destroy(square);
  }
  {
    const char* png_path = "./test-data/asan.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_image asan = stufflib_png_read_image(png_path);
    if (!asan.data.size) {
      fprintf(stderr, "failed reading PNG image %s\n", png_path);
      return 1;
    }
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, asan);
    }
    assert(asan.header.width == 958);
    assert(asan.header.height == 458);
    assert(asan.header.bit_depth == 8);
    assert(asan.header.color_type == stufflib_png_rgb);
    assert(asan.data.size == 24640);
    stufflib_png_image_destroy(asan);
  }
  return 0;
}

typedef int test_function(const int);

int main(int argc, char* const argv[argc + 1]) {
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  test_function* tests[] = {
      _test_read_single_pixel_chunks,
      _test_read_large_image_with_many_chunks,
      _test_read_single_pixel_header,
      _test_read_img_header,
      _test_read_single_pixel_no_compression,
      _test_read_single_pixel_with_fixed_compression,
      _test_read_small_images_with_dynamic_compression,
      _test_read_large_images_with_dynamic_compression,
  };
  const char* test_names[] = {
      "read_single_pixel_chunks",
      "read_large_image_with_many_chunks",
      "read_single_pixel_header",
      "read_img_header",
      "read_single_pixel_no_compression",
      "read_single_pixel_with_fixed_compression",
      "read_small_images_with_dynamic_compression",
      "read_large_images_with_dynamic_compression",
  };

  for (size_t t = 0; t < STUFFLIB_ARRAY_LEN(tests); ++t) {
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
