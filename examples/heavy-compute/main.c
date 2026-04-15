/* clang-format off */
#include "cmp.h"
#include <stdio.h>
/* clang-format on */

int main(int argc, char **argv) {
  cmp_app_config_t config;
  int err;

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

  /* Set the Modality to SYNC_MULTI */
  cmp_modality_t mod;
  cmp_modality_sync_multi_init(&mod, 4); /* 4 background threads */

  printf("Starting UI in SYNC_MULTI modality with 4 worker threads...\n");
  /* Offload fractal generation to background thread pool while UI renders
   * smooth */

  return 0;
}
