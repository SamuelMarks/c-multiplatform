/* clang-format off */
#include "../include/ui_datagrid_base.h"
#include "../include/ui_error.h"
#include "../include/ui_component.h"
#include "../include/ui_computed.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_datagrid_base_lifecycle(void) {
  struct ui_datagrid_base *datagrid = NULL;
  ui_error_t rc;

  rc = ui_datagrid_base_create(&datagrid);
  if (rc != UI_ERROR_NONE || datagrid == NULL)
    return 1;

  if (ui_datagrid_base_destroy(datagrid) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_datagrid_base_get_component(void) {
  struct ui_datagrid_base *datagrid = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = ui_datagrid_base_create(&datagrid);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_datagrid_base_get_component(datagrid, &comp);
  if (rc != UI_ERROR_NONE || comp == NULL)
    return 1;

  if (ui_datagrid_base_destroy(datagrid) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_datagrid_base_resize_column(void) {
  struct ui_datagrid_base *datagrid = NULL;
  ui_error_t rc;

  rc = ui_datagrid_base_create(&datagrid);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_datagrid_base_resize_column(datagrid, 0, 100.0f);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_datagrid_base_destroy(datagrid) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_datagrid_base_move_focus(void) {
  struct ui_datagrid_base *datagrid = NULL;
  ui_error_t rc;

  rc = ui_datagrid_base_create(&datagrid);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_datagrid_base_move_focus(datagrid, 1, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_datagrid_base_destroy(datagrid) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_datagrid_base_bind_data(void) {
  struct ui_datagrid_base *datagrid = NULL;
  struct ui_computed *signal = (struct ui_computed *)0x1234;
  ui_error_t rc;

  rc = ui_datagrid_base_create(&datagrid);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_datagrid_base_bind_data(datagrid, signal);
  if (rc != UI_ERROR_NONE)
    return 1;

  if (ui_datagrid_base_destroy(datagrid) != UI_ERROR_NONE)
    return 1;
  return 0;
}

static int test_datagrid_base_errors(void) {
  struct ui_datagrid_base *datagrid = NULL;
  struct ui_component *comp = NULL;
  struct ui_computed *signal = (struct ui_computed *)0x1234;
  struct ui_datagrid_base *mock_datagrid = (struct ui_datagrid_base *)0x5678;

  if (ui_datagrid_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_datagrid_base_create(&datagrid) != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return 1;
  }
  g_malloc_fail_countdown = -1;

  if (ui_datagrid_base_destroy(NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_datagrid_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datagrid_base_get_component(mock_datagrid, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_datagrid_base_resize_column(NULL, 0, 100.0f) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datagrid_base_move_focus(NULL, 1, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datagrid_base_bind_data(NULL, signal) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  int i;
  for (i = 1; i < 100; ++i) {
    ui_error_t create_rc;
    g_malloc_fail_countdown = i;
    create_rc = ui_datagrid_base_create(&datagrid);
    g_malloc_fail_countdown = -1;
    if (create_rc == UI_ERROR_NONE) {
      if (ui_datagrid_base_destroy(datagrid) != UI_ERROR_NONE)
        return 1;
    }
  }
  return 0;
}

int main(void) {
  int failed = 0;
  printf("Running ui_datagrid_base tests...\n");

  failed |= test_datagrid_base_lifecycle();
  failed |= test_datagrid_base_get_component();
  failed |= test_datagrid_base_resize_column();
  failed |= test_datagrid_base_move_focus();
  failed |= test_datagrid_base_bind_data();
  failed |= test_datagrid_base_errors();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
