#ifndef SL_RECORD_H_INCLUDED
#define SL_RECORD_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stufflib_io.h"
#include "stufflib_macros.h"

struct sl_record {
  char layout[8];  // TODO enum
  char type[64];   // TODO enum
  char name[128];
  char path[2048];
  size_t size;
  int n_dims;
  size_t dim_size[8];
};

bool sl_record_validate_metadata(const struct sl_record r[const static 1]) {
  if (!r->layout || !r->type || !r->name || !r->path || !r->n_dims) {
    SL_LOG_ERROR("record contains nulls");
    return false;
  }

  bool is_sparse = SL_STR_EQ(r->layout, "sparse");
  bool is_dense = SL_STR_EQ(r->layout, "dense");
  if ((!is_sparse && !is_dense) || (is_dense && !r->size)) {
    SL_LOG_ERROR("invalid data layout '%s'", r->layout);
    return false;
  }

  bool is_float32 = SL_STR_EQ(r->type, "float32");
  bool is_int8 = SL_STR_EQ(r->type, "int8");
  bool is_int32 = SL_STR_EQ(r->type, "int32");
  bool is_int64 = SL_STR_EQ(r->type, "int64");
  if (!is_float32 && !is_int8 && !is_int32 && !is_int64) {
    SL_LOG_ERROR("unknown data type '%s'", r->type);
    return false;
  }

  for (int d = 0; d < r->n_dims; ++d) {
    if (r->dim_size[d] == 0) {
      SL_LOG_ERROR("unexpected non-zero size for dimension %d", d);
      return false;
    }
  }

  return true;
}

size_t sl_record_item_size(struct sl_record r[const static 1]) {
  if (SL_STR_EQ(r->type, "float32")) {
    return sizeof(float);
  } else if (SL_STR_EQ(r->type, "int8")) {
    return sizeof(int8_t);
  } else if (SL_STR_EQ(r->type, "int32")) {
    return sizeof(int32_t);
  } else if (SL_STR_EQ(r->type, "int64")) {
    return sizeof(int64_t);
  } else {
    SL_LOG_ERROR("cannot infer size of unknown record data type '%s'", r->type);
    return 0;
  }
}

bool sl_record_read_metadata(struct sl_record record[const static 1],
                             const char path[const static 1],
                             const char name[const static 1]) {
  bool ok = false;

  struct sl_file file = {0};
  {
    char full_path[1024] = {0};
    if (!sl_file_format_path(SL_ARRAY_LEN(full_path),
                             full_path,
                             path,
                             name,
                             ".sl_record_meta")) {
      goto done;
    }
    if (!sl_file_open(&file, full_path, "r")) {
      goto done;
    }
  }
  strcpy(record->path, path);
  strcpy(record->name, name);

  if (EOF == fscanf(file.file,
                    "name: %s\ntype: %s\nlayout: %s\nsize: %zu\ndims: %d\n",
                    record->name,
                    record->type,
                    record->layout,
                    &(record->size),
                    &(record->n_dims)) ||
      record->n_dims == 0) {
    SL_LOG_ERROR("failed reading %s", file.path);
    goto done;
  }

  for (int d = 0; d < record->n_dims; ++d) {
    if (EOF == fscanf(file.file, "dim%*d: %zu\n", record->dim_size + d)) {
      SL_LOG_ERROR("failed reading dimension %d from %s", d, file.path);
      goto done;
    }
  }

  if (!sl_record_validate_metadata(record)) {
    goto done;
  }

  ok = true;
done:
  sl_file_close(&file);
  return ok;
}

bool sl_record_write_metadata(const struct sl_record record[const static 1]) {
  bool ok = false;
  struct sl_file file = {0};

  if (!sl_record_validate_metadata(record)) {
    goto done;
  }

  {
    char full_path[1024] = {0};
    if (!sl_file_format_path(SL_ARRAY_LEN(full_path),
                             full_path,
                             record->path,
                             record->name,
                             ".sl_record_meta")) {
      SL_LOG_ERROR("cannot format metadata path '%s/%s'",
                   record->path,
                   record->name);
      goto done;
    }
    if (!sl_file_open(&file, full_path, "w")) {
      SL_LOG_ERROR("cannot create record metadata file '%s'", file.path);
      goto done;
    }
  }

  if (0 > fprintf(file.file,
                  "name: %s\ntype: %s\nlayout: %s\nsize: %zu\ndims: %d\n",
                  record->name,
                  record->type,
                  record->layout,
                  record->size,
                  record->n_dims)) {
    SL_LOG_ERROR("failed writing to record metadata '%s'", file.path);
    goto done;
  }

  for (int d = 0; d < record->n_dims; ++d) {
    if (0 > fprintf(file.file, "dim%d: %zu\n", d, record->dim_size[d])) {
      SL_LOG_ERROR("failed writing dimension %d to '%s'", d, file.path);
      goto done;
    }
  }

  ok = true;
done:
  sl_file_close(&file);
  return ok;
}

#endif  // SL_RECORD_H_INCLUDED
