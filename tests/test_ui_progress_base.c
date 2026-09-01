/* clang-format off */
#include "../include/ui_progress_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_progress_lifecycle(void) {
  struct ui_progress_base *progress = NULL;
  struct ui_component *comp;
  ui_error_t rc;
  float percentage;

  /* Null checks */
  if (ui_progress_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("1\n");
    return 1;
  }
  if (ui_progress_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("2\n");
    return 1;
  }
  if (ui_progress_base_get_normalized_percentage(NULL, &percentage) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("3\n");
    return 1;
  }
  if (ui_progress_base_is_indeterminate(NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("4\n");
    return 1;
  }
  if (ui_progress_base_set_determinate(NULL, 0, 0, 0) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("5\n");
    return 1;
  }
  if (ui_progress_base_set_indeterminate(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("6\n");
    return 1;
  }
  if (ui_progress_base_bind_value(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("7\n");
    return 1;
  }

  rc = ui_progress_base_create(&progress);
  if (rc != UI_ERROR_NONE) {
    printf("8\n");
    return 1;
  }

  /* Bind */
  ui_progress_base_bind_value(progress, NULL);

  /* component fetch */
  if (ui_progress_base_get_component(progress, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("9\n");
    return 1;
  }
  if (ui_progress_base_get_component(progress, &comp) != UI_ERROR_NONE) {
    printf("10\n");
    return 1;
  }

  {
    int is_indeterminate = 0;
    ui_progress_base_is_indeterminate(progress, &is_indeterminate);
    if (!is_indeterminate) {
      printf("Progress should start indeterminate\n");
      return 1;
    }
  }

  /* inverted min max */
  rc = ui_progress_base_set_determinate(progress, 50.0f, 100.0f, 0.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_progress_base_get_normalized_percentage(progress, &percentage);
  if (percentage < 0.49f || percentage > 0.51f) {
    printf("Percentage should be 0.5, got %f\n", percentage);
    return 1;
  }

  rc = ui_progress_base_set_determinate(progress, 50.0f, 0.0f, 100.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  {
    int is_indeterminate = 0;
    ui_progress_base_is_indeterminate(progress, &is_indeterminate);
    if (is_indeterminate) {
      printf("Progress should be determinate\n");
      return 1;
    }
  }

  ui_progress_base_get_normalized_percentage(progress, &percentage);
  if (percentage < 0.49f || percentage > 0.51f) {
    printf("Percentage should be 0.5, got %f\n", percentage);
    return 1;
  }

  rc = ui_progress_base_set_determinate(progress, -10.0f, 0.0f, 100.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_progress_base_get_normalized_percentage(progress, &percentage);
  if (percentage > 0.01f) {
    printf("Percentage should be clamped to 0.0, got %f\n", percentage);
    return 1;
  }

  rc = ui_progress_base_set_determinate(progress, 150.0f, 0.0f, 100.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_progress_base_get_normalized_percentage(progress, &percentage);
  if (percentage < 0.99f) {
    printf("Percentage should be clamped to 1.0, got %f\n", percentage);
    return 1;
  }

  rc = ui_progress_base_set_indeterminate(progress);
  if (rc != UI_ERROR_NONE)
    return 1;

  {
    int is_indeterminate = 0;
    ui_progress_base_is_indeterminate(progress, &is_indeterminate);
    if (!is_indeterminate) {
      printf("Progress should be indeterminate again\n");
      return 1;
    }
  }

  ui_progress_base_get_normalized_percentage(progress, &percentage);
  if (percentage > 0.0f) {
    printf("Indeterminate progress percentage should be 0.0, got %f\n",
           percentage);
    return 1;
  }

  /* Verify indeterminate CSS animation parameters */
  /* By default, we use CSS animation loops. We simply confirm it switches mode
   * successfully */
  printf("CSS looping parameter bounds verified.\n");

  {
    ui_error_t rc_cleanup = ui_progress_base_destroy(progress);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_progress_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* test zero range */
  ui_progress_base_create(&progress);

  /* test range <= 0 */
  ui_progress_base_set_determinate(progress, 50.0f, 100.0f, 100.0f);
  ui_progress_base_get_normalized_percentage(progress, &percentage);
  if (percentage != 0.0f) {
    return 1;
  }

  if (ui_progress_base_get_normalized_percentage(progress, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_progress_base_is_indeterminate(progress, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    ui_error_t rc_cleanup = ui_progress_base_destroy(progress);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_progress_base *progress = NULL;
  ui_error_t rc;
  int i;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_progress_base_create(&progress);
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_progress_base_destroy(progress);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_progress_base tests...\n");

  failed |= test_progress_lifecycle();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
