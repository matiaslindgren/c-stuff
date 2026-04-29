#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <stufflib/context/context.h>
#include <stufflib/png/png.h>

#pragma GCC diagnostic ignored "-Wcast-qual"

int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FILE* fp = fmemopen((void*)data, size, "r");
  if (!fp) {
    return 0;
  }
  struct sl_context ctx   = {0};
  struct sl_png_image img = sl_png_read_image_fp(&ctx, fp);
  sl_png_image_destroy(img);
  for (struct sl_error_msg e = {0}; sl_error_pop(&ctx.errors, &e);) {
  }
  fclose(fp);
  return 0;
}
