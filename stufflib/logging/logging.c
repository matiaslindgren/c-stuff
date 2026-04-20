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

__attribute__((format(printf, 5, 0))) static void sl_logging_vwritef(
    FILE* stream,
    const char* level,
    const char* file,
    unsigned long line,
    const char* fmt,
    va_list args
) {
  // TODO json writer
  fprintf(stream, "{\"level\":\"%s\"", level);
  fprintf(stream, ",\"file\":\"%s\"", file);
  fprintf(stream, ",\"line\":%lu", line);
  fprintf(stream, ",\"msg\":\"");
  static char msg[SL_LOGGING_MAX_LENGTH];
  vsnprintf(msg, sizeof(msg), fmt, args);
  for (const char* p = msg; *p; ++p) {
    switch (*p) {
      case '"': {
        fprintf(stream, "\\\"");
        break;
      }
      case '\\': {
        fprintf(stream, "\\\\");
        break;
      }
      case '\n': {
        fprintf(stream, "\\n");
        break;
      }
      case '\r': {
        fprintf(stream, "\\r");
        break;
      }
      case '\t': {
        fprintf(stream, "\\t");
        break;
      }
      case '\b': {
        fprintf(stream, "\\b");
        break;
      }
      case '\f': {
        fprintf(stream, "\\f");
        break;
      }
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x07:
      case 0x0b:
      case 0x0e:
      case 0x0f:
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
      case 0x18:
      case 0x19:
      case 0x1a:
      case 0x1b:
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f: {
        fprintf(stream, "\\u%04x", (unsigned char)*p);
        break;
      }
      default: {
        fputc(*p, stream);
        break;
      }
    }
  }
  fprintf(stream, "\"}\n");
}

void sl_logging_writef(
    FILE* stream,
    const char* level,
    const char* file,
    unsigned long line,
    const char* fmt,
    ...
) {
  va_list args;
  va_start(args, fmt);
  sl_logging_vwritef(stream, level, file, line, fmt, args);
  va_end(args);
}
