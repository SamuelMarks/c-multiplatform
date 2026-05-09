/* clang-format off */
#include "app.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  int rc = 0;
  if (app_init() != 0) {
    return 1;
  }
  app_run();
  app_shutdown();
  if (rc != 0) {
    return rc;
  }
  return rc;
}
