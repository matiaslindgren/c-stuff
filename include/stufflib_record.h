#ifndef SL_RECORD_H_INCLUDED
#define SL_RECORD_H_INCLUDED

#include <stdio.h>

#include "stufflib_linalg.h"
#include "stufflib_macros.h"
#include "stufflib_misc.h"

struct sl_record {
  char layout[8];
  char type[64];
  char name[128];
  char path[2048];
  size_t pos;
  size_t size;
  int n_dims;
  size_t dim_size[8];
};

bool sl_record_is_valid(const struct sl_record r[const static 1]) {
  if (!r->layout || !r->type || !r->name || !r->path || !r->n_dims) {
    return false;
  }
  bool is_sparse = strcmp(r->layout, "sparse") == 0;
  bool is_dense = strcmp(r->layout, "dense") == 0;
  if ((!is_sparse && !is_dense) || (is_dense && !r->size)) {
    return false;
  }
  bool is_float = strcmp(r->type, "float") == 0;
  bool is_int = strcmp(r->type, "int") == 0;
  if (!is_float && !is_int) {
    return false;
  }
  for (int d = 0; d < r->n_dims; ++d) {
    if (r->dim_size[d] == 0) {
      return false;
    }
  }
  return true;
}

bool sl_record_read_metadata(struct sl_record record[const static 1],
                             const char path[const static 1],
                             const char name[const static 1]) {
  FILE* fp_meta = nullptr;
  bool ok = false;

  char inpath[256] = {0};
  if (!sl_misc_format_path(SL_ARRAY_LEN(inpath),
                           inpath,
                           path,
                           name,
                           "sl_record_meta")) {
    goto done;
  }

  fp_meta = fopen(inpath, "r");
  if (!fp_meta) {
    SL_LOG_ERROR("cannot open file %s for reading", inpath);
    goto done;
  }
  strcpy(record->path, path);
  strcpy(record->name, name);

  if (EOF == fscanf(fp_meta,
                    "name: %s\ntype: %s\nlayout: %s\nsize: %zu\ndims: %d\n",
                    record->name,
                    record->type,
                    record->layout,
                    &(record->size),
                    &(record->n_dims)) ||
      record->n_dims == 0) {
    SL_LOG_ERROR("failed reading %s", inpath);
    goto done;
  }
  for (int d = 0; d < record->n_dims; ++d) {
    if (EOF == fscanf(fp_meta, "dim%*d: %zu\n", record->dim_size + d)) {
      SL_LOG_ERROR("failed reading dimension %d from %s", d, inpath);
      goto done;
    }
  }

  if (!sl_record_is_valid(record)) {
    SL_LOG_ERROR("read metadata from %s/%s but it is invalid", path, name);
    goto done;
  }

  ok = true;
done:
  if (fp_meta) {
    fclose(fp_meta);
    fp_meta = nullptr;
  }
  return ok;
}

bool sl_record_write_metadata(const struct sl_record record[const static 1]) {
  FILE* fp_meta = nullptr;
  bool ok = false;

  if (!sl_record_is_valid(record)) {
    SL_LOG_ERROR("record metadata must not contain nulls");
    goto done;
  }

  char meta_path[256] = {0};
  if (!sl_misc_format_path(SL_ARRAY_LEN(meta_path),
                           meta_path,
                           record->path,
                           record->name,
                           "sl_record_meta")) {
    goto done;
  }

  fp_meta = fopen(meta_path, "w");
  if (!fp_meta) {
    SL_LOG_ERROR("cannot open file %s for writing", meta_path);
    goto done;
  }

  if (0 > fprintf(fp_meta,
                  "name: %s\ntype: %s\nlayout: %s\nsize: %zu\ndims: %d\n",
                  record->name,
                  record->type,
                  record->layout,
                  record->size,
                  record->n_dims)) {
    SL_LOG_ERROR("failed writing %s", meta_path);
    goto done;
  }
  for (int d = 0; d < record->n_dims; ++d) {
    if (0 > fprintf(fp_meta, "dim%d: %zu\n", d, record->dim_size[d])) {
      SL_LOG_ERROR("failed writing dimension %d to %s", d, meta_path);
      goto done;
    }
  }

  ok = true;
done:
  if (fp_meta) {
    fclose(fp_meta);
  }
  return ok;
}

bool sl_record_append_data(struct sl_record record[const static 1],
                           void* raw_data,
                           const size_t count) {
  FILE* fp_data = nullptr;
  FILE* fp_sparse_index = nullptr;
  bool ok = false;

  if (!sl_record_is_valid(record)) {
    SL_LOG_ERROR("record metadata must not contain nulls");
    goto done;
  }

  char data_path[256] = {0};
  if (!sl_misc_format_path(SL_ARRAY_LEN(data_path),
                           data_path,
                           path,
                           record->path,
                           record->name,
                           "sl_record_data")) {
    goto done;
  }

  fp_data = fopen(data_path, "ab");
  if (!fp_data) {
    SL_LOG_ERROR("cannot open file %s for appending data", data_path);
    goto done;
  }

  unsigned char* data = raw_data;
  size_t size = 0;
  if (strcmp(record->type, "float")) {
    size = sizeof(float);
  } else if (strcmp(record->type, "int")) {
    size = sizeof(int);
  } else {
    SL_LOG_ERROR("unknown type '%s'", record->type);
    goto done;
  }

  // TODO enum
  if (strcmp(record->layout, "sparse") == 0) {
    char sparse_index_path[256] = {0};
    if (!sl_misc_format_path(SL_ARRAY_LEN(sparse_index_path),
                             sparse_index_path,
                             path,
                             name,
                             "sl_record_sparse_index")) {
      goto done;
    }

    fp_sparse_index = fopen(sparse_index_path, "ab");
    if (!fp_sparse_index) {
      SL_LOG_ERROR("cannot open file %s for appending sparse index",
                   sparse_index_path);
      goto done;
    }

    size_t prev_pos = record->pos;
    for (size_t i = 0; i < count; ++i) {
      const size_t pos = record->pos + i;
      unsigned char* value = data + pos * size;
      if (!sl_misc_is_zero(size, value)) {
        if (1 != fwrite(value, size, 1, fp_data)) {
          SL_LOG_ERROR("failed appending data to %s", data_path);
          goto done;
        }
        const int offset = (int)(pos - prev_pos);
        if (1 != fwrite(&offset, sizeof(int), 1, fp_sparse_index)) {
          SL_LOG_ERROR("failed appending sparse index to %s",
                       sparse_index_path);
          goto done;
        }
        prev_pos = pos;
        record->size += 1;
      }
    }
  } else {
    if (count != fwrite(data, size, count, fp_data)) {
      SL_LOG_ERROR("failed appending data to %s", data_path);
      goto done;
    }
  }

  record->pos += count;

  ok = true;
done:
  if (fp_data) {
    fclose(fp_data);
  }
  if (fp_sparse_index) {
    fclose(fp_sparse_index);
  }
  return ok;
}

bool sl_record_read_data(struct sl_record record[const static 1],
                         void* raw_data) {
  FILE* fp_data = nullptr;
  FILE* fp_sparse_index = nullptr;
  bool ok = false;

  char data_path[256] = {0};
  if (!sl_misc_format_path(SL_ARRAY_LEN(data_path),
                           data_path,
                           path,
                           record->path,
                           record->name,
                           "sl_record_data")) {
    goto done;
  }

  fp_data = fopen(data_path, "rb");
  if (!fp_data) {
    SL_LOG_ERROR("cannot open file %s for reading data", data_path);
    goto done;
  }

  unsigned char* data = raw_data;
  size_t size = 0;
  if (strcmp(record->type, "float")) {
    size = sizeof(float);
  } else if (strcmp(record->type, "int")) {
    size = sizeof(int);
  } else {
    SL_LOG_ERROR("unknown type '%s'", record->type);
    goto done;
  }

  // TODO enum
  if (strcmp(record->layout, "sparse") == 0) {
    char sparse_index_path[256] = {0};
    if (!sl_misc_format_path(SL_ARRAY_LEN(sparse_index_path),
                             sparse_index_path,
                             path,
                             name,
                             "sl_record_sparse_index")) {
      goto done;
    }

    fp_sparse_index = fopen(sparse_index_path, "rb");
    if (!fp_sparse_index) {
      SL_LOG_ERROR("cannot open file %s for reading", sparse_index_path);
      goto done;
    }

    size_t pos = 0;
    for (size_t i = 0; i < record->size; ++i) {
      int offset = -1;
      if ((1 != fread(&offset, sizeof(int), 1, fp_sparse_index)) ||
          offset < 0) {
        SL_LOG_ERROR("failed reading sparse index from %s", sparse_index_path);
        goto done;
      }
      pos += (size_t)offset;
      if (1 != fread(data + pos * size, size, 1, fp_data)) {
        SL_LOG_ERROR("failed reading data from %s", data_path);
        goto done;
      }
    }
  } else {
    if (record->size != fread(data, size, record->size, fp_data)) {
      SL_LOG_ERROR("failed reading data from %s", data_path);
      goto done;
    }
  }

  ok = true;
done:
  if (fp_data) {
    fclose(fp_data);
  }
  if (fp_sparse_index) {
    fclose(fp_sparse_index);
  }
  return ok;
}

#endif  // SL_RECORD_H_INCLUDED
