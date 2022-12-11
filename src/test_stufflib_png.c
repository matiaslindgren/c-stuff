#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib.h"
#include "stufflib_png.h"

int main(int argc, char* const argv[argc + 1]) {
  int verbose = stufflib_parse_argv_flag(argc, argv, "-v");

  {
    const char* png_path = "./test-data/green-pixel.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct stufflib_png_image green_pixel = stufflib_png_read(png_path);
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, green_pixel);
    }
    assert(green_pixel.header.width == 1);
    assert(green_pixel.header.height == 1);
    assert(green_pixel.header.bit_depth == 8);
    assert(green_pixel.header.color_type == 2);
    free(green_pixel.data);
  }

  {
    const char* png_path = "./test-data/white-img.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct stufflib_png_image white_img = stufflib_png_read(png_path);
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, white_img);
    }
    assert(white_img.header.width == 32);
    assert(white_img.header.height == 64);
    assert(white_img.header.bit_depth == 8);
    assert(white_img.header.color_type == 2);
    free(white_img.data);
  }

  {
    const char* png_path = "./test-data/github-squares.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct stufflib_png_image github_squares = stufflib_png_read(png_path);
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, github_squares);
    }
    assert(github_squares.header.width == 81);
    assert(github_squares.header.height == 77);
    assert(github_squares.header.bit_depth == 8);
    assert(github_squares.header.color_type == 6);
    free(github_squares.data);
  }

  {
    const char* png_path = "./test-data/github-profile.png";
    if (verbose) {
      printf("%s\n", png_path);
    }
    struct stufflib_png_image github_profile = stufflib_png_read(png_path);
    if (verbose) {
      stufflib_png_dump_img_meta(stdout, github_profile);
    }
    assert(github_profile.header.width == 420);
    assert(github_profile.header.height == 420);
    assert(github_profile.header.bit_depth == 8);
    assert(github_profile.header.color_type == 2);
    free(github_profile.data);
  }

  return EXIT_SUCCESS;
}
