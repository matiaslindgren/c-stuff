#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_png.h"
#include "stufflib_unionfind.h"

int segment(const stufflib_args args[const static 1]) {
  if (stufflib_args_count_positional(args) != 3) {
    STUFFLIB_PRINT_ERROR("too few arguments to segment");
    return 0;
  }

  int ok = 0;

  stufflib_png_image src = {0};
  stufflib_png_image dst = {0};
  stufflib_unionfind segments = {0};
  size_t* segment_sizes = 0;
  double* segment_sums = 0;

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

  // padded
  const size_t width = src.header.width + 2;
  const size_t height = src.header.height + 2;
  const size_t bytes_per_px = 3;
  const double distance_threshold =
      stufflib_math_clamp(0, (double)(threshold_percent) / 100, 1);

  if (!stufflib_unionfind_init(&segments, width * height)) {
    STUFFLIB_PRINT_ERROR(
        "failed initializing union_find structure for segments");
    goto done;
  }
  segment_sizes = calloc(width * height, sizeof(size_t));
  if (!segment_sizes) {
    STUFFLIB_PRINT_ERROR("failed allocating memory for segment sizes");
    goto done;
  }
  segment_sums = calloc(bytes_per_px * width * height, sizeof(double));
  if (!segment_sums) {
    STUFFLIB_PRINT_ERROR("failed allocating memory for segment sums");
    goto done;
  }

  for (size_t row = 0; row < height; ++row) {
    for (size_t col = 0; col < width; ++col) {
      const unsigned char* pixel = stufflib_png_image_get_pixel(&src, row, col);
      const size_t idx = row * width + col;
      segment_sizes[idx] = 1;
      for (size_t byte = 0; byte < bytes_per_px; ++byte) {
        segment_sums[bytes_per_px * idx + byte] = (double)(pixel[byte]) / 255;
      }
    }
  }

  for (size_t iteration = 0, num_merges = SIZE_MAX; num_merges > 0;
       ++iteration) {
    num_merges = 0;
    clock_t iter_begin_clock = clock();
    if (verbose) {
      printf("iteration %zu\n", iteration);
    }
    for (size_t row = 2; row < height - 1; ++row) {
      for (size_t col = 2; col < width - 1; ++col) {
        const size_t cur_idx = row * width + col;
        const size_t cur_seg = stufflib_unionfind_find_root(&segments, cur_idx);
        const double* cur_sum = segment_sums + bytes_per_px * cur_seg;
        const double* cur_avg = stufflib_math_linalg_scalar_vmul(
            1 / (double)(segment_sizes[cur_seg]),
            bytes_per_px,
            (double[3]){0},
            cur_sum);

        double dist2left = distance_threshold;
        double dist2above = distance_threshold;

        const size_t left_idx = row * width + col - 1;
        const size_t left_seg =
            stufflib_unionfind_find_root(&segments, left_idx);
        if (left_seg != cur_seg) {
          const double* left_sum = segment_sums + bytes_per_px * left_seg;
          const double* left_avg = stufflib_math_linalg_scalar_vmul(
              1 / (double)(segment_sizes[left_seg]),
              bytes_per_px,
              (double[3]){0},
              left_sum);
          const double* left_diff = stufflib_math_linalg_vsub(bytes_per_px,
                                                              (double[3]){0},
                                                              left_avg,
                                                              cur_avg);
          dist2left = stufflib_math_linalg_norm2(bytes_per_px, left_diff);
        }

        const size_t above_idx = (row - 1) * width + col;
        const size_t above_seg =
            stufflib_unionfind_find_root(&segments, above_idx);
        if (above_seg != cur_seg) {
          const double* above_sum = segment_sums + bytes_per_px * above_seg;
          const double* above_avg = stufflib_math_linalg_scalar_vmul(
              1 / (double)(segment_sizes[above_seg]),
              bytes_per_px,
              (double[3]){0},
              above_sum);
          const double* above_diff = stufflib_math_linalg_vsub(bytes_per_px,
                                                               (double[3]){0},
                                                               above_avg,
                                                               cur_avg);
          dist2above = stufflib_math_linalg_norm2(bytes_per_px, above_diff);
        }

        if (dist2left < dist2above && dist2left < distance_threshold) {
          stufflib_unionfind_union(&segments, left_seg, cur_seg);
          segment_sizes[left_seg] += segment_sizes[cur_seg];
          stufflib_math_linalg_vadd_inplace(
              bytes_per_px,
              segment_sums + bytes_per_px * left_seg,
              segment_sums + bytes_per_px * cur_seg);
          ++num_merges;
        } else if (dist2above < distance_threshold) {
          stufflib_unionfind_union(&segments, above_seg, cur_seg);
          segment_sizes[above_seg] += segment_sizes[cur_seg];
          stufflib_math_linalg_vadd_inplace(
              bytes_per_px,
              segment_sums + bytes_per_px * above_seg,
              segment_sums + bytes_per_px * cur_seg);
          ++num_merges;
        }
      }
    }
    clock_t iter_end_clock = clock();
    if (verbose) {
      const double iter_msec =
          1e3 * ((double)(iter_end_clock - iter_begin_clock)) / CLOCKS_PER_SEC;
      printf("  merged %zu segments in %g msec\n", num_merges, iter_msec);
    }
  }

  stufflib_png_image_copy(&dst, &src);
  for (size_t row = 0; row < height; ++row) {
    for (size_t col = 0; col < width; ++col) {
      const size_t idx = row * width + col;
      const size_t seg = stufflib_unionfind_find_root(&segments, idx);
      const size_t src_row = seg / width;
      const size_t src_col = seg % width;
      stufflib_png_image_set_pixel(
          &dst,
          row,
          col,
          stufflib_png_image_get_pixel(&src, src_row, src_col));
    }
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
  stufflib_unionfind_destroy(&segments);
  free(segment_sizes);
  free(segment_sums);
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
  const char* const png_path = stufflib_args_get_positional(args, 1);
  img = stufflib_png_read_image(png_path);
  if (!img.data.size) {
    STUFFLIB_PRINT_ERROR("failed reading PNG image %s", png_path);
    goto done;
  }
  printf("file: %s\n", png_path);
  stufflib_png_dump_img_meta(stdout, img);
  printf("filters:\n");
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
