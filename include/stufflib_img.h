#ifndef _STUFFLIB_IMG_H_INCLUDED
#define _STUFFLIB_IMG_H_INCLUDED
#include <math.h>

#include "stufflib_math.h"
#include "stufflib_png.h"
#include "stufflib_unionfind.h"

stufflib_png_image stufflib_img_segment_rgb(const stufflib_png_image src,
                                            const size_t num_iterations,
                                            const double threshold) {
  // padded
  const size_t width = src.header.width + 2;
  const size_t height = src.header.height + 2;
  const size_t bytes_per_px = 3;
  const double mass_threshold =
      fmin(1, fmax(0, threshold)) *
      stufflib_math_linalg_norm2(bytes_per_px, (const double[]){1, 1, 1});

  stufflib_unionfind segments = {0};
  size_t* segment_size = calloc(width * height, sizeof(size_t));
  double* segment_mass = calloc(width * height, sizeof(double));
  assert(stufflib_unionfind_init(&segments, width * height));
  assert(segment_size);
  assert(segment_mass);
  for (size_t row = 0; row < height; ++row) {
    for (size_t col = 0; col < width; ++col) {
      const unsigned char* pixel = stufflib_png_image_get_pixel(&src, row, col);
      const double unit_pixel[] = (const double[]){
          pixel[0] / 255,
          pixel[1] / 255,
          pixel[2] / 255,
      };
      const double mass = stufflib_math_linalg_norm2(bytes_per_px, unit_pixel);
      const size_t idx = row * width + col;
      segment_size[idx] = 1;
      segment_mass[idx] = mass;
    }
  }

  for (size_t i = 0; i < num_iterations; ++i) {
    for (size_t row = 2; row < height - 1; ++row) {
      for (size_t col = 2; col < width - 1; ++col) {
        const size_t cur_idx = row * width + col;
        const size_t cur_seg = stufflib_unionfind_find_root(&segments, cur_idx);
        const double cur_mass = segment_mass[cur_seg] / segment_size[cur_seg];

        const size_t left_idx = row * width + col - 1;
        const size_t left_seg =
            stufflib_unionfind_find_root(&segments, left_idx);
        const double left_mass =
            segment_mass[left_seg] / segment_size[left_seg];
        const double left_diff = fabs(left_mass - cur_mass);

        const size_t above_idx = (row - 1) * width + col;
        const size_t above_seg =
            stufflib_unionfind_find_root(&segments, above_idx);
        const double above_mass =
            segment_mass[above_seg] / segment_size[above_seg];
        const double above_diff = fabs(above_mass - cur_mass);

        if (left_diff < above_diff && left_diff < mass_threshold) {
          stufflib_unionfind_union(&segments, left_seg, cur_seg);
          segment_size[left_seg] += segment_size[cur_seg];
          segment_mass[left_seg] += segment_mass[cur_seg];
        } else if (above_diff < mass_threshold) {
          stufflib_unionfind_union(&segments, above_seg, cur_seg);
          segment_size[above_seg] += segment_size[cur_seg];
          segment_mass[above_seg] += segment_mass[cur_seg];
        }
      }
    }
  }

  free(segment_size);
  free(segment_mass);

  stufflib_png_image dst = {0};
  stufflib_png_image_copy(&dst, &src);
  for (size_t row = 0; row < height; ++row) {
    for (size_t col = 0; col < width; ++col) {
      const size_t idx = row * width + col;
      const size_t seg = stufflib_unionfind_find_root(&segments, idx);
      memcpy(dst.data.data + idx, src.data.data + seg, bytes_per_px);
    }
  }
  stufflib_unionfind_destroy(&segments);
  return dst;
}

#endif  // _STUFFLIB_IMG_H_INCLUDED
