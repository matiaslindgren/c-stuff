#ifndef SL_LOGGING_H_INCLUDED
#define SL_LOGGING_H_INCLUDED

#include <stdio.h>

#define SL_LOGGING_LEVEL_none  0
#define SL_LOGGING_LEVEL_error 1
#define SL_LOGGING_LEVEL_info  2
#define SL_LOGGING_LEVEL_trace 3

extern int sl_logging_level_runtime;

#ifndef SL_LOGGING_MAX_LENGTH
  #define SL_LOGGING_MAX_LENGTH 4'096
#endif

__attribute__((format(printf, 5, 6))) void sl_logging_writef(
    FILE* stream,
    const char* level,
    const char* file,
    unsigned long line,
    const char* fmt,
    ...
);

#define SL_LOGF(stream, level, ...)                                       \
  do {                                                                    \
    if (sl_logging_level_runtime >= SL_LOGGING_LEVEL_##level) {           \
      sl_logging_writef(stream, #level, __FILE__, __LINE__, __VA_ARGS__); \
    }                                                                     \
  } while (false)

#define SL_LOGF_TRACE(stream, ...) SL_LOGF(stream, trace, __VA_ARGS__)
#define SL_LOGF_INFO(stream, ...)  SL_LOGF(stream, info, __VA_ARGS__)
#define SL_LOGF_ERROR(stream, ...) SL_LOGF(stream, error, __VA_ARGS__)

#define SL_LOG(level, ...) SL_LOGF(stderr, level, __VA_ARGS__)
#define SL_LOG_TRACE(...)  SL_LOGF_TRACE(stderr, __VA_ARGS__)
#define SL_LOG_INFO(...)   SL_LOGF_INFO(stderr, __VA_ARGS__)
#define SL_LOG_ERROR(...)  SL_LOGF_ERROR(stderr, __VA_ARGS__)

#endif  // SL_LOGGING_H_INCLUDED
