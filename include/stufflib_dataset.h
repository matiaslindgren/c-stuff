#ifndef SL_DATASET_H_INCLUDED
#define SL_DATASET_H_INCLUDED

#include <stdio.h>

#include "stufflib_linalg.h"
#include "stufflib_macros.h"

struct sl_ds_dataset {
  char type[8];
  char name[128];
  char path[2048];
  size_t pos;
  size_t size;
  int n_dims;
  size_t dim_size[8];
};

bool sl_ds_is_valid(const struct sl_ds_dataset ds[const static 1]) {
  bool is_sparse = strcmp(ds->type, "sparse") == 0;
  bool is_dense = strcmp(ds->type, "dense") == 0;
  if (!is_sparse && !is_dense) {
    return false;
  }
  if (!ds->type || !ds->name || !ds->path || (is_dense && !ds->size) ||
      !ds->n_dims) {
    return false;
  }
  for (int d = 0; d < ds->n_dims; ++d) {
    if (ds->dim_size[d] == 0) {
      return false;
    }
  }
  return true;
}

bool sl_ds_read(struct sl_ds_dataset ds[const static 1],
                const char path[const static 1],
                const char name[const static 1]) {
  FILE* fp_meta = nullptr;
  bool ok = false;

  char inpath[256] = {0};
  if (0 >
      snprintf(inpath, SL_ARRAY_LEN(inpath), "%s/%s.sl_ds_meta", path, name)) {
    SL_LOG_ERROR("failed formatting metadata path with prefix %s/%s",
                 path,
                 name);
    goto done;
  }
  strcpy(ds->path, path);
  strcpy(ds->name, name);

  fp_meta = fopen(inpath, "r");
  if (!fp_meta) {
    SL_LOG_ERROR("cannot open file %s for reading", inpath);
    goto done;
  }

  if (EOF == fscanf(fp_meta,
                    "name: %s\ntype: %s\nsize: %zu\ndims: %d\n",
                    ds->name,
                    ds->type,
                    &(ds->size),
                    &(ds->n_dims)) ||
      ds->n_dims == 0) {
    SL_LOG_ERROR("failed reading %s", inpath);
    goto done;
  }
  for (int d = 0; d < ds->n_dims; ++d) {
    if (EOF == fscanf(fp_meta, "dim%*d: %zu\n", ds->dim_size + d)) {
      SL_LOG_ERROR("failed writing dimension %d to %s", d, inpath);
      goto done;
    }
  }

  if (!sl_ds_is_valid(ds)) {
    SL_LOG_ERROR("metadata read from %s/%s is invalid", path, name);
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

bool sl_ds_finalize(const struct sl_ds_dataset ds[const static 1]) {
  FILE* fp_meta = nullptr;
  bool ok = false;

  if (!sl_ds_is_valid(ds)) {
    SL_LOG_ERROR("dataset metadata must not contain nulls");
    goto done;
  }

  char meta_path[256] = {0};
  if (0 > snprintf(meta_path,
                   SL_ARRAY_LEN(meta_path),
                   "%s/%s.sl_ds_meta",
                   ds->path,
                   ds->name)) {
    SL_LOG_ERROR("failed formatting metadata path with prefix %s/%s",
                 ds->path,
                 ds->name);
    goto done;
  }

  fp_meta = fopen(meta_path, "w");
  if (!fp_meta) {
    SL_LOG_ERROR("cannot open file %s for writing", meta_path);
    goto done;
  }

  if (0 > fprintf(fp_meta,
                  "name: %s\ntype: %s\nsize: %zu\ndims: %d\n",
                  ds->name,
                  ds->type,
                  ds->size,
                  ds->n_dims)) {
    SL_LOG_ERROR("failed writing %s", meta_path);
    goto done;
  }
  for (int d = 0; d < ds->n_dims; ++d) {
    if (0 > fprintf(fp_meta, "dim%d: %zu\n", d, ds->dim_size[d])) {
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

bool sl_ds_append(struct sl_ds_dataset ds[const static 1],
                  const struct sl_la_matrix data[const static 1]) {
  FILE* fp_data = nullptr;
  FILE* fp_sparse_index = nullptr;
  bool ok = false;

  if (!sl_ds_is_valid(ds)) {
    SL_LOG_ERROR("dataset metadata must not contain nulls");
    goto done;
  }

  char data_path[256] = {0};
  if (0 > snprintf(data_path,
                   SL_ARRAY_LEN(data_path),
                   "%s/%s.sl_ds_data",
                   ds->path,
                   ds->name)) {
    SL_LOG_ERROR("failed formatting dataset path with prefix %s/%s",
                 ds->path,
                 ds->name);
    goto done;
  }

  fp_data = fopen(data_path, "ab");
  if (!fp_data) {
    SL_LOG_ERROR("cannot open file %s for appending", data_path);
    goto done;
  }

  const size_t count = (size_t)data->rows * (size_t)data->cols;
  // TODO enum
  if (strcmp(ds->type, "sparse") == 0) {
    char sparse_index_path[256] = {0};
    if (0 > snprintf(sparse_index_path,
                     SL_ARRAY_LEN(sparse_index_path),
                     "%s/%s.sl_ds_sparse_index",
                     ds->path,
                     ds->name)) {
      SL_LOG_ERROR("failed formatting sparse index path with prefix %s/%s",
                   ds->path,
                   ds->name);
      goto done;
    }

    fp_sparse_index = fopen(sparse_index_path, "ab");
    if (!fp_sparse_index) {
      SL_LOG_ERROR("cannot open file %s for appending", sparse_index_path);
      goto done;
    }

    size_t prev_pos = ds->pos;
    for (size_t i = 0; i < count; ++i) {
      const size_t pos = ds->pos + i;
      if (data->data[pos]) {
        if (1 != fwrite(data->data + pos, sizeof(float), 1, fp_data)) {
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
        ds->size += 1;
      }
    }
  } else {
    if (count != fwrite(data->data, sizeof(float), count, fp_data)) {
      SL_LOG_ERROR("failed appending data to %s", data_path);
      goto done;
    }
  }

  ds->pos += count;

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

#endif  // SL_DATASET_H_INCLUDED
