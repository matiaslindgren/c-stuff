#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "stufflib_args.h"
#include "stufflib_img.h"
#include "stufflib_macros.h"
#include "stufflib_png.h"

bool segment(const sl_args args[const static 1]) {
  if (sl_args_count_positional(args) != 3) {
    SL_LOG_ERROR("too few arguments to PNG segment");
    return false;
  }

  bool ok = false;

  sl_png_image src = {0};
  sl_png_image dst = {0};

  const char* const png_src_path = sl_args_get_positional(args, 1);
  const char* const png_dst_path = sl_args_get_positional(args, 2);
  const size_t threshold_percent =
      sl_args_parse_uint(args, "--threshold-percent", 10);
  const bool verbose = sl_args_parse_flag(args, "-v");

  if (verbose) {
    printf("read %s\n", png_src_path);
  }
  src = sl_png_read_image(png_src_path);
  if (!src.data.size) {
    SL_LOG_ERROR("failed reading PNG image %s", png_src_path);
    goto done;
  }
  if (verbose) {
    printf("segmenting, threshold %zu%%\n", threshold_percent);
  }
  sl_img_segment_rgb(&dst, &src, threshold_percent);

  if (verbose) {
    printf("write %s\n", png_dst_path);
  }
  if (!sl_png_write_image(dst, png_dst_path)) {
    SL_LOG_ERROR("failed writing PNG image %s", png_dst_path);
    goto done;
  }

  ok = true;

done:
  sl_png_image_destroy(src);
  sl_png_image_destroy(dst);
  return ok;
}

bool info(const sl_args args[const static 1]) {
  if (sl_args_count_positional(args) != 2) {
    SL_LOG_ERROR("too few arguments to PNG info");
    return false;
  }

  bool ok = false;

  sl_png_chunks chunks = {0};
  sl_png_image img = {0};

  const char* const png_path = sl_args_get_positional(args, 1);

  printf("{");
  chunks = sl_png_read_chunks(png_path);
  printf("\"chunks\":");
  sl_png_dump_chunk_type_freq(stdout, chunks);

  sl_png_header header = sl_png_read_header(png_path);
  printf(",\"header\":");
  sl_png_dump_header(stdout, header);

  if (!sl_png_is_supported(header)) {
    SL_LOG_ERROR("PNG contains unsupported features, not reading IDAT chunks");
    ok = true;
    goto done;
  }

  img = sl_png_read_image(png_path);
  printf(",\"data\":");
  sl_png_dump_img_data_info(stdout, img);
  ok = true;

done:
  printf("}\n");
  sl_png_chunks_destroy(chunks);
  sl_png_image_destroy(img);
  return ok;
}

bool dump_raw(const sl_args args[const static 1]) {
  if (sl_args_count_positional(args) < 3) {
    SL_LOG_ERROR("too few arguments to PNG dump_raw");
    return false;
  }

  bool ok = false;

  const char* const png_path = sl_args_get_positional(args, 1);
  sl_png_chunks img_chunks = sl_png_read_chunks(png_path);
  if (!img_chunks.count) {
    SL_LOG_ERROR("failed reading PNG IDAT chunks from %s", png_path);
    goto done;
  }

  for (size_t i = 0; i < img_chunks.count; ++i) {
    for (size_t i_arg = 2;; ++i_arg) {
      const char* const block_type = sl_args_get_positional(args, i_arg);
      if (!block_type) {
        break;
      }
      const sl_png_chunk chunk = img_chunks.chunks[i];
      if (strncmp(sl_png_chunk_types[chunk.type], block_type, 4) == 0) {
        fwrite(chunk.data.data,
               sizeof(chunk.data.data[0]),
               chunk.data.size,
               stdout);
        break;
      }
    }
  }

  ok = true;

done:
  sl_png_chunks_destroy(img_chunks);
  return ok;
}

void print_usage(const sl_args args[const static 1]) {
  fprintf(
      stderr,
      ("usage:"
       "\n"
       "   %s info png_path"
       "\n"
       "   %s dump_raw png_path block_type [block_types...]"
       "\n"
       "   %s segment png_src_path png_dst_path [--threshold-percent=N] [-v]"
       "\n"),
      args->program,
      args->program,
      args->program);
}

int main(int argc, char* const argv[argc + 1]) {
  sl_args args = sl_args_from_argv(argc, argv);
  bool ok = false;
  const char* command = sl_args_get_positional(&args, 0);
  if (command) {
    if (strcmp(command, "segment") == 0) {
      ok = segment(&args);
    } else if (strcmp(command, "info") == 0) {
      ok = info(&args);
    } else if (strcmp(command, "dump_raw") == 0) {
      ok = dump_raw(&args);
    } else {
      SL_LOG_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  sl_args_destroy(&args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
