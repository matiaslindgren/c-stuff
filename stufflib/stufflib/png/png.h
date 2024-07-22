#ifndef SL_PNG_H_INCLUDED
#define SL_PNG_H_INCLUDED
// Reference:
// 1. "PNG (Portable Network Graphics) Specification, Version 1.2",
//    http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html,
//    accessed 2023-01-18
#include <stdint.h>
#include <stdio.h>

enum sl_png_chunk_type {
  sl_png_null_chunk = 0,
  sl_png_IHDR,
  sl_png_PLTE,
  sl_png_IDAT,
  sl_png_IEND,
  sl_png_bKGD,
  sl_png_cHRM,
  sl_png_dSIG,
  sl_png_eXIf,
  sl_png_gAMA,
  sl_png_hIST,
  sl_png_iCCP,
  sl_png_iTXt,
  sl_png_pHYs,
  sl_png_sBIT,
  sl_png_sPLT,
  sl_png_sRGB,
  sl_png_sTER,
  sl_png_tEXt,
  sl_png_tIME,
  sl_png_tRNS,
  sl_png_zTXt,
  sl_png_num_chunk_types,
};

static const char* sl_png_chunk_types[] = {
    [sl_png_IHDR] = "IHDR", [sl_png_PLTE] = "PLTE", [sl_png_IDAT] = "IDAT",
    [sl_png_IEND] = "IEND", [sl_png_bKGD] = "bKGD", [sl_png_cHRM] = "cHRM",
    [sl_png_dSIG] = "dSIG", [sl_png_eXIf] = "eXIf", [sl_png_gAMA] = "gAMA",
    [sl_png_hIST] = "hIST", [sl_png_iCCP] = "iCCP", [sl_png_iTXt] = "iTXt",
    [sl_png_pHYs] = "pHYs", [sl_png_sBIT] = "sBIT", [sl_png_sPLT] = "sPLT",
    [sl_png_sRGB] = "sRGB", [sl_png_sTER] = "sTER", [sl_png_tEXt] = "tEXt",
    [sl_png_tIME] = "tIME", [sl_png_tRNS] = "tRNS", [sl_png_zTXt] = "zTXt",
};

enum sl_png_color_type {
  sl_png_grayscale = 0,
  sl_png_rgb = 2,
  sl_png_indexed = 3,
  sl_png_grayscale_alpha = 4,
  sl_png_rgba = 6,
  sl_png_num_color_types,
};

static const char* sl_png_color_types[] = {
    [sl_png_grayscale] = "grayscale",
    [sl_png_rgb] = "rgb",
    [sl_png_indexed] = "indexed",
    [sl_png_grayscale_alpha] = "grayscale with alpha",
    [sl_png_rgba] = "rgba",
};

static size_t sl_png_bytes_per_pixel[] = {
    // TODO
    [sl_png_grayscale] = 1,
    [sl_png_rgb] = 3,
    [sl_png_indexed] = 0,
    [sl_png_grayscale_alpha] = 2,
    [sl_png_rgba] = 4,
};

enum sl_png_filter_type {
  sl_png_filter_none = 0,
  sl_png_filter_sub,
  sl_png_filter_up,
  sl_png_filter_avg,
  sl_png_filter_paeth,
  sl_png_num_filter_types,
};

static const char* sl_png_filter_types[] = {
    [sl_png_filter_none] = "None",
    [sl_png_filter_sub] = "Sub",
    [sl_png_filter_up] = "Up",
    [sl_png_filter_avg] = "Average",
    [sl_png_filter_paeth] = "Paeth",
};

struct sl_png_header {
  // TODO abstract away +2 padding-headache
  size_t width;
  size_t height;
  unsigned bit_depth;
  enum sl_png_color_type color_type;
  unsigned compression;
  unsigned filter;
  unsigned interlace;
};

struct sl_png_chunk {
  enum sl_png_chunk_type type;
  struct sl_span data;
  uint32_t crc32;
};

struct sl_png_chunks {
  size_t count;
  struct sl_png_chunk* chunks;
};

struct sl_png_image {
  struct sl_png_header header;
  struct sl_span data;
  struct sl_span filter;
};

void sl_png_chunk_destroy(struct sl_png_chunk chunk);

void sl_png_chunks_destroy(struct sl_png_chunks chunks);

void sl_png_header_destroy(struct sl_png_header /* header */);

void sl_png_image_destroy(struct sl_png_image image);

void sl_png_image_copy(struct sl_png_image dst[static 1],
                       struct sl_png_image src[static 1]);

struct sl_png_image sl_png_image_rgb_create(const size_t width,
                                            const size_t height);

unsigned char* sl_png_image_get_pixel(struct sl_png_image image[static 1],
                                      size_t row,
                                      size_t col);

void sl_png_image_set_pixel(struct sl_png_image image[static 1],
                            size_t row,
                            size_t col,
                            const unsigned char* new_value);

bool sl_png_is_supported(struct sl_png_header header);

void sl_png_dump_header(FILE stream[const static 1],
                        struct sl_png_header header);

void sl_png_dump_img_data_info(FILE stream[const static 1],
                               struct sl_png_image image);

void sl_png_dump_img_meta(FILE stream[const static 1],
                          struct sl_png_image image);

void sl_png_dump_chunk_type_freq(FILE stream[const static 1],
                                 struct sl_png_chunks chunks);

enum sl_png_chunk_type sl_png_find_chunk_type(
    const char type_id[const static 1]);

struct sl_png_chunk sl_png_read_next_chunk(FILE fp[const static 1]);

struct sl_png_header sl_png_parse_header(struct sl_png_chunk chunk);

uint32_t sl_png_chunk_compute_crc32(struct sl_png_chunk chunk[const static 1]);

bool sl_png_has_signature(const unsigned char buf[const static 8]);

struct sl_png_chunks sl_png_read_n_chunks(const char filename[const static 1],
                                          size_t count);

struct sl_png_chunks sl_png_read_chunks(const char filename[const static 1]);

struct sl_png_header sl_png_read_header(const char filename[const static 1]);

size_t sl_png_data_size(struct sl_png_header header);

size_t sl_png_idat_max_size(struct sl_png_header header);

struct sl_span sl_png_pack_image_data(struct sl_png_image image[static 1]);

void sl_png_unpack_and_pad_image_data(struct sl_png_image image[static 1]);

enum sl_png_filter_type sl_png_parse_filter_type(unsigned filter);

bool sl_png_unapply_filter(struct sl_png_image image[static 1]);

struct sl_png_image sl_png_read_image(const char filename[const static 1]);

bool sl_png_chunk_fwrite_header(FILE stream[const static 1],
                                struct sl_png_header header);

bool sl_png_chunk_fwrite(FILE stream[const static 1],
                         const char chunk_type[const static 1],
                         struct sl_span data[const static 1]);

bool sl_png_write_image(struct sl_png_image image,
                        const char filename[const static 1]);

#endif  // SL_PNG_H_INCLUDED
