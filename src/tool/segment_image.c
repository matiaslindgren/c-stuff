#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_img.h"
#include "stufflib_macros.h"
#include "stufflib_png.h"

int main(int argc, char* const argv[argc + 1]) {
  if (!(argc == 3 || argc == 4 || argc == 5)) {
    STUFFLIB_PRINT_ERROR(
        "usage: %s png_src_path png_dst_path [--threshold-percent=N] [-v]",
        argv[0]);
    return EXIT_FAILURE;
  }

  const char* png_src_path = argv[1];
  const char* png_dst_path = argv[2];
  const size_t threshold =
      stufflib_argv_parse_uint(argc, argv, "--threshold-percent", 10);
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  if (verbose) {
    printf("read %s\n", png_src_path);
  }
  stufflib_png_image img = stufflib_png_read_image(png_src_path);
  if (!img.data.size) {
    return EXIT_FAILURE;
  }
  if (verbose) {
    printf("segmenting, threshold %zu%%\n", threshold);
  }
  stufflib_png_image res =
      stufflib_img_segment_rgb(img, (double)(threshold) / 100, verbose);

  if (verbose) {
    printf("write %s\n", png_dst_path);
  }
  int ok = stufflib_png_write_image(res, png_dst_path);

  stufflib_png_image_destroy(img);
  stufflib_png_image_destroy(res);

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
