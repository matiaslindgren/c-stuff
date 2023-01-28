#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stufflib_args.h"
#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_png.h"
#include "stufflib_unionfind.h"

static stufflib_png_image segment(const stufflib_png_image src,
                                  const double threshold,
                                  const int verbose) {
  // padded
  const size_t width = src.header.width + 2;
  const size_t height = src.header.height + 2;
  const size_t bytes_per_px = 3;
  const double distance_threshold = stufflib_math_clamp(0, threshold, 1);

  stufflib_unionfind segments = {0};
  size_t* segment_sizes = calloc(width * height, sizeof(size_t));
  double* segment_sums = calloc(bytes_per_px * width * height, sizeof(double));
  assert(stufflib_unionfind_init(&segments, width * height));
  assert(segment_sizes);
  assert(segment_sums);
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

  free(segment_sizes);
  free(segment_sums);

  stufflib_png_image dst = {0};
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
  stufflib_unionfind_destroy(&segments);
  return dst;
}

int main(int argc, char* const argv[argc + 1]) {
  if (!(argc == 3 || argc == 4 || argc == 5)) {
    STUFFLIB_PRINT_ERROR(
        "usage: %s png_src_path png_dst_path [--threshold-percent=N] [-v]",
        argv[0]);
    return EXIT_FAILURE;
  }

  stufflib_args* args = stufflib_args_from_argv(argc, argv);

  const char* png_src_path = stufflib_args_get_positional(args, 0);
  const char* png_dst_path = stufflib_args_get_positional(args, 1);
  const size_t threshold =
      stufflib_args_parse_uint(args, "--threshold-percent", 10);
  const int verbose = stufflib_args_parse_flag(args, "-v");

  stufflib_args_destroy(args);
  args = 0;

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
  stufflib_png_image res = segment(img, (double)(threshold) / 100, verbose);

  if (verbose) {
    printf("write %s\n", png_dst_path);
  }
  int ok = stufflib_png_write_image(res, png_dst_path);

  stufflib_png_image_destroy(img);
  stufflib_png_image_destroy(res);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
