#include <stdio.h>
#include <stdlib.h>

#include "stufflib_png.h"

int main(int argc, char* const argv[argc + 1]) {
  if (!(argc == 2 || argc == 3)) {
    fprintf(stderr, "usage: %s png_path [data_dump_count]\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char* png_path = argv[1];
  const size_t data_dump_count = argc == 3 ? strtoull(argv[2], 0, 10) : 0;

  struct stufflib_png_image img = stufflib_png_read(png_path);
  if (!img.size) {
    return EXIT_FAILURE;
  }

  printf("FILE\n  %s\n", png_path);
  printf("HEADER\n");
  stufflib_png_dump_img_meta(stdout, img);

  if (data_dump_count) {
    printf("DATA\n");
    stufflib_png_dump_img_data(stdout, img, data_dump_count);
  }

  free(img.data);

  return EXIT_SUCCESS;
}
