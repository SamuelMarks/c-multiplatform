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

  /* Set the Modality to ASYNC_SINGLE_SINGLE */
  (void)cmp_modality_async_single_init(&mod);
  /* Here we would setup a HTTP server socket using c-abstract-http via
     cmp_net_*, but for example purposes we just print that it would run the
     async loop. */
  printf("Starting UI in ASYNC_SINGLE_SINGLE modality...\n");

  /* cmp_run_loop(&mod); */
  /* ... mock exit ... */
  if (rc != 0) {
    return rc;
  }
  return rc;
}
