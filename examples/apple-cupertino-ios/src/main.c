/* clang-format off */
#include "app.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  if (app_init() != 0) {
    return 1;
  }
  app_run();
  app_shutdown();
  return 0;
}