#ifndef SL_STRING_H_INCLUDED
#define SL_STRING_H_INCLUDED
#include <stdio.h>

#include "../span/span.h"

struct sl_string {
  size_t length;
  struct sl_span utf8_data;
};

void sl_string_destroy(struct sl_string string[static 1]);

struct sl_string sl_string_from_utf8(struct sl_span utf8_data[const static 1]);

struct sl_span sl_string_view_utf8_data(struct sl_string str[const static 1]);

bool sl_string_is_ascii(const struct sl_string str[const static 1]);

void sl_string_copy_ascii(char dst[const static 1],
                          const struct sl_string str[const static 1]);

struct sl_string sl_string_concat(struct sl_string str1[const static 1],
                                  struct sl_string str2[const static 1]);

void sl_string_extend(struct sl_string str1[const static 1],
                      struct sl_string str2[const static 1]);

struct sl_string sl_string_slice(struct sl_string str[const static 1],
                                 size_t begin,
                                 size_t end);

bool sl_string_fprint(FILE stream[const static 1],
                      struct sl_string str[const static 1]);

#endif  // SL_STRING_H_INCLUDED
