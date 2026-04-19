#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stufflib/logging/logging.h>

#ifndef SL_LOGGING_LEVEL_DEFAULT
  #define SL_LOGGING_LEVEL_DEFAULT error
#endif

#define SL_LOGGING_LEVEL_CONCAT_(a, b) a##b
#define SL_LOGGING_LEVEL_CONCAT(a, b)  SL_LOGGING_LEVEL_CONCAT_(a, b)
#define SL_LOGGING_LEVEL_DEFAULT_VALUE \
  SL_LOGGING_LEVEL_CONCAT(SL_LOGGING_LEVEL_, SL_LOGGING_LEVEL_DEFAULT)

int sl_logging_level_runtime = SL_LOGGING_LEVEL_DEFAULT_VALUE;

__attribute__((constructor)) static void sl_logging_init(void) {
  const char* env = getenv("SL_LOGGING_LEVEL");
  if (!env) {
    return;
  }
  if (strcmp(env, "none") == 0) {
    sl_logging_level_runtime = SL_LOGGING_LEVEL_none;
  } else if (strcmp(env, "error") == 0) {
    sl_logging_level_runtime = SL_LOGGING_LEVEL_error;
  } else if (strcmp(env, "info") == 0) {
    sl_logging_level_runtime = SL_LOGGING_LEVEL_info;
  } else if (strcmp(env, "trace") == 0) {
    sl_logging_level_runtime = SL_LOGGING_LEVEL_trace;
  }
}

void sl_logging_write(
    const char* level,
    const char* file,
    unsigned long line,
    const char* fmt,
    ...
) {
  fprintf(stderr, "{\"level\":\"%s\"", level);
  fprintf(stderr, ",\"file\":\"%s\"", file);
  fprintf(stderr, ",\"line\":%lu", line);
  fprintf(stderr, ",\"msg\":\"");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\"}\n");
}
