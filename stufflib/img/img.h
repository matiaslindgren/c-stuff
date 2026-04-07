#ifndef SL_IMG_H_INCLUDED
#define SL_IMG_H_INCLUDED

#include <stddef.h>
#include <stufflib/context/context.h>
#include <stufflib/png/png.h>

void sl_img_segment_rgb(
    struct sl_context ctx[static 1],
    struct sl_png_image dst[const static 1],
    struct sl_png_image src[const static 1],
    size_t threshold_percent
);

#endif  // SL_IMG_H_INCLUDED
