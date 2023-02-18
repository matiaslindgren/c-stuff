#ifndef _STUFFLIB_PNG_H_INCLUDED
#define _STUFFLIB_PNG_H_INCLUDED
// Reference:
// 1. "PNG (Portable Network Graphics) Specification, Version 1.2",
//    http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html,
//    accessed 2023-01-18
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_deflate.h"
#include "stufflib_hash.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"

enum stufflib_png_chunk_type {
  stufflib_png_null_chunk = 0,
  stufflib_png_IHDR,
  stufflib_png_PLTE,
  stufflib_png_IDAT,
  stufflib_png_IEND,
  stufflib_png_bKGD,
  stufflib_png_cHRM,
  stufflib_png_dSIG,
  stufflib_png_eXIf,
  stufflib_png_gAMA,
  stufflib_png_hIST,
  stufflib_png_iCCP,
  stufflib_png_iTXt,
  stufflib_png_pHYs,
  stufflib_png_sBIT,
  stufflib_png_sPLT,
  stufflib_png_sRGB,
  stufflib_png_sTER,
  stufflib_png_tEXt,
  stufflib_png_tIME,
  stufflib_png_tRNS,
  stufflib_png_zTXt,
  stufflib_png_num_chunk_types,
};

const char* stufflib_png_chunk_types[] = {
    [stufflib_png_IHDR] = "IHDR", [stufflib_png_PLTE] = "PLTE",
    [stufflib_png_IDAT] = "IDAT", [stufflib_png_IEND] = "IEND",
    [stufflib_png_bKGD] = "bKGD", [stufflib_png_cHRM] = "cHRM",
    [stufflib_png_dSIG] = "dSIG", [stufflib_png_eXIf] = "eXIf",
    [stufflib_png_gAMA] = "gAMA", [stufflib_png_hIST] = "hIST",
    [stufflib_png_iCCP] = "iCCP", [stufflib_png_iTXt] = "iTXt",
    [stufflib_png_pHYs] = "pHYs", [stufflib_png_sBIT] = "sBIT",
    [stufflib_png_sPLT] = "sPLT", [stufflib_png_sRGB] = "sRGB",
    [stufflib_png_sTER] = "sTER", [stufflib_png_tEXt] = "tEXt",
    [stufflib_png_tIME] = "tIME", [stufflib_png_tRNS] = "tRNS",
    [stufflib_png_zTXt] = "zTXt",
};

enum stufflib_png_color_type {
  stufflib_png_grayscale = 0,
  stufflib_png_rgb = 2,
  stufflib_png_indexed = 3,
  stufflib_png_grayscale_alpha = 4,
  stufflib_png_rgba = 6,
  stufflib_png_num_color_types,
};

const char* stufflib_png_color_types[] = {
    [stufflib_png_grayscale] = "grayscale",
    [stufflib_png_rgb] = "rgb",
    [stufflib_png_indexed] = "indexed",
    [stufflib_png_grayscale_alpha] = "grayscale with alpha",
    [stufflib_png_rgba] = "rgba",
};

size_t stufflib_png_bytes_per_pixel[] = {
    // TODO
    [stufflib_png_grayscale] = 1,
    [stufflib_png_rgb] = 3,
    [stufflib_png_indexed] = 0,
    [stufflib_png_grayscale_alpha] = 2,
    [stufflib_png_rgba] = 4,
};

enum stufflib_png_filter_type {
  stufflib_png_filter_none = 0,
  stufflib_png_filter_sub,
  stufflib_png_filter_up,
  stufflib_png_filter_avg,
  stufflib_png_filter_paeth,
  stufflib_png_num_filter_types,
};

const char* stufflib_png_filter_types[] = {
    [stufflib_png_filter_none] = "None",
    [stufflib_png_filter_sub] = "Sub",
    [stufflib_png_filter_up] = "Up",
    [stufflib_png_filter_avg] = "Average",
    [stufflib_png_filter_paeth] = "Paeth",
};

typedef struct stufflib_png_header stufflib_png_header;
struct stufflib_png_header {
  // TODO abstract away +2 padding-headache
  size_t width;
  size_t height;
  unsigned bit_depth;
  enum stufflib_png_color_type color_type;
  unsigned compression;
  unsigned filter;
  unsigned interlace;
};

typedef struct stufflib_png_chunk stufflib_png_chunk;
struct stufflib_png_chunk {
  enum stufflib_png_chunk_type type;
  stufflib_data data;
  uint32_t crc32;
};

typedef struct stufflib_png_chunks stufflib_png_chunks;
struct stufflib_png_chunks {
  size_t count;
  stufflib_png_chunk* chunks;
};

typedef struct stufflib_png_image stufflib_png_image;
struct stufflib_png_image {
  stufflib_png_header header;
  stufflib_data data;
  stufflib_data filter;
};

void stufflib_png_chunk_destroy(stufflib_png_chunk chunk) {
  stufflib_misc_data_destroy(&chunk.data);
}

void stufflib_png_chunks_destroy(stufflib_png_chunks chunks) {
  for (size_t i = 0; i < chunks.count; ++i) {
    stufflib_png_chunk_destroy(chunks.chunks[i]);
  }
  free(chunks.chunks);
}

void stufflib_png_header_destroy(stufflib_png_header header) {
  // no-op
}

void stufflib_png_image_destroy(stufflib_png_image image) {
  stufflib_misc_data_destroy(&image.data);
  stufflib_misc_data_destroy(&image.filter);
}

bool stufflib_png_image_copy(stufflib_png_image dst[restrict static 1],
                             const stufflib_png_image src[restrict static 1]) {
  dst->header = src->header;
  if (!stufflib_misc_data_copy(&dst->data, &src->data)) {
    return false;
  }
  if (!stufflib_misc_data_copy(&dst->filter, &src->filter)) {
    return false;
  }
  return true;
}

unsigned char* stufflib_png_image_get_pixel(
    const stufflib_png_image image[static 1],
    const size_t row,
    const size_t col) {
  const size_t width = image->header.width;
  const size_t bytes_per_px =
      stufflib_png_bytes_per_pixel[image->header.color_type];
  const size_t data_pos = (row * (width + 2) + col) * bytes_per_px;
  return image->data.data + data_pos;
}

void stufflib_png_image_set_pixel(stufflib_png_image image[static 1],
                                  const size_t row,
                                  const size_t col,
                                  const unsigned char* new_value) {
  memcpy(stufflib_png_image_get_pixel(image, row, col),
         new_value,
         stufflib_png_bytes_per_pixel[image->header.color_type]);
}

bool stufflib_png_is_supported(const stufflib_png_header header) {
  return (header.compression == 0 && header.filter == 0 &&
          header.interlace == 0 &&
          (header.color_type == stufflib_png_rgb ||
           header.color_type == stufflib_png_rgba) &&
          header.bit_depth == 8);
}

void stufflib_png_dump_header(FILE stream[const static 1],
                              const stufflib_png_header header) {
  fprintf(stream, "{");
  fprintf(stream, "\"width\":%zu,", header.width);
  fprintf(stream, "\"height\":%zu,", header.height);
  fprintf(stream, "\"bit depth\":%u,", header.bit_depth);
  fprintf(stream,
          "\"color type\":\"%s\",",
          stufflib_png_color_types[header.color_type]);
  fprintf(stream, "\"compression\":%u,", header.compression);
  fprintf(stream, "\"filter\":%u,", header.filter);
  fprintf(stream, "\"interlace\":%u", header.interlace);
  fprintf(stream, "}");
}

void stufflib_png_dump_img_data_info(FILE stream[const static 1],
                                     const stufflib_png_image image) {
  fprintf(stream, "{");
  fprintf(stream, "\"length\":%zu,", image.data.size);
  fprintf(stream, "\"filters\":{");
  size_t freq[stufflib_png_num_filter_types] = {0};
  for (size_t i = 0; i < image.filter.size; ++i) {
    ++freq[image.filter.data[i]];
  }
  bool did_print = false;
  for (size_t filter = 0; filter < stufflib_png_num_filter_types; ++filter) {
    if (freq[filter]) {
      const char* filter_name = stufflib_png_filter_types[filter];
      if (did_print) {
        fprintf(stream, ",");
      }
      fprintf(stream, "\"%s\":%zu", filter_name, freq[filter]);
      did_print = true;
    }
  }
  fprintf(stream, "}}");
}

void stufflib_png_dump_img_meta(FILE stream[const static 1],
                                const stufflib_png_image image) {
  stufflib_png_dump_img_data_info(stream, image);
  stufflib_png_dump_header(stream, image.header);
}

void stufflib_png_dump_chunk_type_freq(FILE stream[const static 1],
                                       const stufflib_png_chunks chunks) {
  fprintf(stream, "{");
  size_t freq[stufflib_png_num_chunk_types] = {0};
  for (size_t i = 0; i < chunks.count; ++i) {
    ++freq[chunks.chunks[i].type];
  }
  bool did_print = false;
  for (size_t type = 0; type < stufflib_png_num_chunk_types; ++type) {
    if (freq[type]) {
      const char* type_name = stufflib_png_chunk_types[type];
      if (did_print) {
        fprintf(stream, ",");
      }
      fprintf(stream, "\"%s\":%zu", type_name, freq[type]);
      did_print = true;
    }
  }
  fprintf(stream, "}");
}

enum stufflib_png_chunk_type stufflib_png_find_chunk_type(
    const char type_id[const static 1]) {
  for (unsigned type = 1; type < stufflib_png_num_chunk_types; ++type) {
    if (strncmp(type_id, stufflib_png_chunk_types[type], 4) == 0) {
      return type;
    }
  }
  return stufflib_png_null_chunk;
}

stufflib_png_chunk stufflib_png_read_next_chunk(FILE fp[const static 1]) {
  stufflib_png_chunk chunk = {0};

  {
    const size_t length_len = 4;
    unsigned char length_buf[length_len];
    if (fread(length_buf, 1, length_len, fp) != length_len) {
      STUFFLIB_PRINT_ERROR("failed reading PNG chunk length");
      goto error;
    }
    const size_t chunk_size = stufflib_misc_parse_big_endian(4, length_buf);
    if (chunk_size > ((size_t)1 << 31)) {
      STUFFLIB_PRINT_ERROR("PNG chunk length too large (%zu)", chunk_size);
      goto error;
    }
    chunk.data.size = chunk_size;
  }

  {
    const size_t type_len = 4;
    char chunk_type[type_len + 1];
    if (fread(chunk_type, 1, type_len, fp) != type_len) {
      STUFFLIB_PRINT_ERROR("failed reading PNG chunk type");
      goto error;
    }
    chunk.type = stufflib_png_find_chunk_type(chunk_type);
  }

  if (chunk.data.size) {
    chunk.data.data = calloc(chunk.data.size, 1);
    if (!chunk.data.data) {
      goto error;
    }
    if (fread(chunk.data.data, 1, chunk.data.size, fp) != chunk.data.size) {
      STUFFLIB_PRINT_ERROR("failed reading PNG chunk data");
      goto error;
    }
  }

  {
    const size_t crc32_len = 4;
    unsigned char crc32_buf[crc32_len];
    if (fread(crc32_buf, 1, crc32_len, fp) != crc32_len) {
      STUFFLIB_PRINT_ERROR("failed reading PNG chunk crc32");
      goto error;
    }
    chunk.crc32 = stufflib_misc_parse_big_endian(4, crc32_buf);
  }

  return chunk;

error:
  stufflib_png_chunk_destroy(chunk);
  return (stufflib_png_chunk){0};
}

stufflib_png_header stufflib_png_parse_header(const stufflib_png_chunk chunk) {
  if (chunk.type != stufflib_png_IHDR) {
    const char* type_str = stufflib_png_chunk_types[chunk.type];
    STUFFLIB_PRINT_ERROR("cannot parse %s chunk as IHDR", type_str);
    return (stufflib_png_header){0};
  }
  unsigned char* const data = chunk.data.data;
  return (stufflib_png_header){
      .width = stufflib_misc_parse_big_endian(4, data),
      .height = stufflib_misc_parse_big_endian(4, data + 4),
      .bit_depth = (unsigned)(data[8]),
      .color_type = (enum stufflib_png_color_type)(data[9]),
      .compression = (unsigned)(data[10]),
      .filter = (unsigned)(data[11]),
      .interlace = (unsigned)(data[12]),
  };
}

uint32_t stufflib_png_chunk_compute_crc32(
    const stufflib_png_chunk chunk[const static 1]) {
  const char* type_str = stufflib_png_chunk_types[chunk->type];
  const uint32_t crc32_type = stufflib_hash_crc32_str(type_str);
  const uint32_t crc32_chunk = stufflib_hash_crc32(crc32_type ^ 0xffffffff,
                                                   chunk->data.size,
                                                   chunk->data.data);
  return crc32_chunk ^ 0xffffffff;
}

bool stufflib_png_has_signature(const unsigned char buf[const static 8]) {
  return buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4e && buf[3] == 0x47 &&
         buf[4] == 0x0d && buf[5] == 0x0a && buf[6] == 0x1a && buf[7] == 0x0a;
}

stufflib_png_chunks stufflib_png_read_n_chunks(
    const char filename[const static 1],
    const size_t count) {
  bool ok = false;

  FILE* fp = nullptr;
  stufflib_png_chunk* chunks = nullptr;

  fp = fopen(filename, "r");
  if (!fp) {
    STUFFLIB_PRINT_ERROR("cannot open %s", filename);
    goto done;
  }

  // check for 8 byte file header containing 'PNG' in ascii
  {
    const size_t header_len = 8;
    unsigned char buf[header_len];
    if (fread(buf, 1, header_len, fp) != header_len) {
      STUFFLIB_PRINT_ERROR("failed reading PNG header");
      goto done;
    }
    if (!stufflib_png_has_signature(buf)) {
      STUFFLIB_PRINT_ERROR("not a PNG image");
      goto done;
    }
  }

  size_t read_count = 0;
  stufflib_png_chunk chunk = {0};

  while (read_count < count &&
         (!read_count || chunk.type != stufflib_png_IEND)) {
    chunk = stufflib_png_read_next_chunk(fp);
    if (chunk.type == stufflib_png_null_chunk) {
      STUFFLIB_PRINT_ERROR("unknown chunk");
      goto done;
    }
    if (chunk.crc32 != stufflib_png_chunk_compute_crc32(&chunk)) {
      STUFFLIB_PRINT_ERROR("mismatching crc32");
      goto done;
    }
    stufflib_png_chunk* tmp =
        realloc(chunks, (read_count + 1) * sizeof(stufflib_png_chunk));
    if (!tmp) {
      STUFFLIB_PRINT_ERROR(
          "failed resizing chunks array during read PNG chunks");
      goto done;
    }
    chunks = tmp;
    chunks[read_count++] = chunk;
  }

  ok = true;

done:
  if (fp) {
    fclose(fp);
  }
  if (!ok) {
    free(chunks);
    return (stufflib_png_chunks){0};
  }
  return (stufflib_png_chunks){.count = read_count, .chunks = chunks};
}

stufflib_png_chunks stufflib_png_read_chunks(
    const char filename[const static 1]) {
  return stufflib_png_read_n_chunks(filename, SIZE_MAX);
}

stufflib_png_header stufflib_png_read_header(
    const char filename[const static 1]) {
  stufflib_png_chunks chunks = stufflib_png_read_n_chunks(filename, 1);
  if (chunks.count != 1) {
    STUFFLIB_PRINT_ERROR("failed reading IHDR chunk from %s", filename);
    return (stufflib_png_header){0};
  }
  stufflib_png_header header = stufflib_png_parse_header(chunks.chunks[0]);
  stufflib_png_chunks_destroy(chunks);
  return header;
}

size_t stufflib_png_data_size(const stufflib_png_header header) {
  const size_t bytes_per_px = stufflib_png_bytes_per_pixel[header.color_type];
  return header.height + bytes_per_px * header.width * header.height;
}

size_t stufflib_png_idat_max_size(const stufflib_png_header header) {
  const size_t header_size = 6;
  const size_t checksum_size = 4;
  const size_t data_size = stufflib_png_data_size(header);
  return header_size + 2 * data_size + checksum_size;
}

stufflib_data stufflib_png_pack_image_data(
    const stufflib_png_image image[static 1]) {
  stufflib_data packed = {0};

  const size_t width = image->header.width;
  const size_t height = image->header.height;
  const size_t bytes_per_px =
      stufflib_png_bytes_per_pixel[image->header.color_type];

  packed = stufflib_misc_data_new(stufflib_png_data_size(image->header));
  if (!packed.size) {
    STUFFLIB_PRINT_ERROR("failed allocating packed image data buffer");
    goto error;
  }

  for (size_t row = 0; row < height; ++row) {
    const size_t filter_idx = bytes_per_px * row * width + row;
    packed.data[filter_idx] = stufflib_png_filter_none;
    for (size_t col = 0; col < width; ++col) {
      const size_t dst_idx = bytes_per_px * (width * row + col) + 1 + row;
      const size_t src_idx =
          bytes_per_px * ((width + 2) * (row + 1) + (col + 1));
      memcpy(packed.data + dst_idx, image->data.data + src_idx, bytes_per_px);
    }
  }

  return packed;

error:
  stufflib_misc_data_destroy(&packed);
  return (stufflib_data){0};
}

bool stufflib_png_unpack_and_pad_image_data(
    stufflib_png_image image[static 1]) {
  stufflib_data filter = {0};
  stufflib_data padded = {0};

  const size_t width = image->header.width;
  const size_t height = image->header.height;
  const size_t bytes_per_px =
      stufflib_png_bytes_per_pixel[image->header.color_type];

  filter = stufflib_misc_data_new(height);
  if (!filter.size) {
    STUFFLIB_PRINT_ERROR("failed allocating scanline filter buffer");
    goto error;
  }
  padded = stufflib_misc_data_new(bytes_per_px * (width + 2) * (height + 2));
  if (!padded.size) {
    STUFFLIB_PRINT_ERROR("failed allocating buffer for padded image data");
    goto error;
  }

  for (size_t row = 0; row < height; ++row) {
    const size_t idx_col0_raw = row * bytes_per_px * width + row;
    filter.data[row] = image->data.data[idx_col0_raw];
    for (size_t col = 0; col < width; ++col) {
      for (size_t byte = 0; byte < bytes_per_px; ++byte) {
        const size_t idx_raw = idx_col0_raw + 1 + col * bytes_per_px + byte;
        const size_t idx_pad = (row + 1) * (width + 2) * bytes_per_px +
                               (col + 1) * bytes_per_px + byte;
        padded.data[idx_pad] = image->data.data[idx_raw];
      }
    }
  }

  image->filter = filter;
  stufflib_misc_data_destroy(&image->data);
  image->data = padded;
  return true;

error:
  stufflib_misc_data_destroy(&filter);
  stufflib_misc_data_destroy(&padded);
  return false;
}

bool stufflib_png_unapply_filter(stufflib_png_image image[static 1]) {
  const size_t width = image->header.width;
  const size_t height = image->header.height;
  const size_t bytes_per_px =
      stufflib_png_bytes_per_pixel[image->header.color_type];

  for (size_t row = 1; row < height + 1; ++row) {
    const enum stufflib_png_filter_type filter = image->filter.data[row - 1];
    if (filter == stufflib_png_filter_none) {
      continue;
    }
    for (size_t col = 1; col < width + 1; ++col) {
      unsigned char* dst_px = stufflib_png_image_get_pixel(image, row, col);
      const unsigned char* left_px =
          stufflib_png_image_get_pixel(image, row, col - 1);
      const unsigned char* above_px =
          stufflib_png_image_get_pixel(image, row - 1, col);
      const unsigned char* left_above_px =
          stufflib_png_image_get_pixel(image, row - 1, col - 1);
      for (size_t byte = 0; byte < bytes_per_px; ++byte) {
        switch (filter) {
          case stufflib_png_filter_sub: {
            dst_px[byte] += left_px[byte];
          } break;
          case stufflib_png_filter_up: {
            dst_px[byte] += above_px[byte];
          } break;
          case stufflib_png_filter_avg: {
            dst_px[byte] += (left_px[byte] + above_px[byte]) / 2;
          } break;
          case stufflib_png_filter_paeth: {
            const int a = left_px[byte];
            const int b = above_px[byte];
            const int c = left_above_px[byte];
            const int p = a + b - c;
            const int pa = abs(p - a);
            const int pb = abs(p - b);
            const int pc = abs(p - c);
            if (pa <= pb && pa <= pc) {
              dst_px[byte] += a;
            } else if (pb <= pc) {
              dst_px[byte] += b;
            } else {
              dst_px[byte] += c;
            }
          } break;
          default: {
            STUFFLIB_PRINT_ERROR("filter not implemented %s",
                                 stufflib_png_filter_types[filter]);
            return false;
          } break;
        }
      }
    }
  }
  return true;
}

stufflib_png_image stufflib_png_read_image(
    const char filename[const static 1]) {
  stufflib_png_image image = {0};
  stufflib_data idat = {0};

  stufflib_png_chunks chunks = stufflib_png_read_chunks(filename);
  if (!chunks.count) {
    goto error;
  }

  image.header = stufflib_png_parse_header(chunks.chunks[0]);
  if (!stufflib_png_is_supported(image.header)) {
    STUFFLIB_PRINT_ERROR(("unsupported PNG features in %s\n"
                          "  image must be:\n"
                          "    8-bit/color\n"
                          "    RGB/A\n"
                          "    non-interlaced\n"
                          "    compression=0\n"
                          "    filter=0\n"
                          "  instead, header is:\n"),
                         filename);
    stufflib_png_dump_header(stderr, image.header);
    goto error;
  }

  for (size_t i = 1; i < chunks.count; ++i) {
    const stufflib_png_chunk chunk = chunks.chunks[i];
    if (chunk.type == stufflib_png_IDAT) {
      if (!stufflib_misc_concat(&idat, &chunk.data)) {
        STUFFLIB_PRINT_ERROR("failed concatenating IDAT block");
        goto error;
      }
    }
  }
  // TODO parse PLTE if header.color_type == stufflib_png_indexed

  image.data = stufflib_misc_data_new(stufflib_png_data_size(image.header));
  if (!image.data.size) {
    STUFFLIB_PRINT_ERROR("failed allocating output buffer");
    goto error;
  }
  const size_t num_decoded = stufflib_inflate(image.data, idat);
  if (num_decoded != image.data.size) {
    STUFFLIB_PRINT_ERROR("failed decoding IDAT stream");
    goto error;
  }
  if (!stufflib_png_unpack_and_pad_image_data(&image)) {
    STUFFLIB_PRINT_ERROR("failed padding decoded image");
    goto error;
  }
  if (!stufflib_png_unapply_filter(&image)) {
    STUFFLIB_PRINT_ERROR("failed unfiltering decoded image");
    goto error;
  }

  stufflib_png_chunks_destroy(chunks);
  stufflib_misc_data_destroy(&idat);
  return image;

error:
  stufflib_png_chunks_destroy(chunks);
  stufflib_misc_data_destroy(&idat);
  stufflib_png_image_destroy(image);
  return (stufflib_png_image){0};
}

bool stufflib_png_chunk_fwrite_header(FILE stream[const static 1],
                                      const stufflib_png_header header) {
  const unsigned char png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
  if (fwrite(png_signature, 1, 8, stream) != 8) {
    return false;
  }
  const unsigned char chunk_len[] = {0, 0, 0, 0xd};
  if (fwrite(chunk_len, 1, 4, stream) != 4) {
    return false;
  }
  unsigned char ihdr_data[4 + 13] = {0};
  memcpy(ihdr_data, "IHDR", 4);
  const unsigned char* width =
      stufflib_misc_encode_big_endian(4, (unsigned char[4]){0}, header.width);
  memcpy(ihdr_data + 4, width, 4);
  const unsigned char* height =
      stufflib_misc_encode_big_endian(4, (unsigned char[4]){0}, header.height);
  memcpy(ihdr_data + 8, height, 4);
  memcpy(ihdr_data + 12, &header.bit_depth, 1);
  memcpy(ihdr_data + 13, &header.color_type, 1);
  memcpy(ihdr_data + 14, &header.compression, 1);
  memcpy(ihdr_data + 15, &header.filter, 1);
  memcpy(ihdr_data + 16, &header.interlace, 1);
  if (fwrite(ihdr_data, 1, 17, stream) != 17) {
    return false;
  }
  const unsigned char* crc32 = stufflib_misc_encode_big_endian(
      4,
      (unsigned char[4]){0},
      stufflib_hash_crc32_bytes(STUFFLIB_ARRAY_LEN(ihdr_data), ihdr_data));
  if (fwrite(crc32, 1, 4, stream) != 4) {
    return false;
  }
  return true;
}

bool stufflib_png_chunk_fwrite(FILE stream[const static 1],
                               const char chunk_type[const static 1],
                               const stufflib_data data[const static 1]) {
  if (data->size > ((size_t)1 << 31)) {
    STUFFLIB_PRINT_ERROR("will not write too large %s chunk of size %zu",
                         chunk_type,
                         data->size);
    return false;
  }
  stufflib_data crc_data = stufflib_misc_data_new(data->size + 4);
  if (!crc_data.size) {
    STUFFLIB_PRINT_ERROR("failed allocating CRC32 buffer for %s chunk",
                         chunk_type);
    return false;
  }

  bool ok = false;

  const unsigned char* chunk_len =
      stufflib_misc_encode_big_endian(4, (unsigned char[4]){0}, data->size);
  if (fwrite(chunk_len, 1, 4, stream) != 4) {
    STUFFLIB_PRINT_ERROR("failed writing %s chunk length", chunk_type);
    goto done;
  }
  memcpy(crc_data.data, chunk_type, 4);
  if (data->size) {
    memcpy(crc_data.data + 4, data->data, data->size);
  }
  if (fwrite(crc_data.data, 1, crc_data.size, stream) != crc_data.size) {
    STUFFLIB_PRINT_ERROR("failed writing %s chunk type + data", chunk_type);
    goto done;
  }
  const unsigned char* crc32 = stufflib_misc_encode_big_endian(
      4,
      (unsigned char[4]){0},
      stufflib_hash_crc32_bytes(crc_data.size, crc_data.data));
  if (fwrite(crc32, 1, 4, stream) != 4) {
    STUFFLIB_PRINT_ERROR("failed writing %s chunk CRC32", chunk_type);
    goto done;
  }

  ok = true;

done:
  stufflib_misc_data_destroy(&crc_data);
  return ok;
}

bool stufflib_png_write_image(const stufflib_png_image image,
                              const char filename[const static 1]) {
  bool write_ok = false;
  stufflib_data packed_data = {0};
  stufflib_data idat = {0};
  FILE* fp = nullptr;

  fp = fopen(filename, "w");
  if (!fp) {
    STUFFLIB_PRINT_ERROR("cannot open %s", filename);
    goto done;
  }
  packed_data = stufflib_png_pack_image_data(&image);
  if (!packed_data.size) {
    STUFFLIB_PRINT_ERROR("failed allocating buffer image data");
    goto done;
  }
  idat = stufflib_misc_data_new(stufflib_png_idat_max_size(image.header));
  if (!idat.size) {
    STUFFLIB_PRINT_ERROR("failed allocating buffer for IDAT chunks");
    goto done;
  }

  idat.size = stufflib_deflate_uncompressed(idat, packed_data);
  idat.data = realloc(idat.data, idat.size);

  if (!stufflib_png_chunk_fwrite_header(fp, image.header)) {
    STUFFLIB_PRINT_ERROR("failed writing PNG header to %s", filename);
    goto done;
  }
  if (!stufflib_png_chunk_fwrite(fp, "IDAT", &idat)) {
    STUFFLIB_PRINT_ERROR("failed writing PNG IDAT chunks to %s", filename);
    goto done;
  }
  if (!stufflib_png_chunk_fwrite(fp, "IEND", &(stufflib_data){0})) {
    STUFFLIB_PRINT_ERROR("failed writing PNG IEND chunk to %s", filename);
    goto done;
  }

  write_ok = true;

done:
  if (fp) {
    fclose(fp);
  }
  stufflib_misc_data_destroy(&idat);
  stufflib_misc_data_destroy(&packed_data);
  return write_ok;
}

#endif  // _STUFFLIB_PNG_H_INCLUDED
