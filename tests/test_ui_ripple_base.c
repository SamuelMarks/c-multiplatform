/* clang-format off */
#include "ui_ripple_base.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  ui_error_t rc;
  struct ui_ripple_config config;
  struct ui_ripple_state state;
  int test_failed = 0;

  rc = ui_ripple_config_init(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from init\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_ripple_config_init(&config);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: config_init failed\n");
    test_failed = 1;
    goto cleanup;
  }

  config.max_radius = 100.0f;

  rc = ui_ripple_start(NULL, 0.0f, 0.0f, &state);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from start\n");
    test_failed = 1;
    goto cleanup;
  }
  rc = ui_ripple_start(&config, 0.0f, 0.0f, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from start (null state)\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_ripple_start(&config, 10.0f, 20.0f, &state);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: start failed\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_ripple_update(NULL, 10.0f, &state);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from update\n");
    test_failed = 1;
    goto cleanup;
  }
  rc = ui_ripple_update(&config, 10.0f, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    printf("Test failed: expected INVALID_ARGUMENT from update (null state)\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_ripple_update(&config, 150.0f, &state);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: update failed\n");
    test_failed = 1;
    goto cleanup;
  }

  if (state.radius <= 0.0f || state.opacity >= 0.1f) {
    printf("Test failed: update logic incorrect\n");
    test_failed = 1;
    goto cleanup;
  }

  rc = ui_ripple_update(&config, 200.0f, &state);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: final update failed\n");
    test_failed = 1;
    goto cleanup;
  }
  if (state.active != 0) {
    printf("Test failed: ripple should be inactive\n");
    test_failed = 1;
    goto cleanup;
  }

  /* Calling update on inactive ripple */
  rc = ui_ripple_update(&config, 10.0f, &state);
  if (rc != UI_ERROR_NONE) {
    printf("Test failed: inactive update failed\n");
    test_failed = 1;
    goto cleanup;
  }

cleanup:
  if (test_failed) {
    return 1;
  }

  printf("All ripple tests passed.\n");
  return 0;
}
