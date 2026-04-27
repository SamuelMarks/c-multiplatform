/* clang-format off */
#include "app.h"
#include <stdio.h>
/* clang-format on */

int main(int argc, char **argv) {
  int rc = 0;
  (void)argc;
  (void)argv;

  printf("Starting OAuth2 Simple Example...\n");
  if (app_init() != 0) {
    fprintf(stderr, "Failed to initialize application.\n");
    return 1;
  }

  app_run();
  app_shutdown();

  printf("Example exited cleanly.\n");
  if (rc != 0) {
    return rc;
  }
  return rc;
}
