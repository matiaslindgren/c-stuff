#ifndef _STUFFLIB_MACROS_H_INCLUDED
#define _STUFFLIB_MACROS_H_INCLUDED

#define STUFFLIB_MIN(x, y) ((x) < (y) ? (x) : (y))
#define STUFFLIB_MAX(x, y) ((x) < (y) ? (y) : (x))

#define STUFFLIB_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

#define STUFFLIB_ONES(n) ((1 << ((n)*CHAR_BIT)) - 1)

#define STUFFLIB_PRINT_ERROR(msg, ...)                       \
  do {                                                       \
    fprintf(stderr, "error (%s@L%d): ", __FILE__, __LINE__); \
    fprintf(stderr, (msg), ##__VA_ARGS__);                   \
    fprintf(stderr, "\n");                                   \
  } while (0)

#endif  // _STUFFLIB_MACROS_H_INCLUDED
