#ifndef _STUFFLIB_PNG_H_INCLUDED
#define _STUFFLIB_PNG_H_INCLUDED
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_misc.h"

struct stufflib_png_header {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t color_type;
  uint8_t compression;
  uint8_t filter;
  uint8_t interlace;
};

struct stufflib_png_image {
  struct stufflib_png_header header;
  size_t size;
  unsigned char* data;
};

struct _stufflib_png_chunk {
  uint32_t length;
  uint32_t crc32;
  unsigned char type[5];
  unsigned char* data;
};

struct stufflib_png_header stufflib_png_make_empty_header() {
  struct stufflib_png_header header = {0};
  return header;
}

struct stufflib_png_image stufflib_png_make_empty_image() {
  struct stufflib_png_image image = {0};
  return image;
}

struct _stufflib_png_chunk stufflib_png_make_empty_chunk() {
  struct _stufflib_png_chunk chunk = {0};
  return chunk;
}

void _stufflib_png_dump_chunk(struct _stufflib_png_chunk chunk) {
  printf("%s: %" PRIu32 " bytes crc32: %" PRIu32 "\n",
         chunk.type,
         chunk.length,
         chunk.crc32);
}

struct _stufflib_png_chunk stufflib_png_parse_next_chunk(FILE* fp) {
  struct _stufflib_png_chunk chunk = {0};

  // parse big-endian 32-bit uint chunk length
  {
    const size_t length_len = 4;
    unsigned char buf[length_len];
    if (fread(buf, 1, length_len, fp) != length_len) {
      fprintf(stderr, "error: failed reading PNG chunk length");
      goto error;
    }
    chunk.length = stufflib_parse_big_endian_u32(buf);
  }

  // parse 4-char chunk type
  {
    const size_t type_len = 4;
    if (fread(chunk.type, 1, type_len, fp) != type_len) {
      fprintf(stderr, "error: failed reading PNG chunk type");
      goto error;
    }
    chunk.type[type_len] = 0;
  }

  // parse chunk data
  {
    unsigned char* data = calloc(chunk.length, sizeof(unsigned char));
    if (!data) {
      goto error;
    }
    if (fread(data, 1, chunk.length, fp) != chunk.length) {
      fprintf(stderr, "error: failed reading PNG chunk data");
      goto error;
    }
    chunk.data = data;
  }

  // parse 32-bit checksum
  {
    const size_t crc32_len = 4;
    unsigned char buf[crc32_len];
    if (fread(buf, 1, crc32_len, fp) != crc32_len) {
      fprintf(stderr, "error: failed reading PNG chunk crc32");
      goto error;
    }
    // TODO CRC
    chunk.crc32 = stufflib_parse_big_endian_u32(buf);
  }

  return chunk;

error:
  return stufflib_png_make_empty_chunk();
}

int stufflib_png_is_supported(struct stufflib_png_header header) {
  return (header.compression == 0 && header.filter == 0 &&
          header.interlace == 0 &&
          (header.color_type == 2 || header.color_type == 6) &&
          header.bit_depth == 8);
}

struct stufflib_png_header stufflib_png_parse_header(
    struct _stufflib_png_chunk chunk) {
  struct stufflib_png_header header = {0};

  size_t chunk_data_pos = 0;

  // parse big-endian 32-bit uint PNG width and height
  {
    size_t field_len = 4;
    unsigned char buf[field_len];

    memcpy(buf, &(chunk.data[chunk_data_pos]), field_len);
    chunk_data_pos += field_len;
    header.width = stufflib_parse_big_endian_u32(buf);

    memcpy(buf, &(chunk.data[chunk_data_pos]), field_len);
    chunk_data_pos += field_len;
    header.height = stufflib_parse_big_endian_u32(buf);
  }

  // parse single byte fields
  {
    const size_t rest_len = 5;
    unsigned char buf[rest_len];

    memcpy(buf, &(chunk.data[chunk_data_pos]), rest_len);
    chunk_data_pos += rest_len;
    header.bit_depth = (uint8_t)(buf[0]);
    header.color_type = (uint8_t)(buf[1]);
    header.compression = (uint8_t)(buf[2]);
    header.filter = (uint8_t)(buf[3]);
    header.interlace = (uint8_t)(buf[4]);
  }

  return header;
}

const char* stufflib_png_format_color_type(uint8_t color_type) {
  color_type = (color_type < 7) ? color_type : 7;
  return (const char* const[]){
      [0] = "grayscale",
      [2] = "truecolor (rgb)",
      [3] = "indexed",
      [4] = "grayscale alpha",
      [6] = "truecolor alpha (rgba)",
      [7] = 0,
  }[color_type];
}

void stufflib_png_dump_img_meta(FILE* stream, struct stufflib_png_image image) {
  fprintf(stream, "  data length: %zu\n", image.size);
  fprintf(stream, "  data begin: %p\n", image.data);
  struct stufflib_png_header header = image.header;
  fprintf(stream, "  width: %" PRIu32 "\n", header.width);
  fprintf(stream, "  height: %" PRIu32 "\n", header.height);
  fprintf(stream, "  bit depth: %" PRIu8 "\n", header.bit_depth);
  fprintf(stream,
          "  color type: %s\n",
          stufflib_png_format_color_type(header.color_type));
  fprintf(stream, "  compression: %" PRIu8 "\n", header.compression);
  fprintf(stream, "  filter: %" PRIu8 "\n", header.filter);
  fprintf(stream, "  interlace: %" PRIu8 "\n", header.interlace);
}

struct stufflib_png_image stufflib_png_read(const char* filename) {
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
      fprintf(stderr, "error: failed reading header of %s\n", filename);
      goto error;
    }
    if (!(buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G')) {
      fprintf(stderr, "error: not a PNG image: %s\n", filename);
      goto error;
    }
  }

  struct stufflib_png_image image = {0};

  {
    // file claims to be PNG, read the PNG header block
    struct _stufflib_png_chunk header_chunk = stufflib_png_parse_next_chunk(fp);
    if (header_chunk.length == 0) {
      goto corrupted_img_error;
    }
    struct stufflib_png_header header = stufflib_png_parse_header(header_chunk);
    free(header_chunk.data);
    if (header.width == 0 || header.height == 0) {
      goto corrupted_img_error;
    }
    image.header = header;
    if (!stufflib_png_is_supported(header)) {
      goto unsupported_img_error;
    }
  }

  // TODO PLTE if color type 3

  size_t num_data = 0;
  // read chunks until end of image
  struct _stufflib_png_chunk data_chunk = stufflib_png_make_empty_chunk();
  while (memcmp(data_chunk.type, "IEND", 4)) {
    data_chunk = stufflib_png_parse_next_chunk(fp);
    if (data_chunk.type[0] == 0) {
      goto corrupted_img_error;
    }
    if (!memcmp(data_chunk.type, "IDAT", 4)) {
      ++num_data;
      if (num_data > 1) {
        fprintf(stderr, "TODO handle multiple IDAT chunks\n");
        free(data_chunk.data);
        goto unsupported_img_error;
      }
      image.data = data_chunk.data;
      image.size += (size_t)(data_chunk.length);
    } else {
      free(data_chunk.data);
    }
  }

  fclose(fp);
  return image;

unsupported_img_error:
  fprintf(stderr, "error: unsupported PNG features in: %s\n", filename);
  stufflib_png_dump_img_meta(stderr, image);
  fprintf(stderr,
          "error: image must be 8-bit/color RGB/A non-interlaced, "
          "compression=0, filter=0\n");
  goto error;
corrupted_img_error:
  fprintf(stderr, "error: corrupted PNG image: %s\n", filename);
  goto error;
error:
  if (fp) {
    fclose(fp);
  }
  return stufflib_png_make_empty_image();
}

#endif  // _STUFFLIB_PNG_H_INCLUDED
