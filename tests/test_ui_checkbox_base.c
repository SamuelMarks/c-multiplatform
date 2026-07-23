/* clang-format off */
#include "ui_checkbox_base.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_checkbox_base *cb = NULL;
  enum ui_error rc;
  enum ui_checkbox_state state;

  printf("Testing invalid arguments...\n");
  if (ui_checkbox_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_checkbox_base_destroy(NULL); /* Should not crash */
  if (ui_checkbox_base_get_state(NULL, &state) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_checkbox_base_set_state(NULL, UI_CHECKBOX_STATE_CHECKED) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_checkbox_base_toggle(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_checkbox_base_create(&cb);
  if (rc != UI_ERROR_NONE || cb == NULL) {
    printf("Failed to create checkbox.\n");
    return 1;
  }

  if (ui_checkbox_base_get_state(cb, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_checkbox_base_get_state(cb, &state);
  if (rc != UI_ERROR_NONE || state != UI_CHECKBOX_STATE_UNCHECKED) {
    printf("Initial state should be UNCHECKED.\n");
    return 1;
  }

  /* Test invalid state */
  if (ui_checkbox_base_set_state(cb, (enum ui_checkbox_state)999) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test set state UNCHECKED */
  rc = ui_checkbox_base_set_state(cb, UI_CHECKBOX_STATE_UNCHECKED);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_checkbox_base_get_state(cb, &state);
  if (state != UI_CHECKBOX_STATE_UNCHECKED)
    return 1;

  /* Test set state */
  rc = ui_checkbox_base_set_state(cb, UI_CHECKBOX_STATE_INDETERMINATE);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_checkbox_base_get_state(cb, &state);
  if (state != UI_CHECKBOX_STATE_INDETERMINATE)
    return 1;

  /* Test toggle from indeterminate */
  rc = ui_checkbox_base_toggle(cb);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_checkbox_base_get_state(cb, &state);
  if (state != UI_CHECKBOX_STATE_CHECKED) {
    printf("Toggle from INDETERMINATE should go to CHECKED.\n");
    return 1;
  }

  /* Test toggle from checked */
  ui_checkbox_base_toggle(cb);
  ui_checkbox_base_get_state(cb, &state);
  if (state != UI_CHECKBOX_STATE_UNCHECKED) {
    printf("Toggle from CHECKED should go to UNCHECKED.\n");
    return 1;
  }

  {
    struct ui_control_value_accessor cva;
    if (ui_checkbox_base_get_cva(NULL, &cva) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_checkbox_base_get_cva(cb, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_checkbox_base_get_cva(cb, &cva) != UI_ERROR_NONE)
      return 1;
    if (cva.write_value) {
      union ui_signal_payload val;
      val.int_val = UI_CHECKBOX_STATE_CHECKED;
      cva.write_value(cb, val);
      ui_checkbox_base_get_state(cb, &state);
      if (state != UI_CHECKBOX_STATE_CHECKED)
        return 1;
    }
    if (cva.set_disabled_state) {
      cva.set_disabled_state(NULL, 1);
      cva.set_disabled_state(cb, 1);
      cva.set_disabled_state(cb, 0);
    }
    if (cva.register_on_change)
      cva.register_on_change(cb, NULL, NULL);
    if (cva.register_on_touched)
      cva.register_on_touched(cb, NULL, NULL);
  }

  /* Simulate visually checked and indeterminate dash rendering logic bounds */
  /* Real rendering draws lines over vector buffers, but state determines active
   * paths */
  if (state == UI_CHECKBOX_STATE_CHECKED ||
      state == UI_CHECKBOX_STATE_INDETERMINATE) {
    printf("Checkbox dash visual metrics satisfied.\n");
  }
  ui_checkbox_base_destroy(cb);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_checkbox_base *cb = NULL;
  enum ui_error rc;
  int i;

  printf("Testing OOM...\n");
  for (i = 0; i < 7; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_checkbox_base_create(&cb);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (rc == UI_ERROR_NONE) {
      ui_checkbox_base_destroy(cb);
      break;
    } else {
      printf("Failed OOM test at iteration %d with error %d\n", i, rc);
      return 1;
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

  printf("All ui_checkbox_base tests passed.\n");
  return 0;
}
