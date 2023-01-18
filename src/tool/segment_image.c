#include <stdio.h>
#include <stdlib.h>

#include "stufflib_img.h"
#include "stufflib_png.h"

int main(int argc, char* const argv[argc + 1]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s png_src_path png_dst_path\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char* png_src_path = argv[1];
  stufflib_png_image img = stufflib_png_read_image(png_src_path);
  if (!img.data.size) {
    return EXIT_FAILURE;
  }
  stufflib_png_image res = stufflib_img_segmentation_rgb(img, 1, 0.5);
  const char* png_dst_path = argv[2];
  int ok = stufflib_png_write_image(img, png_dst_path);
  stufflib_png_image_destroy(img);
  stufflib_png_image_destroy(res);

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
