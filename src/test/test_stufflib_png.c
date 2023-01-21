#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"
#include "stufflib_png.h"

int test_read_single_pixel_chunks(const int verbose) {
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
  return 1;
}

int test_read_large_image_with_many_chunks(const int verbose) {
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
  return 1;
}

int test_read_single_pixel_header(const int verbose) {
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
      return 0;
    }
    if (verbose) {
      stufflib_png_dump_header(stdout, pixel);
    }
    assert(pixel.width == 1);
    assert(pixel.height == 1);
    assert(pixel.bit_depth == 8);
    assert(pixel.color_type == stufflib_png_rgb);
  }
  return 1;
}

int test_read_img_header(const int verbose) {
  {
    const char* png_path = "./test-data/white-square-rgba-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_header white_square = stufflib_png_read_header(png_path);
    if (!white_square.width) {
      return 0;
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
      return 0;
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
      return 0;
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
      return 0;
    }
    if (verbose) {
      stufflib_png_dump_header(stdout, asan);
    }
    assert(asan.width == 958);
    assert(asan.height == 458);
    assert(asan.bit_depth == 8);
    assert(asan.color_type == stufflib_png_rgb);
  }

  return 1;
}

static inline int _test_read_single_pixel(const char* png_path,
                                          const size_t on_index,
                                          const int verbose) {
  if (verbose) {
    printf("%s\n", png_path);
  }
  stufflib_png_image pixel = stufflib_png_read_image(png_path);
  if (!pixel.data.size) {
    STUFFLIB_PRINT_ERROR("failed reading PNG image %s", png_path);
    return 0;
  }
  if (verbose) {
    stufflib_png_dump_img_meta(stdout, pixel);
  }
  assert(pixel.header.width == 1);
  assert(pixel.header.height == 1);
  assert(pixel.header.bit_depth == 8);
  assert(pixel.header.color_type == stufflib_png_rgb);

  assert(pixel.data.size == 9 * 3);
  unsigned char expected_data[9 * 3] = {0};
  expected_data[4 * 3 + on_index] = 255;
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(expected_data); ++i) {
    assert(pixel.data.data[i] == expected_data[i]);
  }

  stufflib_png_image_destroy(pixel);
  return 1;
}

int test_read_single_pixel_no_compression(const int verbose) {
  for (size_t on_pixel = 0; on_pixel < 3; ++on_pixel) {
    const char* png_path = (const char*[]){
        "./test-data/ff0000-1x1-rgb-nocomp.png",
        "./test-data/00ff00-1x1-rgb-nocomp.png",
        "./test-data/0000ff-1x1-rgb-nocomp.png",
    }[on_pixel];
    if (!_test_read_single_pixel(png_path, on_pixel, verbose)) {
      return 0;
    }
  }
  return 1;
}

int test_read_single_pixel_with_fixed_compression(const int verbose) {
  for (size_t on_pixel = 0; on_pixel < 3; ++on_pixel) {
    const char* png_path = (const char*[]){
        "./test-data/ff0000-1x1-rgb-fixed.png",
        "./test-data/00ff00-1x1-rgb-fixed.png",
        "./test-data/0000ff-1x1-rgb-fixed.png",
    }[on_pixel];
    if (!_test_read_single_pixel(png_path, on_pixel, verbose)) {
      return 0;
    }
  }
  return 1;
}

int test_read_rgba_image_with_dynamic_compression(const int verbose) {
  const char* png_path = "./test-data/white-square-rgba-dynamic.png";
  if (verbose) {
    printf("%s\n", png_path);
  }
  stufflib_png_image white_square = stufflib_png_read_image(png_path);
  if (!white_square.data.size) {
    return 0;
  }
  if (verbose) {
    stufflib_png_dump_img_meta(stdout, white_square);
  }
  const size_t width = 26;
  const size_t height = 28;
  const size_t bpp = 4;
  assert(white_square.header.width == width);
  assert(white_square.header.height == height);
  assert(white_square.header.bit_depth == 8);
  assert(white_square.header.color_type == stufflib_png_rgba);
  assert(white_square.data.size == bpp * (width + 2) * (height + 2));
  for (size_t y = 1; y < height + 1; ++y) {
    for (size_t x = 1; x < width + 1; ++x) {
      const unsigned char* px =
          white_square.data.data + bpp * (y * (width + 2) + x);
      for (size_t byte = 0; byte < bpp; ++byte) {
        assert(px[byte] == 0xff);
      }
    }
  }
  stufflib_png_image_destroy(white_square);
  return 1;
}

int test_read_small_images_with_dynamic_compression(const int verbose) {
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
      STUFFLIB_PRINT_ERROR("failed reading PNG image %s", png_path);
      return 0;
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
    assert(img.data.size == 3 * (width + 2) * (height + 2));
    for (size_t y = 1; y < height + 1; ++y) {
      for (size_t x = 1; x < width + 1; ++x) {
        const unsigned char* px = img.data.data + 3 * (y * (width + 2) + x);
        assert(px[0] == ((rgb & 0xff0000) >> 16));
        assert(px[1] == ((rgb & 0x00ff00) >> 8));
        assert(px[2] == ((rgb & 0x0000ff) >> 0));
      }
    }
    stufflib_png_image_destroy(img);
  }
  return 1;
}

int test_read_large_images_with_dynamic_compression(const int verbose) {
  {
    const char* png_path = "./test-data/aabbcc-1600x1600-rgb-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    stufflib_png_image square = stufflib_png_read_image(png_path);
    if (!square.data.size) {
      STUFFLIB_PRINT_ERROR("failed reading PNG image %s", png_path);
      return 0;
    }
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, square);
    }
    const size_t size = 1600;
    assert(square.header.width == size);
    assert(square.header.height == size);
    assert(square.header.bit_depth == 8);
    assert(square.header.color_type == stufflib_png_rgb);
    assert(square.data.size == 3 * (size + 2) * (size + 2));
    for (size_t y = 1; y < size + 1; ++y) {
      for (size_t x = 1; x < size + 1; ++x) {
        const unsigned char* px = square.data.data + 3 * (y * (size + 2) + x);
        assert(px[0] == 0xaa);
        assert(px[1] == 0xbb);
        assert(px[2] == 0xcc);
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
      STUFFLIB_PRINT_ERROR("failed reading PNG image %s", png_path);
      return 0;
    }
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, asan);
    }
    const size_t width = 958;
    const size_t height = 458;
    assert(asan.header.width == width);
    assert(asan.header.height == height);
    assert(asan.header.bit_depth == 8);
    assert(asan.header.color_type == stufflib_png_rgb);
    assert(asan.filter.size == height);
    assert(asan.data.size == 3 * (width + 2) * (height + 2));
    stufflib_png_image_destroy(asan);
  }
  return 1;
}

int _test_read_write_read(const int verbose, const char* img0_path) {
  if (verbose) {
    printf("%s\n", img0_path);
  }
  stufflib_png_image img0 = stufflib_png_read_image(img0_path);
  if (!img0.data.size) {
    STUFFLIB_PRINT_ERROR("failed reading PNG image %s", img0_path);
    return 0;
  }
  if (verbose) {
    stufflib_png_dump_img_meta(stdout, img0);
  }
  const char* img1_path = "/tmp/stufflib_test.png";
  assert(stufflib_png_write_image(img0, img1_path));
  stufflib_png_image img1 = stufflib_png_read_image(img1_path);
  if (img1.data.size != img0.data.size) {
    fprintf(stderr,
            "written %s img size %zu is not equal to size %zu of original %s\n",
            img1_path,
            img1.data.size,
            img0.data.size,
            img0_path);
    return 0;
  }
  if (verbose) {
    stufflib_png_dump_img_meta(stdout, img1);
  }
  stufflib_png_image_destroy(img1);
  stufflib_png_image_destroy(img0);
  return 1;
}

int test_read_write_read_single_pixel(const int verbose) {
  const char* paths[] = {
      "./test-data/ff0000-1x1-rgb-nocomp.png",
      "./test-data/00ff00-1x1-rgb-nocomp.png",
      "./test-data/0000ff-1x1-rgb-nocomp.png",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(paths); ++i) {
    const char* png_path = paths[i];
    if (!_test_read_write_read(verbose, png_path)) {
      return 0;
    }
  }
  return 1;
}

int test_read_write_read_small(const int verbose) {
  const char* paths[] = {
      "./test-data/0099ee-80x160-rgb-dynamic.png",
      "./test-data/cc1177-80x160-rgb-dynamic.png",
      "./test-data/ffaa55-80x160-rgb-dynamic.png",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(paths); ++i) {
    const char* png_path = paths[i];
    if (!_test_read_write_read(verbose, png_path)) {
      return 0;
    }
  }
  return 1;
}

int test_read_write_read_large(const int verbose) {
  const char* paths[] = {
      "./test-data/aabbcc-1600x1600-rgb-dynamic.png",
      "./test-data/github-profile-rgb-dynamic.png",
      "./test-data/asan.png",
  };
  for (size_t i = 0; i < STUFFLIB_ARRAY_LEN(paths); ++i) {
    const char* png_path = paths[i];
    if (!_test_read_write_read(verbose, png_path)) {
      return 0;
    }
  }
  return 1;
}

typedef int test_function(const int);

int main(int argc, char* const argv[argc + 1]) {
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  test_function* tests[] = {
      test_read_single_pixel_chunks,
      test_read_large_image_with_many_chunks,
      test_read_single_pixel_header,
      test_read_img_header,
      test_read_single_pixel_no_compression,
      test_read_single_pixel_with_fixed_compression,
      test_read_rgba_image_with_dynamic_compression,
      test_read_small_images_with_dynamic_compression,
      test_read_large_images_with_dynamic_compression,
      test_read_write_read_single_pixel,
      test_read_write_read_small,
      test_read_write_read_large,
  };
  const char* test_names[] = {
      "test_read_single_pixel_chunks",
      "test_read_large_image_with_many_chunks",
      "test_read_single_pixel_header",
      "test_read_img_header",
      "test_read_single_pixel_no_compression",
      "test_read_single_pixel_with_fixed_compression",
      "test_read_rgba_image_with_dynamic_compression",
      "test_read_small_images_with_dynamic_compression",
      "test_read_large_images_with_dynamic_compression",
      "test_read_write_read_single_pixel",
      "test_read_write_read_small",
      "test_read_write_read_large",
  };

  for (size_t t = 0; t < STUFFLIB_ARRAY_LEN(tests); ++t) {
    if (verbose) {
      printf("\n%s\n", test_names[t]);
    }
    if (!tests[t](verbose)) {
      STUFFLIB_PRINT_ERROR("test %s (%zu) failed", test_names[t], t);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
