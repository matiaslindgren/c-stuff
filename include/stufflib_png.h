#ifndef SL_PNG_H_INCLUDED
#define SL_PNG_H_INCLUDED
// Reference:
// 1. "PNG (Portable Network Graphics) Specification, Version 1.2",
//    http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html,
//    accessed 2023-01-18
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_data.h"
#include "stufflib_deflate.h"
#include "stufflib_hash.h"
#include "stufflib_macros.h"
#include "stufflib_memory.h"
#include "stufflib_misc.h"

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

const char* sl_png_chunk_types[] = {
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

const char* sl_png_color_types[] = {
    [sl_png_grayscale] = "grayscale",
    [sl_png_rgb] = "rgb",
    [sl_png_indexed] = "indexed",
    [sl_png_grayscale_alpha] = "grayscale with alpha",
    [sl_png_rgba] = "rgba",
};

size_t sl_png_bytes_per_pixel[] = {
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

const char* sl_png_filter_types[] = {
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
  struct sl_data data;
  uint32_t crc32;
};

struct sl_png_chunks {
  size_t count;
  struct sl_png_chunk* chunks;
};

struct sl_png_image {
  struct sl_png_header header;
  struct sl_data data;
  struct sl_data filter;
};

void sl_png_chunk_destroy(struct sl_png_chunk chunk) {
  sl_data_delete(&chunk.data);
}

void sl_png_chunks_destroy(struct sl_png_chunks chunks) {
  for (size_t i = 0; i < chunks.count; ++i) {
    sl_png_chunk_destroy(chunks.chunks[i]);
  }
  sl_free(chunks.chunks);
}

void sl_png_header_destroy(struct sl_png_header header) {
  // no-op
}

void sl_png_image_destroy(struct sl_png_image image) {
  sl_data_delete(&(image.data));
  sl_data_delete(&(image.filter));
}

void sl_png_image_copy(struct sl_png_image dst[restrict static 1],
                       const struct sl_png_image src[restrict static 1]) {
  dst->header = src->header;
  dst->data = sl_data_copy(&(src->data));
  dst->filter = sl_data_copy(&(src->filter));
}

unsigned char* sl_png_image_get_pixel(const struct sl_png_image image[static 1],
                                      const size_t row,
                                      const size_t col) {
  const size_t width = image->header.width;
  const size_t bytes_per_px = sl_png_bytes_per_pixel[image->header.color_type];
  const size_t data_pos = (row * (width + 2) + col) * bytes_per_px;
  return image->data.data + data_pos;
}

void sl_png_image_set_pixel(struct sl_png_image image[static 1],
                            const size_t row,
                            const size_t col,
                            const unsigned char* new_value) {
  memcpy(sl_png_image_get_pixel(image, row, col),
         new_value,
         sl_png_bytes_per_pixel[image->header.color_type]);
}

bool sl_png_is_supported(const struct sl_png_header header) {
  return (
      header.compression == 0 && header.filter == 0 && header.interlace == 0 &&
      (header.color_type == sl_png_rgb || header.color_type == sl_png_rgba) &&
      header.bit_depth == 8);
}

void sl_png_dump_header(FILE stream[const static 1],
                        const struct sl_png_header header) {
  fprintf(stream, "{");
  fprintf(stream, "\"width\":%zu,", header.width);
  fprintf(stream, "\"height\":%zu,", header.height);
  fprintf(stream, "\"bit depth\":%u,", header.bit_depth);
  fprintf(stream,
          "\"color type\":\"%s\",",
          sl_png_color_types[header.color_type]);
  fprintf(stream, "\"compression\":%u,", header.compression);
  fprintf(stream, "\"filter\":%u,", header.filter);
  fprintf(stream, "\"interlace\":%u", header.interlace);
  fprintf(stream, "}");
}

void sl_png_dump_img_data_info(FILE stream[const static 1],
                               const struct sl_png_image image) {
  fprintf(stream, "{");
  fprintf(stream, "\"length\":%zu,", image.data.size);
  fprintf(stream, "\"filters\":{");
  size_t freq[sl_png_num_filter_types] = {0};
  for (size_t i = 0; i < image.filter.size; ++i) {
    ++freq[image.filter.data[i]];
  }
  bool did_print = false;
  for (size_t filter = 0; filter < sl_png_num_filter_types; ++filter) {
    if (freq[filter]) {
      const char* filter_name = sl_png_filter_types[filter];
      if (did_print) {
        fprintf(stream, ",");
      }
      fprintf(stream, "\"%s\":%zu", filter_name, freq[filter]);
      did_print = true;
    }
  }
  fprintf(stream, "}}");
}

void sl_png_dump_img_meta(FILE stream[const static 1],
                          const struct sl_png_image image) {
  sl_png_dump_img_data_info(stream, image);
  sl_png_dump_header(stream, image.header);
}

void sl_png_dump_chunk_type_freq(FILE stream[const static 1],
                                 const struct sl_png_chunks chunks) {
  fprintf(stream, "{");
  size_t freq[sl_png_num_chunk_types] = {0};
  for (size_t i = 0; i < chunks.count; ++i) {
    ++freq[chunks.chunks[i].type];
  }
  bool did_print = false;
  for (size_t type = 0; type < sl_png_num_chunk_types; ++type) {
    if (freq[type]) {
      const char* type_name = sl_png_chunk_types[type];
      if (did_print) {
        fprintf(stream, ",");
      }
      fprintf(stream, "\"%s\":%zu", type_name, freq[type]);
      did_print = true;
    }
  }
  fprintf(stream, "}");
}

enum sl_png_chunk_type sl_png_find_chunk_type(
    const char type_id[const static 1]) {
  for (unsigned type = 1; type < sl_png_num_chunk_types; ++type) {
    if (strncmp(type_id, sl_png_chunk_types[type], 4) == 0) {
      return type;
    }
  }
  return sl_png_null_chunk;
}

struct sl_png_chunk sl_png_read_next_chunk(FILE fp[const static 1]) {
  struct sl_png_chunk chunk = {0};

  {
    const size_t length_len = 4;
    unsigned char length_buf[length_len];
    if (fread(length_buf, 1, length_len, fp) != length_len) {
      SL_LOG_ERROR("failed reading PNG chunk length");
      goto error;
    }
    const size_t chunk_size = sl_misc_parse_big_endian(4, length_buf);
    if (chunk_size > ((size_t)1 << 31)) {
      SL_LOG_ERROR("PNG chunk length too large (%zu)", chunk_size);
      goto error;
    }
    chunk.data.size = chunk_size;
  }

  {
    const size_t type_len = 4;
    char chunk_type[type_len + 1];
    if (fread(chunk_type, 1, type_len, fp) != type_len) {
      SL_LOG_ERROR("failed reading PNG chunk type");
      goto error;
    }
    chunk.type = sl_png_find_chunk_type(chunk_type);
  }

  if (chunk.data.size) {
    chunk.data = sl_data_create(chunk.data.size);
    if (fread(chunk.data.data, 1, chunk.data.size, fp) != chunk.data.size) {
      SL_LOG_ERROR("failed reading PNG chunk data");
      goto error;
    }
  }

  {
    const size_t crc32_len = 4;
    unsigned char crc32_buf[crc32_len];
    if (fread(crc32_buf, 1, crc32_len, fp) != crc32_len) {
      SL_LOG_ERROR("failed reading PNG chunk crc32");
      goto error;
    }
    chunk.crc32 = sl_misc_parse_big_endian(4, crc32_buf);
  }

  return chunk;

error:
  sl_png_chunk_destroy(chunk);
  return (struct sl_png_chunk){0};
}

struct sl_png_header sl_png_parse_header(const struct sl_png_chunk chunk) {
  if (chunk.type != sl_png_IHDR) {
    const char* type_str = sl_png_chunk_types[chunk.type];
    SL_LOG_ERROR("cannot parse %s chunk as IHDR", type_str);
    return (struct sl_png_header){0};
  }
  unsigned char* const data = chunk.data.data;
  return (struct sl_png_header){
      .width = sl_misc_parse_big_endian(4, data),
      .height = sl_misc_parse_big_endian(4, data + 4),
      .bit_depth = data[8],
      .color_type = data[9],
      .compression = data[10],
      .filter = data[11],
      .interlace = data[12],
  };
}

uint32_t sl_png_chunk_compute_crc32(
    const struct sl_png_chunk chunk[const static 1]) {
  const char* type_str = sl_png_chunk_types[chunk->type];
  const uint32_t crc32_type = sl_hash_crc32_str(type_str);
  const uint32_t crc32_chunk = sl_hash_crc32(crc32_type ^ 0xffffffff,
                                             chunk->data.size,
                                             chunk->data.data);
  return crc32_chunk ^ 0xffffffff;
}

bool sl_png_has_signature(const unsigned char buf[const static 8]) {
  return buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4e && buf[3] == 0x47 &&
         buf[4] == 0x0d && buf[5] == 0x0a && buf[6] == 0x1a && buf[7] == 0x0a;
}

struct sl_png_chunks sl_png_read_n_chunks(const char filename[const static 1],
                                          const size_t count) {
  bool ok = false;

  FILE* fp = nullptr;
  struct sl_png_chunk* chunks = nullptr;

  fp = fopen(filename, "r");
  if (!fp) {
    SL_LOG_ERROR("cannot open %s", filename);
    goto done;
  }

  // check for 8 byte file header containing 'PNG' in ascii
  {
    const size_t header_len = 8;
    unsigned char buf[header_len];
    if (fread(buf, 1, header_len, fp) != header_len) {
      SL_LOG_ERROR("failed reading PNG header");
      goto done;
    }
    if (!sl_png_has_signature(buf)) {
      SL_LOG_ERROR("not a PNG image");
      goto done;
    }
  }

  size_t read_count = 0;
  struct sl_png_chunk chunk = {0};

  while (read_count < count && (!read_count || chunk.type != sl_png_IEND)) {
    chunk = sl_png_read_next_chunk(fp);
    if (chunk.type == sl_png_null_chunk) {
      SL_LOG_ERROR("unknown chunk");
      goto done;
    }
    if (chunk.crc32 != sl_png_chunk_compute_crc32(&chunk)) {
      SL_LOG_ERROR("mismatching crc32");
      goto done;
    }
    chunks = sl_realloc(chunks,
                        read_count,
                        read_count + 1,
                        sizeof(struct sl_png_chunk));
    chunks[read_count++] = chunk;
  }

  ok = true;

done:
  if (fp) {
    fclose(fp);
  }
  if (!ok) {
    sl_free(chunks);
    return (struct sl_png_chunks){0};
  }
  return (struct sl_png_chunks){.count = read_count, .chunks = chunks};
}

struct sl_png_chunks sl_png_read_chunks(const char filename[const static 1]) {
  return sl_png_read_n_chunks(filename, SIZE_MAX);
}

struct sl_png_header sl_png_read_header(const char filename[const static 1]) {
  struct sl_png_chunks chunks = sl_png_read_n_chunks(filename, 1);
  if (chunks.count != 1) {
    SL_LOG_ERROR("failed reading IHDR chunk from %s", filename);
    return (struct sl_png_header){0};
  }
  struct sl_png_header header = sl_png_parse_header(chunks.chunks[0]);
  sl_png_chunks_destroy(chunks);
  return header;
}

size_t sl_png_data_size(const struct sl_png_header header) {
  const size_t bytes_per_px = sl_png_bytes_per_pixel[header.color_type];
  return header.height + bytes_per_px * header.width * header.height;
}

size_t sl_png_idat_max_size(const struct sl_png_header header) {
  const size_t header_size = 6;
  const size_t checksum_size = 4;
  const size_t data_size = sl_png_data_size(header);
  return header_size + 2 * data_size + checksum_size;
}

struct sl_data sl_png_pack_image_data(
    const struct sl_png_image image[static 1]) {
  struct sl_data packed = {0};

  const size_t width = image->header.width;
  const size_t height = image->header.height;
  const size_t bytes_per_px = sl_png_bytes_per_pixel[image->header.color_type];

  packed = sl_data_create(sl_png_data_size(image->header));

  for (size_t row = 0; row < height; ++row) {
    const size_t filter_idx = bytes_per_px * row * width + row;
    packed.data[filter_idx] = sl_png_filter_none;
    for (size_t col = 0; col < width; ++col) {
      const size_t dst_idx = bytes_per_px * (width * row + col) + 1 + row;
      const size_t src_idx =
          bytes_per_px * ((width + 2) * (row + 1) + (col + 1));
      memcpy(packed.data + dst_idx, image->data.data + src_idx, bytes_per_px);
    }
  }

  return packed;
}

void sl_png_unpack_and_pad_image_data(struct sl_png_image image[static 1]) {
  struct sl_data filter = {0};
  struct sl_data padded = {0};

  const size_t width = image->header.width;
  const size_t height = image->header.height;
  const size_t bytes_per_px = sl_png_bytes_per_pixel[image->header.color_type];

  filter = sl_data_create(height);
  padded = sl_data_create(bytes_per_px * (width + 2) * (height + 2));

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
  sl_data_delete(&image->data);
  image->data = padded;
}

bool sl_png_unapply_filter(struct sl_png_image image[static 1]) {
  const size_t width = image->header.width;
  const size_t height = image->header.height;
  const size_t bytes_per_px = sl_png_bytes_per_pixel[image->header.color_type];

  for (size_t row = 1; row < height + 1; ++row) {
    const enum sl_png_filter_type filter = image->filter.data[row - 1];
    if (filter == sl_png_filter_none) {
      continue;
    }
    for (size_t col = 1; col < width + 1; ++col) {
      unsigned char* dst_px = sl_png_image_get_pixel(image, row, col);
      const unsigned char* left_px =
          sl_png_image_get_pixel(image, row, col - 1);
      const unsigned char* above_px =
          sl_png_image_get_pixel(image, row - 1, col);
      const unsigned char* left_above_px =
          sl_png_image_get_pixel(image, row - 1, col - 1);
      for (size_t byte = 0; byte < bytes_per_px; ++byte) {
        switch (filter) {
          case sl_png_filter_sub: {
            dst_px[byte] += left_px[byte];
          } break;
          case sl_png_filter_up: {
            dst_px[byte] += above_px[byte];
          } break;
          case sl_png_filter_avg: {
            dst_px[byte] += (left_px[byte] + above_px[byte]) / 2;
          } break;
          case sl_png_filter_paeth: {
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
            SL_LOG_ERROR("filter not implemented %s",
                         sl_png_filter_types[filter]);
            return false;
          } break;
        }
      }
    }
  }
  return true;
}

struct sl_png_image sl_png_read_image(const char filename[const static 1]) {
  struct sl_png_image image = {0};
  struct sl_data idat = {0};

  struct sl_png_chunks chunks = sl_png_read_chunks(filename);
  if (!chunks.count) {
    goto error;
  }

  image.header = sl_png_parse_header(chunks.chunks[0]);
  if (!sl_png_is_supported(image.header)) {
    SL_LOG_ERROR(("unsupported PNG features in %s\n"
                  "  image must be:\n"
                  "    8-bit/color\n"
                  "    RGB/A\n"
                  "    non-interlaced\n"
                  "    compression=0\n"
                  "    filter=0\n"
                  "  instead, header is:\n"),
                 filename);
    goto error;
  }

  for (size_t i = 1; i < chunks.count; ++i) {
    const struct sl_png_chunk chunk = chunks.chunks[i];
    if (chunk.type == sl_png_IDAT) {
      sl_data_extend(&idat, &chunk.data);
    }
  }
  // TODO parse PLTE if header.color_type == sl_png_indexed

  image.data = sl_data_create(sl_png_data_size(image.header));
  const size_t num_decoded = sl_inflate(image.data, idat);
  if (num_decoded != image.data.size) {
    SL_LOG_ERROR("failed decoding IDAT stream");
    goto error;
  }
  sl_png_unpack_and_pad_image_data(&image);
  if (!sl_png_unapply_filter(&image)) {
    SL_LOG_ERROR("failed unfiltering decoded image");
    goto error;
  }

  sl_png_chunks_destroy(chunks);
  sl_data_delete(&idat);
  return image;

error:
  sl_png_chunks_destroy(chunks);
  sl_data_delete(&idat);
  sl_png_image_destroy(image);
  return (struct sl_png_image){0};
}

bool sl_png_chunk_fwrite_header(FILE stream[const static 1],
                                const struct sl_png_header header) {
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
      sl_misc_encode_big_endian(4, (unsigned char[4]){0}, header.width);
  memcpy(ihdr_data + 4, width, 4);
  const unsigned char* height =
      sl_misc_encode_big_endian(4, (unsigned char[4]){0}, header.height);
  memcpy(ihdr_data + 8, height, 4);
  memcpy(ihdr_data + 12, &header.bit_depth, 1);
  memcpy(ihdr_data + 13, &header.color_type, 1);
  memcpy(ihdr_data + 14, &header.compression, 1);
  memcpy(ihdr_data + 15, &header.filter, 1);
  memcpy(ihdr_data + 16, &header.interlace, 1);
  if (fwrite(ihdr_data, 1, 17, stream) != 17) {
    return false;
  }
  const unsigned char* crc32 = sl_misc_encode_big_endian(
      4,
      (unsigned char[4]){0},
      sl_hash_crc32_bytes(SL_ARRAY_LEN(ihdr_data), ihdr_data));
  if (fwrite(crc32, 1, 4, stream) != 4) {
    return false;
  }
  return true;
}

bool sl_png_chunk_fwrite(FILE stream[const static 1],
                         const char chunk_type[const static 1],
                         const struct sl_data data[const static 1]) {
  if (data->size > ((size_t)1 << 31)) {
    SL_LOG_ERROR("will not write too large %s chunk of size %zu",
                 chunk_type,
                 data->size);
    return false;
  }
  struct sl_data crc_data = sl_data_create(data->size + 4);

  bool ok = false;

  const unsigned char* chunk_len =
      sl_misc_encode_big_endian(4, (unsigned char[4]){0}, data->size);
  if (fwrite(chunk_len, 1, 4, stream) != 4) {
    SL_LOG_ERROR("failed writing %s chunk length", chunk_type);
    goto done;
  }
  memcpy(crc_data.data, chunk_type, 4);
  if (data->size) {
    memcpy(crc_data.data + 4, data->data, data->size);
  }
  if (fwrite(crc_data.data, 1, crc_data.size, stream) != crc_data.size) {
    SL_LOG_ERROR("failed writing %s chunk type + data", chunk_type);
    goto done;
  }
  const unsigned char* crc32 = sl_misc_encode_big_endian(
      4,
      (unsigned char[4]){0},
      sl_hash_crc32_bytes(crc_data.size, crc_data.data));
  if (fwrite(crc32, 1, 4, stream) != 4) {
    SL_LOG_ERROR("failed writing %s chunk CRC32", chunk_type);
    goto done;
  }

  ok = true;

done:
  sl_data_delete(&crc_data);
  return ok;
}

bool sl_png_write_image(const struct sl_png_image image,
                        const char filename[const static 1]) {
  bool write_ok = false;
  struct sl_data packed_data = {0};
  struct sl_data idat = {0};
  FILE* fp = nullptr;

  fp = fopen(filename, "w");
  if (!fp) {
    SL_LOG_ERROR("cannot open %s", filename);
    goto done;
  }
  packed_data = sl_png_pack_image_data(&image);
  idat = sl_data_create(sl_png_idat_max_size(image.header));

  const size_t new_size = sl_deflate_uncompressed(idat, packed_data);
  idat.data = sl_realloc(idat.data, idat.size, new_size, 1);
  idat.size = new_size;

  if (!sl_png_chunk_fwrite_header(fp, image.header)) {
    SL_LOG_ERROR("failed writing PNG header to %s", filename);
    goto done;
  }
  if (!sl_png_chunk_fwrite(fp, "IDAT", &idat)) {
    SL_LOG_ERROR("failed writing PNG IDAT chunks to %s", filename);
    goto done;
  }
  if (!sl_png_chunk_fwrite(fp, "IEND", &(struct sl_data){0})) {
    SL_LOG_ERROR("failed writing PNG IEND chunk to %s", filename);
    goto done;
  }

  write_ok = true;

done:
  if (fp) {
    fclose(fp);
  }
  sl_data_delete(&idat);
  sl_data_delete(&packed_data);
  return write_ok;
}

#endif  // SL_PNG_H_INCLUDED
