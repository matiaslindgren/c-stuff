#ifndef SL_JSON_H_INCLUDED
#define SL_JSON_H_INCLUDED

#include <stddef.h>

bool sl_json_parse(size_t len, const char json[const static len]);

bool sl_json_str(
    const char json[const restrict static 1],
    const char key[const restrict static 1],
    size_t len,
    char out[const restrict static len]
);

bool sl_json_int(
    const char json[const restrict static 1],
    const char key[const restrict static 1],
    long long out[const restrict static 1]
);

#endif  // SL_JSON_H_INCLUDED
