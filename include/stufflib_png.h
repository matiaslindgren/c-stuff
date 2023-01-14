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
    "",
    "rgb",
    "indexed",
    "grayscale with alpha",
    "",
    "rgba",
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
};

void stufflib_png_chunk_destroy(stufflib_png_chunk chunk) {
  stufflib_misc_data_destroy(chunk.data);
}

void stufflib_png_chunks_destroy(stufflib_png_chunks chunks) {
  for (size_t i = 0; i < chunks.count; ++i) {
    stufflib_png_chunk_destroy(chunks.chunks[i]);
  }
  free(chunks.chunks);
}

void stufflib_png_image_destroy(stufflib_png_image image) {
  stufflib_misc_data_destroy(image.data);
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
  fprintf(stream, "  data length: %zu\n", image.data.size);
  fprintf(stream, "  data begin: %p\n", image.data.data);
  stufflib_png_dump_header(stream, image.header);
}

void stufflib_png_dump_img_data(FILE stream[const static 1],
                                const stufflib_png_image image) {
  const size_t bytes_per_line = 40;
  for (size_t i = 0; i < image.data.size; ++i) {
    if (i) {
      if (i % bytes_per_line == 0) {
        fprintf(stream, "\n");
      } else if (i % 2 == 0) {
        fprintf(stream, " ");
      }
    }
    fprintf(stream, "%02x", image.data.data[i]);
  }
  fprintf(stream, "\n");
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

size_t stufflib_png_data_size(stufflib_png_header header) {
  // TODO properly
  const int bytes_per_pixel = 3 + (header.color_type == stufflib_png_rgba);
  return header.height + header.width * header.height * bytes_per_pixel;
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

  image.data.size = stufflib_png_data_size(image.header);
  image.data.data = calloc(image.data.size, sizeof(unsigned char));
  if (!image.data.data) {
    fprintf(stderr, "failed allocating output buffer\n");
    goto error;
  }
  const size_t num_decoded = stufflib_inflate(image.data, idat);
  if (num_decoded != image.data.size) {
    fprintf(stderr, "failed decoding IDAT stream\n");
    goto error;
  }

  stufflib_png_chunks_destroy(chunks);
  stufflib_misc_data_destroy(idat);
  return image;

error:
  stufflib_png_chunks_destroy(chunks);
  stufflib_misc_data_destroy(idat);
  stufflib_png_image_destroy(image);
  return (stufflib_png_image){0};
}

#endif  // _STUFFLIB_PNG_H_INCLUDED
