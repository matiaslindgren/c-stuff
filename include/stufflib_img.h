#ifndef SL_IMG_H_INCLUDED
#define SL_IMG_H_INCLUDED

#include "stufflib_macros.h"
#include "stufflib_math.h"
#include "stufflib_png.h"
#include "stufflib_unionfind.h"

void sl_img_segment_rgb(struct sl_png_image dst[const static 1],
                        const struct sl_png_image src[const static 1],
                        const size_t threshold_percent) {
  struct sl_unionfind segments = {0};
  size_t* segment_sizes = nullptr;
  double* segment_sums = nullptr;

  // padded
  const size_t width = src->header.width + 2;
  const size_t height = src->header.height + 2;
  const size_t bytes_per_px = 3;
  const double distance_threshold =
      sl_math_clamp(0, threshold_percent / 100.0, 1);

  sl_unionfind_init(&segments, width * height);
  segment_sizes = sl_alloc(width * height, sizeof(size_t));
  segment_sums = sl_alloc(bytes_per_px * width * height, sizeof(double));

  for (size_t row = 0; row < height; ++row) {
    for (size_t col = 0; col < width; ++col) {
      const unsigned char* pixel = sl_png_image_get_pixel(src, row, col);
      const size_t idx = row * width + col;
      segment_sizes[idx] = 1;
      for (size_t byte = 0; byte < bytes_per_px; ++byte) {
        segment_sums[bytes_per_px * idx + byte] = pixel[byte] / 255.0;
      }
    }
  }

  for (size_t num_merges = SIZE_MAX; num_merges > 0;) {
    num_merges = 0;
    for (size_t row = 2; row < height - 1; ++row) {
      for (size_t col = 2; col < width - 1; ++col) {
        const size_t cur_idx = row * width + col;
        const size_t cur_seg = sl_unionfind_find_root(&segments, cur_idx);
        const double* cur_sum = segment_sums + bytes_per_px * cur_seg;
        const double* cur_avg =
            sl_math_linalg_scalar_vmul(sl_math_inv(segment_sizes[cur_seg]),
                                       bytes_per_px,
                                       (double[3]){0},
                                       cur_sum);

        double dist2left = distance_threshold;
        double dist2above = distance_threshold;

        const size_t left_idx = row * width + col - 1;
        const size_t left_seg = sl_unionfind_find_root(&segments, left_idx);
        if (left_seg != cur_seg) {
          const double* left_sum = segment_sums + bytes_per_px * left_seg;
          const double* left_avg =
              sl_math_linalg_scalar_vmul(sl_math_inv(segment_sizes[left_seg]),
                                         bytes_per_px,
                                         (double[3]){0},
                                         left_sum);
          const double* left_diff = sl_math_linalg_vsub(bytes_per_px,
                                                        (double[3]){0},
                                                        left_avg,
                                                        cur_avg);
          dist2left = sl_math_linalg_norm2(bytes_per_px, left_diff);
        }

        const size_t above_idx = (row - 1) * width + col;
        const size_t above_seg = sl_unionfind_find_root(&segments, above_idx);
        if (above_seg != cur_seg) {
          const double* above_sum = segment_sums + bytes_per_px * above_seg;
          const double* above_avg =
              sl_math_linalg_scalar_vmul(sl_math_inv(segment_sizes[above_seg]),
                                         bytes_per_px,
                                         (double[3]){0},
                                         above_sum);
          const double* above_diff = sl_math_linalg_vsub(bytes_per_px,
                                                         (double[3]){0},
                                                         above_avg,
                                                         cur_avg);
          dist2above = sl_math_linalg_norm2(bytes_per_px, above_diff);
        }

        if (dist2left < dist2above && dist2left < distance_threshold) {
          sl_unionfind_union(&segments, left_seg, cur_seg);
          segment_sizes[left_seg] += segment_sizes[cur_seg];
          sl_math_linalg_vadd_inplace(bytes_per_px,
                                      segment_sums + bytes_per_px * left_seg,
                                      segment_sums + bytes_per_px * cur_seg);
          ++num_merges;
        } else if (dist2above < distance_threshold) {
          sl_unionfind_union(&segments, above_seg, cur_seg);
          segment_sizes[above_seg] += segment_sizes[cur_seg];
          sl_math_linalg_vadd_inplace(bytes_per_px,
                                      segment_sums + bytes_per_px * above_seg,
                                      segment_sums + bytes_per_px * cur_seg);
          ++num_merges;
        }
      }
    }
  }

  sl_png_image_copy(dst, src);
  for (size_t row = 0; row < height; ++row) {
    for (size_t col = 0; col < width; ++col) {
      const size_t idx = row * width + col;
      const size_t seg = sl_unionfind_find_root(&segments, idx);
      const size_t src_row = seg / width;
      const size_t src_col = seg % width;
      sl_png_image_set_pixel(dst,
                             row,
                             col,
                             sl_png_image_get_pixel(src, src_row, src_col));
    }
  }

  sl_unionfind_destroy(&segments);
  sl_free(segment_sizes);
  sl_free(segment_sums);
}

#endif  // SL_IMG_H_INCLUDED
