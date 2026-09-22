/* clang-format off */
#include "ui_meter_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t test_meter_base(void) {
  struct ui_meter_base *meter = NULL;
  ui_error_t err;
  const char *attr_val = NULL;

  err = ui_meter_base_create(&meter);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_meter_base_set_bounds(meter, 0.0f, 100.0f);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_meter_base_set_value(meter, 75.0f);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_dom_node_get_attribute(meter->base.shadow_root, "aria-valuenow",
                                  &attr_val);
  if (err != UI_ERROR_NONE || strncmp(attr_val, "75.0", 4) != 0) {
    return UI_ERROR_UNKNOWN;
  }

  err = ui_meter_base_set_thresholds(meter, 20.0f, 80.0f, 50.0f);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  err = ui_component_destroy((struct ui_component *)meter);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

static ui_error_t test_edge_cases(void) {
  struct ui_meter_base *meter = NULL;
  ui_error_t rc;
  int i;

  rc = ui_meter_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_meter_base_set_bounds(NULL, 0.0f, 100.0f);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_meter_base_set_value(NULL, 75.0f);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_meter_base_set_thresholds(NULL, 20.0f, 80.0f, 50.0f);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_meter_base_bind_value(NULL, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_meter_base_create(&meter);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_meter_base_bind_value(meter, (struct ui_signal *)1);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_component_destroy((struct ui_component *)meter);
  if (rc != UI_ERROR_NONE)
    return rc;

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_meter_mock_fail;
    extern int g_meter_attr_fail_idx;
    int k;

    /* set_tag_name fails */
    g_meter_mock_fail = 1;
    rc = ui_meter_base_create(&meter);
    if (rc != UI_ERROR_UNKNOWN)
      return UI_ERROR_UNKNOWN;
    g_meter_mock_fail = 0;

    /* create: attr failures 1 to 4 */
    for (k = 1; k <= 4; k++) {
      g_meter_attr_fail_idx = k;
      rc = ui_meter_base_create(&meter);
      if (rc != UI_ERROR_UNKNOWN)
        return UI_ERROR_UNKNOWN;
      g_meter_attr_fail_idx = 0;
    }

    /* update: attr failures 1 to 9 */
    rc = ui_meter_base_create(&meter);
    if (rc != UI_ERROR_NONE)
      return rc;
    for (k = 1; k <= 9; k++) {
      g_meter_attr_fail_idx = k;
      rc = ui_meter_base_set_value(meter, 50.0f);
      if (rc != UI_ERROR_UNKNOWN)
        return UI_ERROR_UNKNOWN;
      g_meter_attr_fail_idx = 0;
    }
    rc = ui_component_destroy((struct ui_component *)meter);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
#endif

  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    if (ui_meter_base_create(&meter) == UI_ERROR_NONE) {
      rc = ui_component_destroy((struct ui_component *)meter);
      if (rc != UI_ERROR_NONE)
        return rc;
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;

  rc = test_meter_base();
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  rc = test_edge_cases();
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  printf("test_ui_meter_base passed\n");
  return 0;
}
