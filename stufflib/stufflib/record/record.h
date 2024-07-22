#ifndef SL_RECORD_H_INCLUDED
#define SL_RECORD_H_INCLUDED

struct sl_record {
  char layout[8];  // TODO enum
  char type[64];   // TODO enum
  char name[128];
  char path[2048];
  size_t size;
  int n_dims;
  size_t dim_size[8];
};

size_t sl_record_item_size(const struct sl_record r[const static 1]);

bool sl_record_validate_metadata(const struct sl_record r[const static 1]);

bool sl_record_read_metadata(struct sl_record record[const static 1],
                             const char path[const static 1],
                             const char name[const static 1]);

bool sl_record_write_metadata(const struct sl_record record[const static 1]);

#endif  // SL_RECORD_H_INCLUDED
