#include <stdio.h>
#include <stdlib.h>

#include "stufflib_argv.h"
#include "stufflib_deflate.h"
#include "stufflib_png.h"

int main(int argc, char* const argv[argc + 1]) {
  if (!(argc == 2 || argc == 3)) {
    fprintf(stderr, "usage: %s png_path [-v]\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char* png_path = argv[1];
  const int verbose = stufflib_argv_parse_flag(argc, argv, "-v");

  printf("FILE\n  %s\n", png_path);

  {
    printf("CHUNKS\n");
    stufflib_png_chunks chunks = stufflib_png_read_chunks(png_path);
    if (!chunks.count) {
      return EXIT_FAILURE;
    }
    for (size_t i = 0; i < chunks.count; ++i) {
      stufflib_png_chunk chunk = chunks.chunks[i];
      printf("%3zu (%5zu bytes): %s\n",
             i,
             chunk.data.size,
             stufflib_png_chunk_types[chunk.type]);
    }
    stufflib_png_chunks_destroy(chunks);
  }

  {
    stufflib_png_image img = stufflib_png_read_image(png_path);
    if (!img.data.size) {
      return EXIT_FAILURE;
    }
    printf("HEADER\n");
    stufflib_png_dump_img_meta(stdout, img);
    printf("FILTER (%zu bytes)\n", img.filter.size);
    stufflib_png_dump_filter_freq(stdout, img.filter);
    if (verbose) {
      stufflib_misc_data_fdump(stdout, img.filter, 40);
    }
    printf("DATA (%zu bytes)\n", img.data.size);
    if (verbose) {
      stufflib_misc_data_fdump(stdout, img.data, 40);
    }
    stufflib_png_image_destroy(img);
  }

  return EXIT_SUCCESS;
}
