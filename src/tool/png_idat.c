#include <stdio.h>
#include <stdlib.h>

#include "stufflib_png.h"

int dump_chunk(stufflib_png_chunks chunks, const size_t index) {
  if (index >= chunks.count) {
    fprintf(stderr,
            "chunk index %zu should be less than chunk count %zu\n",
            index,
            chunks.count);
    return 0;
  }
  stufflib_png_chunk chunk = chunks.chunks[index];
  if (chunk.type != stufflib_png_IDAT) {
    fprintf(stderr,
            "chunk index %zu points to chunk of type %s, not IDAT\n",
            index,
            stufflib_png_chunk_types[chunk.type]);
    return 0;
  }
  fwrite(chunk.data.data, sizeof(chunk.data.data[0]), chunk.data.size, stdout);
  return 1;
}

int main(int argc, char* const argv[argc + 1]) {
  if (!(argc == 2 || argc == 3)) {
    fprintf(stderr, "usage: %s png_path [chunk_index]\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char* png_path = argv[1];
  stufflib_png_chunks chunks = stufflib_png_read_chunks(png_path);
  if (!chunks.count) {
    return EXIT_FAILURE;
  }

  int status = EXIT_SUCCESS;

  if (argc == 3) {
    const size_t chunk_index = strtoull(argv[2], 0, 10);
    if (!dump_chunk(chunks, chunk_index)) {
      status = EXIT_FAILURE;
    }
  } else {
    for (size_t i = 0; i < chunks.count; ++i) {
      stufflib_png_chunk chunk = chunks.chunks[i];
      if (chunk.type == stufflib_png_IDAT) {
        if (!dump_chunk(chunks, i)) {
          status = EXIT_FAILURE;
          break;
        }
      }
    }
  }

  stufflib_png_chunks_destroy(chunks);
  return status;
}
