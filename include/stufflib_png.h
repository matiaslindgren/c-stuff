#ifndef _STUFFLIB_PNG_H_INCLUDED
#define _STUFFLIB_PNG_H_INCLUDED
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_inflate.h"
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
    "0",    "IHDR", "PLTE", "IDAT", "IEND", "bKGD", "cHRM", "dSIG",
    "eXIf", "gAMA", "hIST", "iCCP", "iTXt", "pHYs", "sBIT", "sPLT",
    "sRGB", "sTER", "tEXt", "tIME", "tRNS", "zTXt",
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
    "grayscale",
    0,
    "rgb",
    "indexed",
    "grayscale with alpha",
    0,
    "rgba",
};

size_t stufflib_png_bytes_per_pixel[] = {
    // TODO
    [stufflib_png_grayscale] = 1,
    0,
    [stufflib_png_rgb] = 3,
    [stufflib_png_indexed] = 0,
    [stufflib_png_grayscale_alpha] = 2,
    0,
    [stufflib_png_rgba] = 4,
    0,
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
    "None",
    "Sub",
    "Up",
    "Average",
    "Paeth",
};

typedef struct stufflib_png_header stufflib_png_header;
struct stufflib_png_header {
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
  uint32_t adler32;
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

void stufflib_png_image_destroy(stufflib_png_image image) {
  stufflib_misc_data_destroy(&image.data);
  stufflib_misc_data_destroy(&image.filter);
}

int stufflib_png_image_copy(stufflib_png_image dst[restrict static 1],
                            const stufflib_png_image src[restrict static 1]) {
  dst->header = src->header;
  if (!stufflib_misc_data_copy(&dst->data, &src->data)) {
    return 0;
  }
  if (!stufflib_misc_data_copy(&dst->filter, &src->filter)) {
    return 0;
  }
  return 1;
}

unsigned char* stufflib_png_image_get_pixel(
    const stufflib_png_image image[static 1],
    const size_t row,
    const size_t col) {
  const size_t width = image->header.width;
  const size_t bytes_per_px =
      stufflib_png_bytes_per_pixel[image->header.color_type];
  const size_t idx = (row * (width + 2) + col) * bytes_per_px;
  return image->data.data + idx;
}

int stufflib_png_is_supported(const stufflib_png_header header) {
  return (header.compression == 0 && header.filter == 0 &&
          header.interlace == 0 &&
          (header.color_type == stufflib_png_rgb ||
           header.color_type == stufflib_png_rgba) &&
          header.bit_depth == 8);
}

void stufflib_png_dump_header(FILE stream[const static 1],
                              const stufflib_png_header header) {
  fprintf(stream, "  width: %zu\n", header.width);
  fprintf(stream, "  height: %zu\n", header.height);
  fprintf(stream, "  bit depth: %u\n", header.bit_depth);
  fprintf(stream,
          "  color type: %s\n",
          stufflib_png_color_types[header.color_type]);
  fprintf(stream, "  compression: %u\n", header.compression);
  fprintf(stream, "  filter: %u\n", header.filter);
  fprintf(stream, "  interlace: %u\n", header.interlace);
}

void stufflib_png_dump_img_meta(FILE stream[const static 1],
                                const stufflib_png_image image) {
  fprintf(stream, "  filter length: %zu\n", image.filter.size);
  fprintf(stream, "  data length: %zu\n", image.data.size);
  fprintf(stream, "  data begin: %p\n", image.data.data);
  stufflib_png_dump_header(stream, image.header);
}

void stufflib_png_dump_filter_freq(FILE stream[const static 1],
                                   const stufflib_data filter) {
  size_t freq[stufflib_png_num_filter_types] = {0};
  for (size_t i = 0; i < filter.size; ++i) {
    ++freq[filter.data[i]];
  }
  fprintf(stream, "  %7s %12s\n", "filter", "count");
  for (size_t filter = 0; filter < stufflib_png_num_filter_types; ++filter) {
    const char* filter_name = stufflib_png_filter_types[filter];
    fprintf(stream, "  %7s %12zu\n", filter_name, freq[filter]);
  }
}

void _stufflib_png_dump_chunk(const stufflib_png_chunk chunk) {
  printf("%s: %zu bytes adler32: %" PRIu32 "\n",
         stufflib_png_chunk_types[chunk.type],
         chunk.data.size,
         chunk.adler32);
}

enum stufflib_png_chunk_type _stufflib_png_find_chunk_type(
    const char* type_id) {
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
      fprintf(stderr, "error: failed reading PNG chunk length\n");
      goto error;
    }
    chunk.data.size = stufflib_misc_parse_big_endian_u32(length_buf);
  }

  {
    const size_t type_len = 4;
    char chunk_type[type_len + 1];
    if (fread(chunk_type, 1, type_len, fp) != type_len) {
      fprintf(stderr, "error: failed reading PNG chunk type\n");
      goto error;
    }
    chunk.type = _stufflib_png_find_chunk_type(chunk_type);
  }

  if (chunk.data.size) {
    chunk.data.data = calloc(chunk.data.size, sizeof(unsigned char));
    if (!chunk.data.data) {
      goto error;
    }
    if (fread(chunk.data.data, 1, chunk.data.size, fp) != chunk.data.size) {
      fprintf(stderr, "error: failed reading PNG chunk data\n");
      goto error;
    }
  }

  {
    const size_t adler32_len = 4;
    unsigned char adler32_buf[adler32_len];
    if (fread(adler32_buf, 1, adler32_len, fp) != adler32_len) {
      fprintf(stderr, "error: failed reading PNG chunk adler32");
      goto error;
    }
    chunk.adler32 = stufflib_misc_parse_big_endian_u32(adler32_buf);
  }

  return chunk;

error:
  stufflib_png_chunk_destroy(chunk);
  return (stufflib_png_chunk){0};
}

stufflib_png_header stufflib_png_parse_header(const stufflib_png_chunk chunk) {
  assert(chunk.type == stufflib_png_IHDR);
  unsigned char* data = chunk.data.data;
  return (stufflib_png_header){
      .width = stufflib_misc_parse_big_endian_u32(data),
      .height = stufflib_misc_parse_big_endian_u32(data + 4),
      .bit_depth = (unsigned)(data[8]),
      .color_type = (enum stufflib_png_color_type)(data[9]),
      .compression = (unsigned)(data[10]),
      .filter = (unsigned)(data[11]),
      .interlace = (unsigned)(data[12]),
  };
}

stufflib_png_chunks stufflib_png_read_chunks(const char* filename) {
  FILE* fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "error: cannot open %s\n", filename);
    goto error;
  }

  // check for 8 byte file header containing 'PNG' in ascii
  {
    const size_t header_len = 8;
    unsigned char buf[header_len];
    if (fread(buf, 1, header_len, fp) != header_len) {
      fprintf(stderr, "error: failed reading PNG header\n");
      goto error;
    }
    if (!(buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G')) {
      fprintf(stderr, "error: not a PNG image\n");
      goto error;
    }
  }

  size_t count = 0;
  stufflib_png_chunk* chunks = 0;

  while (!count || chunks[count - 1].type != stufflib_png_IEND) {
    stufflib_png_chunk chunk = stufflib_png_read_next_chunk(fp);
    if (chunk.type == stufflib_png_null_chunk) {
      free(chunks);
      fprintf(stderr, "error: unknown chunk\n");
      goto corrupted_image_error;
    }
    chunks = realloc(chunks, (count + 1) * sizeof(stufflib_png_chunk));
    assert(chunks);
    chunks[count++] = chunk;
  }

  fclose(fp);
  return (stufflib_png_chunks){.count = count, .chunks = chunks};

corrupted_image_error:
  fprintf(stderr, "error: cannot read file as PNG: %s\n", filename);
error:
  if (fp) {
    fclose(fp);
  }
  return (stufflib_png_chunks){0};
}

stufflib_png_header stufflib_png_read_header(const char* filename) {
  stufflib_png_chunks chunks = stufflib_png_read_chunks(filename);
  if (!chunks.count) {
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

int stufflib_png_unpack_and_pad_image_data(stufflib_png_image image[static 1]) {
  stufflib_data filter = {0};
  stufflib_data padded = {0};

  const size_t width = image->header.width;
  const size_t height = image->header.height;
  const size_t bytes_per_px =
      stufflib_png_bytes_per_pixel[image->header.color_type];

  filter = stufflib_misc_data_new(height);
  if (!filter.size) {
    fprintf(stderr, "failed allocating scanline filter buffer\n");
    goto error;
  }
  padded = stufflib_misc_data_new(bytes_per_px * (width + 2) * (height + 2));
  if (!padded.size) {
    fprintf(stderr, "failed allocating buffer for padded image data\n");
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
  return 1;

error:
  stufflib_misc_data_destroy(&filter);
  stufflib_misc_data_destroy(&padded);
  return 0;
}

int stufflib_png_unapply_filter(stufflib_png_image image[static 1]) {
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
            fprintf(stderr,
                    "filter not implemented %s\n",
                    stufflib_png_filter_types[filter]);
            return 0;
          } break;
        }
      }
    }
  }
  return 1;
}

stufflib_png_image stufflib_png_read_image(const char* filename) {
  stufflib_png_image image = {0};
  stufflib_data idat = {0};

  stufflib_png_chunks chunks = stufflib_png_read_chunks(filename);
  if (!chunks.count) {
    goto error;
  }

  image.header = stufflib_png_parse_header(chunks.chunks[0]);
  if (!stufflib_png_is_supported(image.header)) {
    fprintf(stderr,
            ("error: unsupported PNG features in %s\n"
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
        fprintf(stderr, "failed concatenating IDAT block\n");
        goto error;
      }
    }
  }
  // TODO parse PLTE if header.color_type == stufflib_png_indexed

  image.data = stufflib_misc_data_new(stufflib_png_data_size(image.header));
  if (!image.data.size) {
    fprintf(stderr, "failed allocating output buffer\n");
    goto error;
  }
  const size_t num_decoded = stufflib_inflate(image.data, idat);
  if (num_decoded != image.data.size) {
    fprintf(stderr, "failed decoding IDAT stream\n");
    goto error;
  }
  if (!stufflib_png_unpack_and_pad_image_data(&image)) {
    fprintf(stderr, "failed padding decoded image\n");
    goto error;
  }
  if (!stufflib_png_unapply_filter(&image)) {
    fprintf(stderr, "failed unfiltering decoded image\n");
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

#endif  // _STUFFLIB_PNG_H_INCLUDED
