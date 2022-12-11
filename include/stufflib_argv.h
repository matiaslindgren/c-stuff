#ifndef _STUFFLIB_ARGV_H_INCLUDED
#define _STUFFLIB_ARGV_H_INCLUDED
#include <stdlib.h>
#include <string.h>

int stufflib_argv_parse_flag(int argc,
                             char* const argv[argc + 1],
                             const char* flag) {
  for (size_t i = 1; i < argc; ++i) {
    if (strcmp(argv[i], flag) == 0) {
      return 1;
    }
  }
  return 0;
}

#endif  // _STUFFLIB_ARGV_H_INCLUDED
