#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_img.h"
#include "stufflib_macros.h"
#include "stufflib_png.h"

int segment(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 3) {
    STUFFLIB_PRINT_ERROR("too few arguments to segment");
    return 0;
  }

  int ok = 0;

  stufflib_png_image src = {0};
  stufflib_png_image dst = {0};

  const char* const png_src_path = stufflib_args_get_positional(args, 1);
  const char* const png_dst_path = stufflib_args_get_positional(args, 2);
  const size_t threshold_percent =
      stufflib_args_parse_uint(args, "--threshold-percent", 10);
  const int verbose = stufflib_args_parse_flag(args, "-v");

  if (verbose) {
    printf("read %s\n", png_src_path);
  }
  src = stufflib_png_read_image(png_src_path);
  if (!src.data.size) {
    STUFFLIB_PRINT_ERROR("failed reading PNG image %s", png_src_path);
    goto done;
  }
  if (verbose) {
    printf("segmenting, threshold %zu%%\n", threshold_percent);
  }
  if (!stufflib_img_segment_rgb(&dst, &src, threshold_percent)) {
    STUFFLIB_PRINT_ERROR("failed segmenting PNG image %s", png_src_path);
    goto done;
  }

  if (verbose) {
    printf("write %s\n", png_dst_path);
  }
  if (!stufflib_png_write_image(dst, png_dst_path)) {
    STUFFLIB_PRINT_ERROR("failed writing PNG image %s", png_dst_path);
    goto done;
  }

  ok = 1;

done:
  stufflib_png_image_destroy(src);
  stufflib_png_image_destroy(dst);
  return ok;
}

int png_info(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 2) {
    STUFFLIB_PRINT_ERROR("too few arguments to png_info");
    return 0;
  }

  int ok = 0;

  stufflib_png_image img = {0};
  stufflib_png_header header = {0};

  const char* const png_path = stufflib_args_get_positional(args, 1);
  header = stufflib_png_read_header(png_path);
  printf("FILE: %s\n", png_path);
  printf("HEADER:\n");
  stufflib_png_dump_header(stdout, header);

  if (!stufflib_png_is_supported(header)) {
    printf("PNG contains unsupported features, not reading IDAT chunks\n");
    ok = 1;
    goto done;
  }

  img = stufflib_png_read_image(png_path);
  printf("DATA:\n");
  stufflib_png_dump_img_data_info(stdout, img);
  printf("FILTERS:\n");
  stufflib_png_dump_filter_freq(stdout, img.filter);
  ok = 1;

done:
  stufflib_png_image_destroy(img);
  return ok;
}

int png_dump_raw(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 2) {
    STUFFLIB_PRINT_ERROR("too few arguments to png_dump_raw");
    return 0;
  }

  int ok = 0;

  const char* const png_path = stufflib_args_get_positional(args, 1);
  stufflib_png_chunks img_chunks = stufflib_png_read_chunks(png_path);
  if (!img_chunks.count) {
    STUFFLIB_PRINT_ERROR("failed reading PNG IDAT chunks from %s", png_path);
    goto done;
  }

  for (size_t i = 0; i < img_chunks.count; ++i) {
    const stufflib_png_chunk chunk = img_chunks.chunks[i];
    if (chunk.type == stufflib_png_IDAT) {
      fwrite(chunk.data.data,
             sizeof(chunk.data.data[0]),
             chunk.data.size,
             stdout);
    }
  }

  ok = 1;

done:
  stufflib_png_chunks_destroy(img_chunks);
  return ok;
}

void print_usage(const stufflib_args args[const static 1]) {
  fprintf(
      stderr,
      ("usage:"
       "\n"
       "   %s segment png_src_path png_dst_path [--threshold-percent=N] [-v]"
       "\n"
       "   %s png_info png_path"
       "\n"
       "   %s png_dump_raw png_path"
       "\n"),
      args->program,
      args->program,
      args->program);
}

int main(int argc, char* const argv[argc + 1]) {
  stufflib_args* args = stufflib_args_from_argv(argc, argv);
  int ok = 0;
  const char* command = stufflib_args_get_positional(args, 0);
  if (command) {
    if (strcmp(command, "segment") == 0) {
      ok = segment(args);
    } else if (strcmp(command, "png_info") == 0) {
      ok = png_info(args);
    } else if (strcmp(command, "png_dump_raw") == 0) {
      ok = png_dump_raw(args);
    } else {
      STUFFLIB_PRINT_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(args);
  }
  stufflib_args_destroy(args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
