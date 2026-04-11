#include <stdlib.h>
#include <stufflib/testing/testing.h>
#include <unistd.h>

bool sl_terminal_use_colors(void) {
  char* no_color = getenv("NO_COLOR");
  if (no_color && no_color[0] != '\0') {
    return false;
  }
  return isatty(STDOUT_FILENO);
}
