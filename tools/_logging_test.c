#include <stufflib/logging/logging.h>

int main(void) {
  SL_LOG_ERROR("error message");
  SL_LOG_INFO("info message");
  SL_LOG_TRACE("trace message");
  return 0;
}
