/* clang-format off */
#include "ui_bottom_app_bar_base.h"
#include "ui_fab_base.h"
#include "ui_component.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static void test_bottom_app_bar_edge_cases(void) {
  struct ui_bottom_app_bar_base *bar = NULL;
  struct ui_component *comp = NULL;
  struct ui_signal *signal = (struct ui_signal *)0x123;
  int i;
  ui_error_t rc;

  /* Invalid arguments */
  assert(ui_bottom_app_bar_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  {
    ui_error_t rc_cleanup = ui_bottom_app_bar_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  assert(ui_bottom_app_bar_base_get_component(NULL, &comp) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_app_bar_base_set_fab(NULL, NULL,
                                        UI_BOTTOM_APP_BAR_FAB_CENTER) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_app_bar_base_bind_active_index(NULL, signal) ==
         UI_ERROR_INVALID_ARGUMENT);

  rc = ui_bottom_app_bar_base_create(&bar);
  assert(rc == UI_ERROR_NONE);

  assert(ui_bottom_app_bar_base_get_component(bar, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_bottom_app_bar_base_bind_active_index(bar, signal) ==
         UI_ERROR_NONE);

  /* Fab alignment END */
  struct ui_fab_base *fab = NULL;
  assert(ui_fab_base_create(&fab) == UI_ERROR_NONE);

  assert(ui_bottom_app_bar_base_set_fab(bar, fab, UI_BOTTOM_APP_BAR_FAB_END) ==
         UI_ERROR_NONE);

  /* NULL fab to test cutout none */
  assert(ui_bottom_app_bar_base_set_fab(
             bar, NULL, UI_BOTTOM_APP_BAR_FAB_CENTER) == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_bottom_app_bar_base_destroy(bar);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_fab_base_destroy(fab);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* OOM loop */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    struct ui_bottom_app_bar_base *test_bar = NULL;
    rc = ui_bottom_app_bar_base_create(&test_bar);
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_bottom_app_bar_base_destroy(test_bar);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      break;
    } else {
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
    }
  }
  g_malloc_fail_countdown = -1;
  printf("test_bottom_app_bar_edge_cases passed\n");
}

int main(void) {

#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_bottom_app_bar_coverage(void);
  run_bottom_app_bar_coverage();
#endif

  struct ui_bottom_app_bar_base *bar = NULL;
  struct ui_fab_base *fab = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_bottom_app_bar_base_create(&bar);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create bottom app bar\n");
    return 1;
  }

  if (ui_bottom_app_bar_base_get_component(bar, &comp) != UI_ERROR_NONE ||
      !comp) {
    printf("Failed to get component\n");
    return 1;
  }

  rc = ui_fab_base_create(&fab);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create fab\n");
    return 1;
  }

  rc = ui_bottom_app_bar_base_set_fab(bar, fab, UI_BOTTOM_APP_BAR_FAB_CENTER);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set fab\n");
    return 1;
  }

  {
    ui_error_t rc_cleanup = ui_bottom_app_bar_base_destroy(bar);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_fab_base_destroy(fab);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  test_bottom_app_bar_edge_cases();

  printf("test_ui_bottom_app_bar_base passed\n");
  return 0;
}
