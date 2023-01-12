#ifndef _STUFFLIB_PNG_H_INCLUDED
#define _STUFFLIB_PNG_H_INCLUDED
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stufflib_inflate.h"
#include "stufflib_misc.h"

typedef struct stufflib_png_header stufflib_png_header;
struct stufflib_png_header {
  size_t size;
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t color_type;
  uint8_t compression;
  uint8_t filter;
  uint8_t interlace;
};

typedef struct stufflib_png_image stufflib_png_image;
struct stufflib_png_image {
  stufflib_png_header header;
  size_t size;
  unsigned char* data;
};

typedef struct _stufflib_png_chunk _stufflib_png_chunk;
struct _stufflib_png_chunk {
  size_t size;
  uint32_t length;
  uint32_t crc32;
  unsigned char type[5];
  unsigned char* data;
};

void _stufflib_png_dump_chunk(_stufflib_png_chunk chunk) {
  printf("%s: %" PRIu32 " bytes crc32: %" PRIu32 "\n",
         chunk.type,
         chunk.length,
         chunk.crc32);
}

_stufflib_png_chunk stufflib_png_parse_next_chunk(FILE* fp) {
  _stufflib_png_chunk chunk = {0};

  // parse big-endian 32-bit uint chunk length
  {
    const size_t length_len = 4;
    unsigned char buf[length_len];
    if (fread(buf, 1, length_len, fp) != length_len) {
      fprintf(stderr, "error: failed reading PNG chunk length\n");
      goto error;
    }
    chunk.length = stufflib_misc_parse_big_endian_u32(buf);
    chunk.size += length_len;
  }

  // parse 4-char chunk type
  {
    const size_t type_len = 4;
    if (fread(chunk.type, 1, type_len, fp) != type_len) {
      fprintf(stderr, "error: failed reading PNG chunk type\n");
      goto error;
    }
    chunk.type[type_len] = 0;
    chunk.size += type_len;
  }

  // parse chunk data
  {
    unsigned char* data = calloc(chunk.length, 1);
    if (!data) {
      goto error;
    }
    if (fread(data, 1, chunk.length, fp) != chunk.length) {
      fprintf(stderr, "error: failed reading PNG chunk data\n");
      free(data);
      goto error;
    }
    chunk.data = data;
    chunk.size += chunk.length;
  }

  // parse 32-bit checksum
  {
    const size_t crc32_len = 4;
    unsigned char buf[crc32_len];
    if (fread(buf, 1, crc32_len, fp) != crc32_len) {
      fprintf(stderr, "error: failed reading PNG chunk crc32");
      free(chunk.data);
      goto error;
    }
    // TODO CRC
    chunk.crc32 = stufflib_misc_parse_big_endian_u32(buf);
    chunk.size += crc32_len;
  }

  return chunk;

error:
  return (_stufflib_png_chunk){0};
}

int stufflib_png_is_supported(stufflib_png_header header) {
  return (header.compression == 0 && header.filter == 0 &&
          header.interlace == 0 &&
          (header.color_type == 2 || header.color_type == 6) &&
          header.bit_depth == 8);
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

void stufflib_png_dump_header(FILE* stream, stufflib_png_header header) {
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

void stufflib_png_dump_img_meta(FILE* stream, stufflib_png_image image) {
  fprintf(stream, "  data length: %zu\n", image.size);
  fprintf(stream, "  data begin: %p\n", image.data);
  stufflib_png_dump_header(stream, image.header);
}

void stufflib_png_dump_img_data(FILE* stream,
                                stufflib_png_image image,
                                const size_t count) {
  const size_t bytes_per_line = 40;
  for (size_t i = 0; i < count && i < image.size; ++i) {
    if (i) {
      if (i % bytes_per_line == 0) {
        fprintf(stream, "\n");
      } else if (i % 2 == 0) {
        fprintf(stream, " ");
      }
    }
    fprintf(stream, "%02x", image.data[i]);
  }
  fprintf(stream, "\n");
}

stufflib_png_header stufflib_png_read_header(const char* filename) {
  stufflib_png_header header = {0};

  FILE* fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "error: cannot open %s\n", filename);
    goto done;
  }

  // check for 8 byte file header containing 'PNG' in ascii
  {
    const size_t header_len = 8;
    unsigned char buf[header_len];
    if (fread(buf, 1, header_len, fp) != header_len) {
      fprintf(stderr, "error: failed reading header of %s\n", filename);
      goto done;
    }
    if (!(buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G')) {
      fprintf(stderr, "error: not a PNG image: %s\n", filename);
      goto done;
    }
    header.size += header_len;
  }

  {
    // file claims to be PNG, read the PNG header block
    _stufflib_png_chunk chunk = stufflib_png_parse_next_chunk(fp);
    if (chunk.length == 0) {
      goto corrupted_header_error;
    }

    header.size += chunk.size;

    size_t chunk_data_pos = 0;

    // parse big-endian 32-bit uint PNG width and height
    {
      size_t field_len = 4;
      unsigned char buf[field_len];

      memcpy(buf, chunk.data + chunk_data_pos, field_len);
      chunk_data_pos += field_len;
      header.width = stufflib_misc_parse_big_endian_u32(buf);

      memcpy(buf, chunk.data + chunk_data_pos, field_len);
      chunk_data_pos += field_len;
      header.height = stufflib_misc_parse_big_endian_u32(buf);
    }

    // parse single byte fields
    {
      const size_t rest_len = 5;
      unsigned char buf[rest_len];

      memcpy(buf, chunk.data + chunk_data_pos, rest_len);
      chunk_data_pos += rest_len;
      header.bit_depth = (uint8_t)(buf[0]);
      header.color_type = (uint8_t)(buf[1]);
      header.compression = (uint8_t)(buf[2]);
      header.filter = (uint8_t)(buf[3]);
      header.interlace = (uint8_t)(buf[4]);
    }

    free(chunk.data);
    if (header.width == 0 || header.height == 0) {
      goto corrupted_header_error;
    }
    if (!stufflib_png_is_supported(header)) {
      goto unsupported_header_error;
    }
  }

  goto done;

unsupported_header_error:
  fprintf(stderr,
          ("error: unsupported PNG features in %s\n"
           "image must be:\n"
           "  8-bit/color\n"
           "  RGB/A\n"
           "  non-interlaced\n"
           "  compression=0\n"
           "  filter=0\n"
           "instead, header is:\n"),
          filename);
  stufflib_png_dump_header(stderr, header);
  goto error;

corrupted_header_error:
  fprintf(stderr, "error: corrupted PNG header in %s\n", filename);
  goto error;

error:
  header = (stufflib_png_header){0};

done:
  if (fp) {
    fclose(fp);
  }
  return header;
}

stufflib_png_image stufflib_png_read_image(const char* filename) {
  stufflib_png_image image = {0};

  FILE* fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "error: cannot open %s\n", filename);
    goto error;
  }

  image.header = stufflib_png_read_header(filename);
  if (!(image.header.width && image.header.height)) {
    goto error;
  }

  if (fseek(fp, image.header.size, SEEK_SET)) {
    fprintf(stderr,
            "error: failed seeking %zu bytes past PNG header in %s\n",
            image.header.size,
            filename);
    goto error;
  }

  // TODO PLTE if color type 3

  size_t num_data = 0;
  // read chunks until end of image
  _stufflib_png_chunk data_chunk = {0};
  while (memcmp(data_chunk.type, "IEND", 4) != 0) {
    data_chunk = stufflib_png_parse_next_chunk(fp);
    if (data_chunk.type[0] == 0) {
      free(data_chunk.data);
      goto corrupted_image_error;
    }
    if (memcmp(data_chunk.type, "IDAT", 4) == 0) {
      ++num_data;
      if (num_data > 1) {
        fprintf(stderr, "TODO handle multiple IDAT chunks\n");
        free(data_chunk.data);
        goto corrupted_image_error;
      }
      stufflib_data img_data =
          stufflib_inflate(data_chunk.length, data_chunk.data);
      if (!img_data.size) {
        free(data_chunk.data);
        goto corrupted_image_error;
      }
      image.data = img_data.data;
      image.size = img_data.size;
    }
    free(data_chunk.data);
  }

  goto done;

corrupted_image_error:
  fprintf(stderr, "error: corrupted PNG image %s\n", filename);
  goto error;

error:
  image = (stufflib_png_image){0};

done:
  if (fp) {
    fclose(fp);
  }
  return image;
}

void stufflib_png_destroy(stufflib_png_image* image) {
  if (image) {
    free(image->data);
    *image = (stufflib_png_image){0};
  }
}

#endif  // _STUFFLIB_PNG_H_INCLUDED
