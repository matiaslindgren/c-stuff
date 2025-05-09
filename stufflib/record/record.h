#ifndef SL_RECORD_H_INCLUDED
#define SL_RECORD_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stufflib/io/io.h"
#include "stufflib/macros/macros.h"

struct sl_record {
  char layout[8];  // TODO enum
  char type[64];   // TODO enum
  char name[128];
  char path[2048];
  size_t size;
  int n_dims;
  size_t dim_size[8];
};

size_t sl_record_item_size(const struct sl_record r[const static 1]) {
  if (SL_STR_EQ(r->type, "float32")) {
    return sizeof(float);
  } else if (SL_STR_EQ(r->type, "int8")) {
    return sizeof(int8_t);
  } else if (SL_STR_EQ(r->type, "int16")) {
    return sizeof(int16_t);
  } else if (SL_STR_EQ(r->type, "int32")) {
    return sizeof(int32_t);
  } else if (SL_STR_EQ(r->type, "int64")) {
    return sizeof(int64_t);
  } else if (SL_STR_EQ(r->type, "uint8")) {
    return sizeof(uint8_t);
  } else if (SL_STR_EQ(r->type, "uint16")) {
    return sizeof(uint16_t);
  } else if (SL_STR_EQ(r->type, "uint32")) {
    return sizeof(uint32_t);
  } else if (SL_STR_EQ(r->type, "uint64")) {
    return sizeof(uint64_t);
  } else {
    return 0;
  }
}

bool sl_record_validate_metadata(const struct sl_record r[const static 1]) {
  if (r->n_dims <= 0) {
    SL_LOG_ERROR("n_dims must be positive");
    return false;
  }

  bool is_sparse = SL_STR_EQ(r->layout, "sparse");
  bool is_dense = SL_STR_EQ(r->layout, "dense");
  if (!is_sparse && !is_dense) {
    SL_LOG_ERROR("unknown data layout '%s'", r->layout);
    return false;
  }
  if (is_dense && !r->size) {
    SL_LOG_ERROR("dense data layout must have a size");
    return false;
  }

  if (sl_record_item_size(r) == 0) {
    SL_LOG_ERROR("unknown data type '%s'", r->type);
    return false;
  }

  // TODO
#if 0
  for (int d = 0; d < r->n_dims; ++d) {
    if (r->dim_size[d] == 0) {
      SL_LOG_ERROR("unexpected non-zero size for dimension %d", d);
      return false;
    }
  }
#endif

  return true;
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
