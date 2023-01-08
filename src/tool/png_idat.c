#include <stdio.h>
#include <stdlib.h>

#include "stufflib_png.h"

int main(int argc, char* const argv[argc + 1]) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s png_path\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char* png_path = argv[1];
  struct stufflib_png_image img = stufflib_png_read(png_path);
  if (!img.size) {
    return EXIT_FAILURE;
  }
  fwrite(img.data, sizeof(img.data[0]), img.size, stdout);
  fflush(stdout);
  free(img.data);
  return EXIT_SUCCESS;
}
