/* clang-format off */
#include "ui_meter_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t test_meter_base(void) {
  struct ui_meter_base *meter;
  ui_error_t err;
  const char *attr_val;

  err = ui_meter_base_create(&meter);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_meter_base_create failed\n");
    exit(1);
    return UI_ERROR_NONE;
  }

  err = ui_meter_base_set_bounds(meter, 0.0f, 100.0f);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_meter_base_set_bounds failed\n");
    exit(1);
  }

  err = ui_meter_base_set_value(meter, 75.0f);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_meter_base_set_value failed\n");
    exit(1);
  }

  err = ui_dom_node_get_attribute(meter->base.shadow_root, "aria-valuenow",
                                  &attr_val);
  if (err != UI_ERROR_NONE || strncmp(attr_val, "75.0", 4) != 0) {
    fprintf(stderr,
            "ui_meter_base_set_value did not set attribute correctly\n");
    exit(1);
  }

  err = ui_meter_base_set_thresholds(meter, 20.0f, 80.0f, 50.0f);
  if (err != UI_ERROR_NONE) {
    fprintf(stderr, "ui_meter_base_set_thresholds failed\n");
    exit(1);
  }

  {
    ui_error_t rc_cleanup = ui_component_destroy((struct ui_component *)meter);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return UI_ERROR_NONE;
}

static int test_edge_cases(void) {
  struct ui_meter_base *meter;
  int i;

  ui_meter_base_create(NULL);
  ui_meter_base_set_bounds(NULL, 0.0f, 100.0f);
  ui_meter_base_set_value(NULL, 75.0f);
  ui_meter_base_set_thresholds(NULL, 20.0f, 80.0f, 50.0f);
  ui_meter_base_bind_value(NULL, NULL);

  ui_meter_base_create(&meter);
  ui_meter_base_bind_value(meter, (struct ui_signal *)1);
  {
    ui_error_t rc_cleanup = ui_component_destroy((struct ui_component *)meter);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    if (ui_meter_base_create(&meter) == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup =
            ui_component_destroy((struct ui_component *)meter);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }
  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  test_meter_base();
  test_edge_cases();
  printf("test_ui_meter_base passed\n");
  return 0;
}
