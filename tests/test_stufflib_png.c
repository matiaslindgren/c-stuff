#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stufflib/args/args.h>
#include <stufflib/context/context.h>
#include <stufflib/macros/macros.h>
#include <stufflib/misc/misc.h>
#include <stufflib/png/png.h>
#include <stufflib/testing/testing.h>

static bool test_read_single_pixel_chunks(struct sl_context ctx[static 1], const bool verbose) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/png/ff0000-1x1-rgb-nocomp.png",
        "./test-data/png/00ff00-1x1-rgb-nocomp.png",
        "./test-data/png/0000ff-1x1-rgb-nocomp.png",
    }[i];
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_chunks chunks = sl_png_read_chunks(ctx, png_path);
    SL_ASSERT_TRUE(chunks.count == 3);
    SL_ASSERT_TRUE(chunks.chunks[0].type == sl_png_IHDR);
    SL_ASSERT_TRUE(chunks.chunks[0].data.size == 13);
    SL_ASSERT_TRUE(chunks.chunks[1].type == sl_png_IDAT);
    SL_ASSERT_TRUE(chunks.chunks[1].data.size == 15);
    SL_ASSERT_TRUE(chunks.chunks[2].type == sl_png_IEND);
    SL_ASSERT_TRUE(chunks.chunks[2].data.size == 0);
    sl_png_chunks_destroy(chunks);
  }
  return true;
}

static bool
test_read_large_image_with_many_chunks(struct sl_context ctx[static 1], const bool verbose) {
  const char* png_path = "./test-data/png/asan.png";
  if (verbose) {
    printf("%s\n", png_path);
  }
  struct sl_png_chunks chunks = sl_png_read_chunks(ctx, png_path);
  SL_ASSERT_TRUE(chunks.count == 6);
  SL_ASSERT_TRUE(chunks.chunks[0].type == sl_png_IHDR);
  SL_ASSERT_TRUE(chunks.chunks[0].data.size == 13);
  size_t i = 1;
  for (; i < 4; ++i) {
    SL_ASSERT_TRUE(chunks.chunks[i].type == sl_png_IDAT);
    SL_ASSERT_TRUE(chunks.chunks[i].data.size == (1 << 13));
  }
  SL_ASSERT_TRUE(chunks.chunks[i].type == sl_png_IDAT);
  SL_ASSERT_TRUE(chunks.chunks[i].data.size == 64);
  SL_ASSERT_TRUE(chunks.chunks[i + 1].type == sl_png_IEND);
  SL_ASSERT_TRUE(chunks.chunks[i + 1].data.size == 0);
  sl_png_chunks_destroy(chunks);
  return true;
}

static bool test_read_single_pixel_header(struct sl_context ctx[static 1], const bool verbose) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/png/ff0000-1x1-rgb-nocomp.png",
        "./test-data/png/00ff00-1x1-rgb-nocomp.png",
        "./test-data/png/0000ff-1x1-rgb-nocomp.png",
    }[i];
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_header pixel = sl_png_read_header(ctx, png_path);
    if (!pixel.width) {
      return false;
    }
    if (verbose) {
      sl_png_dump_header(stdout, pixel);
    }
    SL_ASSERT_TRUE(pixel.width == 1);
    SL_ASSERT_TRUE(pixel.height == 1);
    SL_ASSERT_TRUE(pixel.bit_depth == 8);
    SL_ASSERT_TRUE(pixel.color_type == sl_png_rgb);
  }
  return true;
}

static bool test_read_img_header(struct sl_context ctx[static 1], const bool verbose) {
  {
    const char* png_path = "./test-data/png/white-square-rgba-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_header white_square = sl_png_read_header(ctx, png_path);
    if (!white_square.width) {
      return false;
    }
    if (verbose) {
      sl_png_dump_header(stdout, white_square);
    }
    SL_ASSERT_TRUE(white_square.width == 26);
    SL_ASSERT_TRUE(white_square.height == 28);
    SL_ASSERT_TRUE(white_square.bit_depth == 8);
    SL_ASSERT_TRUE(white_square.color_type == sl_png_rgba);
  }

  {
    const char* png_path = "./test-data/png/github-squares-rgb-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_header github_squares = sl_png_read_header(ctx, png_path);
    if (!github_squares.width) {
      return false;
    }
    if (verbose) {
      sl_png_dump_header(stdout, github_squares);
    }
    SL_ASSERT_TRUE(github_squares.width == 81);
    SL_ASSERT_TRUE(github_squares.height == 77);
    SL_ASSERT_TRUE(github_squares.bit_depth == 8);
    SL_ASSERT_TRUE(github_squares.color_type == sl_png_rgb);
  }

  {
    const char* png_path = "./test-data/png/github-profile-rgb-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_header github_profile = sl_png_read_header(ctx, png_path);
    if (!github_profile.width) {
      return false;
    }
    if (verbose) {
      sl_png_dump_header(stdout, github_profile);
    }
    SL_ASSERT_TRUE(github_profile.width == 420);
    SL_ASSERT_TRUE(github_profile.height == 420);
    SL_ASSERT_TRUE(github_profile.bit_depth == 8);
    SL_ASSERT_TRUE(github_profile.color_type == sl_png_rgb);
  }

  {
    const char* png_path = "./test-data/png/asan.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_header asan = sl_png_read_header(ctx, png_path);
    if (!asan.width) {
      return false;
    }
    if (verbose) {
      sl_png_dump_header(stdout, asan);
    }
    SL_ASSERT_TRUE(asan.width == 958);
    SL_ASSERT_TRUE(asan.height == 458);
    SL_ASSERT_TRUE(asan.bit_depth == 8);
    SL_ASSERT_TRUE(asan.color_type == sl_png_rgb);
  }

  return true;
}

static inline int test_read_single_pixel(
    struct sl_context ctx[static 1],
    const char* png_path,
    const size_t on_index,
    const bool verbose
) {
  if (verbose) {
    printf("%s\n", png_path);
  }
  struct sl_png_image pixel = sl_png_read_image(ctx, png_path);
  if (!pixel.data.size) {
    SL_LOG_ERROR("failed reading PNG image %s", png_path);
    return false;
  }
  if (verbose) {
    sl_png_dump_img_meta(stdout, pixel);
  }
  SL_ASSERT_TRUE(pixel.header.width == 1);
  SL_ASSERT_TRUE(pixel.header.height == 1);
  SL_ASSERT_TRUE(pixel.header.bit_depth == 8);
  SL_ASSERT_TRUE(pixel.header.color_type == sl_png_rgb);

  SL_ASSERT_TRUE(pixel.data.size == 9 * 3);
  unsigned char expected_data[9 * 3] = {0};
  expected_data[4 * 3 + on_index]    = 255;
  for (size_t i = 0; i < SL_ARRAY_LEN(expected_data); ++i) {
    SL_ASSERT_TRUE(pixel.data.data[i] == expected_data[i]);
  }

  sl_png_image_destroy(pixel);
  return true;
}

static bool
test_read_single_pixel_no_compression(struct sl_context ctx[static 1], const bool verbose) {
  for (size_t on_pixel = 0; on_pixel < 3; ++on_pixel) {
    const char* png_path = (const char*[]){
        "./test-data/png/ff0000-1x1-rgb-nocomp.png",
        "./test-data/png/00ff00-1x1-rgb-nocomp.png",
        "./test-data/png/0000ff-1x1-rgb-nocomp.png",
    }[on_pixel];
    if (!test_read_single_pixel(ctx, png_path, on_pixel, verbose)) {
      return false;
    }
  }
  return true;
}

static bool
test_read_single_pixel_with_fixed_compression(struct sl_context ctx[static 1], const bool verbose) {
  for (size_t on_pixel = 0; on_pixel < 3; ++on_pixel) {
    const char* png_path = (const char*[]){
        "./test-data/png/ff0000-1x1-rgb-fixed.png",
        "./test-data/png/00ff00-1x1-rgb-fixed.png",
        "./test-data/png/0000ff-1x1-rgb-fixed.png",
    }[on_pixel];
    if (!test_read_single_pixel(ctx, png_path, on_pixel, verbose)) {
      return false;
    }
  }
  return true;
}

static bool
test_read_rgba_image_with_dynamic_compression(struct sl_context ctx[static 1], const bool verbose) {
  const char* png_path = "./test-data/png/white-square-rgba-dynamic.png";
  if (verbose) {
    printf("%s\n", png_path);
  }
  struct sl_png_image white_square = sl_png_read_image(ctx, png_path);
  if (!white_square.data.size) {
    return false;
  }
  if (verbose) {
    sl_png_dump_img_meta(stdout, white_square);
  }
  const size_t width  = 26;
  const size_t height = 28;
  const size_t bpp    = 4;
  SL_ASSERT_TRUE(white_square.header.width == width);
  SL_ASSERT_TRUE(white_square.header.height == height);
  SL_ASSERT_TRUE(white_square.header.bit_depth == 8);
  SL_ASSERT_TRUE(white_square.header.color_type == sl_png_rgba);
  SL_ASSERT_TRUE(white_square.data.size == bpp * (width + 2) * (height + 2));
  for (size_t y = 1; y < height + 1; ++y) {
    for (size_t x = 1; x < width + 1; ++x) {
      const unsigned char* px = white_square.data.data + bpp * (y * (width + 2) + x);
      for (size_t byte = 0; byte < bpp; ++byte) {
        SL_ASSERT_TRUE(px[byte] == 0xff);
      }
    }
  }
  sl_png_image_destroy(white_square);
  return true;
}

static bool test_read_small_images_with_dynamic_compression(
    struct sl_context ctx[static 1],
    const bool verbose
) {
  for (size_t i = 0; i < 3; ++i) {
    const char* png_path = (const char*[]){
        "./test-data/png/0099ee-80x160-rgb-dynamic.png",
        "./test-data/png/cc1177-80x160-rgb-dynamic.png",
        "./test-data/png/ffaa55-80x160-rgb-dynamic.png",
    }[i];
    const size_t rgb = (size_t[]){
        0x0099ee,
        0xcc1177,
        0xffaa55,
    }[i];
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_image img = sl_png_read_image(ctx, png_path);
    if (!img.data.size) {
      SL_LOG_ERROR("failed reading PNG image %s", png_path);
      return false;
    }
    if (verbose) {
      sl_png_dump_img_meta(stdout, img);
    }
    const size_t width  = 80;
    const size_t height = 160;
    SL_ASSERT_TRUE(img.header.width == width);
    SL_ASSERT_TRUE(img.header.height == height);
    SL_ASSERT_TRUE(img.header.bit_depth == 8);
    SL_ASSERT_TRUE(img.header.color_type == sl_png_rgb);
    SL_ASSERT_TRUE(img.data.size == 3 * (width + 2) * (height + 2));
    for (size_t y = 1; y < height + 1; ++y) {
      for (size_t x = 1; x < width + 1; ++x) {
        const unsigned char* px = img.data.data + 3 * (y * (width + 2) + x);
        SL_ASSERT_TRUE(px[0] == ((rgb & 0xff0000) >> 16));
        SL_ASSERT_TRUE(px[1] == ((rgb & 0x00ff00) >> 8));
        SL_ASSERT_TRUE(px[2] == ((rgb & 0x0000ff) >> 0));
      }
    }
    sl_png_image_destroy(img);
  }
  return true;
}

static bool test_read_large_images_with_dynamic_compression(
    struct sl_context ctx[static 1],
    const bool verbose
) {
  {
    const char* png_path = "./test-data/png/aabbcc-1600x1600-rgb-dynamic.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_image square = sl_png_read_image(ctx, png_path);
    if (!square.data.size) {
      SL_LOG_ERROR("failed reading PNG image %s", png_path);
      return false;
    }
    if (verbose) {
      sl_png_dump_img_meta(stdout, square);
    }
    const size_t size = 1600;
    SL_ASSERT_TRUE(square.header.width == size);
    SL_ASSERT_TRUE(square.header.height == size);
    SL_ASSERT_TRUE(square.header.bit_depth == 8);
    SL_ASSERT_TRUE(square.header.color_type == sl_png_rgb);
    SL_ASSERT_TRUE(square.data.size == 3 * (size + 2) * (size + 2));
    for (size_t y = 1; y < size + 1; ++y) {
      for (size_t x = 1; x < size + 1; ++x) {
        const unsigned char* px = square.data.data + 3 * (y * (size + 2) + x);
        SL_ASSERT_TRUE(px[0] == 0xaa);
        SL_ASSERT_TRUE(px[1] == 0xbb);
        SL_ASSERT_TRUE(px[2] == 0xcc);
      }
    }
    sl_png_image_destroy(square);
  }

  {
    const char* png_path = "./test-data/png/asan.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct sl_png_image asan = sl_png_read_image(ctx, png_path);
    if (!asan.data.size) {
      SL_LOG_ERROR("failed reading PNG image %s", png_path);
      return false;
    }
    if (verbose) {
      sl_png_dump_img_meta(stdout, asan);
    }
    const size_t width  = 958;
    const size_t height = 458;
    SL_ASSERT_TRUE(asan.header.width == width);
    SL_ASSERT_TRUE(asan.header.height == height);
    SL_ASSERT_TRUE(asan.header.bit_depth == 8);
    SL_ASSERT_TRUE(asan.header.color_type == sl_png_rgb);
    SL_ASSERT_TRUE(asan.filter.size == height);
    SL_ASSERT_TRUE(asan.data.size == 3 * (width + 2) * (height + 2));
    sl_png_image_destroy(asan);
  }
  return true;
}

int test_read_write_read(
    struct sl_context ctx[static 1],
    const bool verbose,
    const char* img0_path
) {
  if (verbose) {
    printf("%s\n", img0_path);
  }
  struct sl_png_image img0 = sl_png_read_image(ctx, img0_path);
  if (!img0.data.size) {
    SL_LOG_ERROR("failed reading PNG image %s", img0_path);
    return false;
  }
  if (verbose) {
    sl_png_dump_img_meta(stdout, img0);
  }

  char img1_path[200] = {0};
  {
    int ok = snprintf(img1_path, SL_ARRAY_LEN(img1_path), "%s/sl_test.png", sl_misc_tmpdir());
    SL_ASSERT_TRUE(ok > 0);
  }
  SL_ASSERT_TRUE(sl_png_write_image(ctx, img0, img1_path));

  struct sl_png_image img1 = sl_png_read_image(ctx, img1_path);
  if (img1.data.size != img0.data.size) {
    fprintf(
        stderr,
        "written %s img size %zu is not equal to size %zu of original %s\n",
        img1_path,
        img1.data.size,
        img0.data.size,
        img0_path
    );
    return false;
  }
  if (verbose) {
    sl_png_dump_img_meta(stdout, img1);
  }
  sl_png_image_destroy(img1);
  sl_png_image_destroy(img0);
  return true;
}

static bool test_read_write_read_single_pixel(struct sl_context ctx[static 1], const bool verbose) {
  const char* paths[] = {
      "./test-data/png/ff0000-1x1-rgb-nocomp.png",
      "./test-data/png/00ff00-1x1-rgb-nocomp.png",
      "./test-data/png/0000ff-1x1-rgb-nocomp.png",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(paths); ++i) {
    const char* png_path = paths[i];
    if (!test_read_write_read(ctx, verbose, png_path)) {
      return false;
    }
  }
  return true;
}

static bool test_read_write_read_small(struct sl_context ctx[static 1], const bool verbose) {
  const char* paths[] = {
      "./test-data/png/0099ee-80x160-rgb-dynamic.png",
      "./test-data/png/cc1177-80x160-rgb-dynamic.png",
      "./test-data/png/ffaa55-80x160-rgb-dynamic.png",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(paths); ++i) {
    const char* png_path = paths[i];
    if (!test_read_write_read(ctx, verbose, png_path)) {
      return false;
    }
  }
  return true;
}

static bool test_read_write_read_large(struct sl_context ctx[static 1], const bool verbose) {
  const char* paths[] = {
      "./test-data/png/aabbcc-1600x1600-rgb-dynamic.png",
      "./test-data/png/github-profile-rgb-dynamic.png",
      "./test-data/png/asan.png",
  };
  for (size_t i = 0; i < SL_ARRAY_LEN(paths); ++i) {
    const char* png_path = paths[i];
    if (!test_read_write_read(ctx, verbose, png_path)) {
      return false;
    }
  }
  return true;
}

SL_TEST_MAIN(
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
    test_read_write_read_large
)
