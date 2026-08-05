/* clang-format off */
#include "ui_divider_base.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_divider_base *divider = NULL;
  ui_error_t rc;
  struct ui_component *comp;

  printf("Testing invalid arguments...\n");
  if (ui_divider_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_divider_base_destroy(NULL) != UI_ERROR_NONE)
    return 1; /* Should not crash */
  if (ui_divider_base_set_orientation(
          NULL, UI_DIVIDER_ORIENTATION_HORIZONTAL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_divider_base_set_inset(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_component *tmp_comp;
    if (ui_divider_base_get_component(NULL, &tmp_comp) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_divider_base_create(&divider) != UI_ERROR_NONE)
      return 1;
    if (ui_divider_base_get_component(divider, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_divider_base_destroy(divider) != UI_ERROR_NONE)
      return 1;
    divider = NULL;
    if (ui_divider_base_get_component(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }
  if (ui_divider_base_bind_data(NULL, (struct ui_signal *)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  printf("Testing normal creation and logic...\n");
  rc = ui_divider_base_create(&divider);
  if (rc != UI_ERROR_NONE || divider == NULL)
    return 1;

  if (ui_divider_base_bind_data(divider, (struct ui_signal *)1) !=
      UI_ERROR_NONE)
    return 1;

  if (ui_divider_base_get_component(divider, &comp) != UI_ERROR_NONE ||
      comp == NULL)
    return 1;

  rc =
      ui_divider_base_set_orientation(divider, UI_DIVIDER_ORIENTATION_VERTICAL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_divider_base_set_orientation(divider,
                                       UI_DIVIDER_ORIENTATION_HORIZONTAL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_divider_base_set_inset(divider, 1);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_divider_base_set_inset(divider, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_divider_base_destroy(divider) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int run_oom_tests(void) {
  struct ui_divider_base *divider = NULL;
  ui_error_t rc;
  int i;

  printf("Testing OOM...\n");
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    divider = NULL;
    rc = ui_divider_base_create(&divider);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      /* fine */
    } else if (rc == UI_ERROR_NONE) {
      if (ui_divider_base_destroy(divider) != UI_ERROR_NONE)
        return 1;
      break; /* We hit enough allocations to succeed */
    } else {
      printf("Unexpected error code %d\n", rc);
      return 1;
    }
  }
  g_malloc_fail_countdown = -1;
  /* OOM loop for setter */
  for (i = 1; i < 100; ++i) {
    if (ui_divider_base_create(&divider) == UI_ERROR_NONE) {
      g_malloc_fail_countdown = i;
      ui_divider_base_set_orientation(divider, UI_DIVIDER_ORIENTATION_VERTICAL);
      g_malloc_fail_countdown = -1;

      g_malloc_fail_countdown = i;
      ui_divider_base_set_orientation(divider,
                                      UI_DIVIDER_ORIENTATION_HORIZONTAL);
      g_malloc_fail_countdown = -1;

      g_malloc_fail_countdown = i;
      ui_divider_base_set_inset(divider, 1);
      g_malloc_fail_countdown = -1;

      g_malloc_fail_countdown = i;
      ui_divider_base_set_inset(divider, 0);
      g_malloc_fail_countdown = -1;

      (void)ui_divider_base_destroy(divider);
    }
  }

  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }
  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }
  printf("All ui_divider_base tests passed.\n");
  return 0;
}
