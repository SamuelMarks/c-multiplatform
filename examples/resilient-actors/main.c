/* clang-format off */
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

int main(int argc, char **argv) {
  int rc = 0;
  cmp_app_config_t config;
  int err;
  cmp_modality_t mod;

  (void)argc;
  (void)argv;

  config.modality = 0;
  config.min_threads = 1;
  config.max_threads = 4;

  err = cmp_app_init(&config);
  if (err != CMP_SUCCESS) {
    printf("Failed to initialize app\n");
    return 1;
  }

  cmp_modality_multiprocess_init(&mod);

  printf("Starting UI in MULTIPROCESS_ACTOR modality...\n");
  /* Background worker processes spawn.
     If a worker crashes, the supervisor revives it via IPC Actor messages. */

  if (rc != 0) {
    return rc;
  }
  return rc;
}
