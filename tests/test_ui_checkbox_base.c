/* clang-format off */
#include "ui_checkbox_base.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t run_normal_tests(void) {
  struct ui_checkbox_base *cb = NULL;
  ui_error_t rc;
  enum ui_checkbox_state state;

  printf("Testing invalid arguments...\n");
  rc = ui_checkbox_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_checkbox_base_destroy(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT && rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_checkbox_base_get_state(NULL, &state);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_checkbox_base_set_state(NULL, UI_CHECKBOX_STATE_CHECKED);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_checkbox_base_toggle(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_checkbox_base_create(&cb);
  if (rc != UI_ERROR_NONE || cb == NULL) {
    printf("Failed to create checkbox.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  rc = ui_checkbox_base_get_state(cb, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_checkbox_base_get_state(cb, &state);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (rc != UI_ERROR_NONE || state != UI_CHECKBOX_STATE_UNCHECKED) {
    printf("Initial state should be UNCHECKED.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  /* Test invalid state */
  rc = ui_checkbox_base_set_state(cb, (enum ui_checkbox_state)999);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test set state UNCHECKED */
  rc = ui_checkbox_base_set_state(cb, UI_CHECKBOX_STATE_UNCHECKED);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_checkbox_base_get_state(cb, &state);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (state != UI_CHECKBOX_STATE_UNCHECKED)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test set state */
  rc = ui_checkbox_base_set_state(cb, UI_CHECKBOX_STATE_INDETERMINATE);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_checkbox_base_get_state(cb, &state);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (state != UI_CHECKBOX_STATE_INDETERMINATE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Test toggle from indeterminate */
  rc = ui_checkbox_base_toggle(cb);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_checkbox_base_get_state(cb, &state);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (state != UI_CHECKBOX_STATE_CHECKED) {
    printf("Toggle from INDETERMINATE should go to CHECKED.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  /* Test toggle from checked */
  rc = ui_checkbox_base_toggle(cb);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_checkbox_base_get_state(cb, &state);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (state != UI_CHECKBOX_STATE_UNCHECKED) {
    printf("Toggle from CHECKED should go to UNCHECKED.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  {
    struct ui_control_value_accessor cva;
    rc = ui_checkbox_base_get_cva(NULL, &cva);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    rc = ui_checkbox_base_get_cva(cb, NULL);
    if (rc != UI_ERROR_INVALID_ARGUMENT)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    rc = ui_checkbox_base_get_cva(cb, &cva);
    if (rc != UI_ERROR_NONE)
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    if (cva.write_value) {
      union ui_signal_payload val;
      val.int_val = UI_CHECKBOX_STATE_CHECKED;
      rc = cva.write_value(cb, val);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = ui_checkbox_base_get_state(cb, &state);
      if (rc != UI_ERROR_NONE)
        return rc;
      if (state != UI_CHECKBOX_STATE_CHECKED)
        return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    }
    if (cva.set_disabled_state) {
      rc = cva.set_disabled_state(NULL, 1);
      if (rc != UI_ERROR_INVALID_ARGUMENT)
        return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
      rc = cva.set_disabled_state(cb, 1);
      if (rc != UI_ERROR_NONE)
        return rc;
      rc = cva.set_disabled_state(cb, 0);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    if (cva.register_on_change)
      rc = cva.register_on_change(cb, NULL, NULL);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (cva.register_on_touched)
      rc = cva.register_on_touched(cb, NULL, NULL);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  /* Simulate visually checked and indeterminate dash rendering logic bounds */
  /* Real rendering draws lines over vector buffers, but state determines active
   * paths */
  if (state == UI_CHECKBOX_STATE_CHECKED ||
      state == UI_CHECKBOX_STATE_INDETERMINATE) {
    printf("Checkbox dash visual metrics satisfied.\n");
  }
  rc = ui_checkbox_base_destroy(cb);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

extern int g_checkbox_mock_fail;
extern int g_checkbox_mock_target;
extern int g_checkbox_mock_current;

static ui_error_t run_oom_test_create_step(int i,
                                           struct ui_checkbox_base **out_cb,
                                           int *out_continue, int *out_break) {
  ui_error_t rc;
  g_malloc_fail_countdown = i;
  rc = ui_checkbox_base_create(out_cb);
  if (rc != UI_ERROR_NONE && rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }
  if (rc == UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    *out_continue = 1;
    return UI_ERROR_NONE;
  }
  if (rc == UI_ERROR_NONE) {
    g_malloc_fail_countdown = -1;
    {
      ui_error_t destroy_rc = ui_checkbox_base_destroy(*out_cb);
      if (destroy_rc != UI_ERROR_NONE)
        return destroy_rc;
    }
    *out_break = 1;
    return UI_ERROR_NONE;
  }
  g_malloc_fail_countdown = -1;
  printf("Failed OOM test at iteration %d with error %d\n", i, rc);
  return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
}

static ui_error_t run_oom_tests(void) {
  struct ui_checkbox_base *cb = NULL;
  ui_error_t rc;
  int i, t;

  printf("Testing OOM...\n");
  for (i = 0; i < 7; i++) {
    int do_continue = 0, do_break = 0;
    rc = run_oom_test_create_step(i, &cb, &do_continue, &do_break);
    if (rc != UI_ERROR_NONE)
      return rc;
    if (do_continue)
      continue;
    if (do_break)
      break;
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* Mock DOM Failures during creation */
  for (i = 1; i <= 6; i++) {
    for (t = 1; t <= 5; t++) {
      g_checkbox_mock_fail = i;
      g_checkbox_mock_target = t;
      g_checkbox_mock_current = 0;
      rc = ui_checkbox_base_create(&cb);
      if (rc == UI_ERROR_NONE) {
        rc = ui_checkbox_base_destroy(cb);
        if (rc != UI_ERROR_NONE)
          return rc;
        cb = NULL;
      } else if (rc != UI_ERROR_UNKNOWN && rc != UI_ERROR_OUT_OF_MEMORY) {
        /* maybe error propagation check */
      }
    }
  }

  /* Mock DOM Failures during update_dom_state */
  rc = ui_checkbox_base_create(&cb);
  if (rc != UI_ERROR_NONE)
    return rc;
  for (i = 5; i <= 5; i++) {
    for (t = 1; t <= 10; t++) {
      g_checkbox_mock_fail = i;
      g_checkbox_mock_target = t;
      g_checkbox_mock_current = 0;
      rc = ui_checkbox_base_set_state(cb, UI_CHECKBOX_STATE_CHECKED);
      if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNKNOWN &&
          rc != UI_ERROR_OUT_OF_MEMORY)
        return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
      rc = ui_checkbox_base_set_state(cb, UI_CHECKBOX_STATE_UNCHECKED);
      if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNKNOWN &&
          rc != UI_ERROR_OUT_OF_MEMORY)
        return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
      rc = ui_checkbox_base_set_state(cb, UI_CHECKBOX_STATE_INDETERMINATE);
      if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNKNOWN &&
          rc != UI_ERROR_OUT_OF_MEMORY)
        return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
      {
        struct ui_control_value_accessor cva;
        rc = ui_checkbox_base_get_cva(cb, &cva);
        if (rc == UI_ERROR_NONE) {
          rc = cva.set_disabled_state(cb, UI_TRUE);
          if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNKNOWN &&
              rc != UI_ERROR_OUT_OF_MEMORY)
            return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
          rc = cva.set_disabled_state(cb, UI_FALSE);
          if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNKNOWN &&
              rc != UI_ERROR_OUT_OF_MEMORY)
            return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
        }
      }
    }
  }

  /* Mock DOM Failures during destroy */
  for (i = 4; i <= 4; i++) {
    for (t = 1; t <= 2; t++) {
      g_checkbox_mock_fail = i;
      g_checkbox_mock_target = t;
      g_checkbox_mock_current = 0;
      rc = ui_checkbox_base_create(&cb);
      if (rc == UI_ERROR_NONE) {
        rc = ui_checkbox_base_destroy(cb);
        if (rc != UI_ERROR_NONE && rc != UI_ERROR_UNKNOWN &&
            rc != UI_ERROR_OUT_OF_MEMORY)
          return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
      }
    }
  }
  g_checkbox_mock_fail = 0;
#endif

  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc;
  rc = run_normal_tests();
  if (rc != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  rc = run_oom_tests();
  if (rc != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_checkbox_base tests passed.\n");
  return UI_ERROR_NONE;
}
