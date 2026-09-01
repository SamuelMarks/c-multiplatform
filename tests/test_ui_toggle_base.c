/* clang-format off */
#include "ui_toggle_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_control_value_accessor.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static int g_change_called = 0;
static int g_change_val = -1;
static int g_touched_called = 0;

static int g_mock_cb_fail = 0;

static ui_error_t on_change(struct ui_toggle_base *toggle, int checked,
                            void *user) {
  (void)toggle;
  (void)user;
  if (g_mock_cb_fail == 1)
    return UI_ERROR_UNKNOWN;
  g_change_called++;
  g_change_val = checked;
  return UI_ERROR_NONE;
}

static ui_error_t on_cva_change(union ui_signal_payload val, void *user) {
  (void)user;
  if (g_mock_cb_fail == 2)
    return UI_ERROR_UNKNOWN;
  g_change_called++;
  g_change_val = val.int_val;
  return UI_ERROR_NONE;
}

static ui_error_t on_cva_touched(void *user) {
  (void)user;
  if (g_mock_cb_fail == 3)
    return UI_ERROR_UNKNOWN;
  g_touched_called++;
  return UI_ERROR_NONE;
}

static int test_normal(void) {
  struct ui_toggle_base *chk1 = NULL;
  struct ui_toggle_base *rad1 = NULL;
  struct ui_toggle_base *rad2 = NULL;
  struct ui_toggle_base *rad3 = NULL;
  struct ui_component *comp;
  struct ui_control_value_accessor cva;
  int is_checked;
  struct ui_event ev;
  int failed = 0;

  /* Null checks */
  failed |= (ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  failed |= (ui_toggle_base_set_disabled(NULL, 1) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toggle_base_is_checked(NULL, &is_checked) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toggle_base_set_checked(NULL, 1) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_toggle_base_set_group_name(NULL, "g1") != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toggle_base_set_on_change(NULL, on_change, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toggle_base_process_event(NULL, &ev, 0.0) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_toggle_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toggle_base_get_cva(NULL, &cva) != UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &chk1));
  failed |=
      (ui_toggle_base_is_checked(chk1, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_toggle_base_get_component(chk1, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toggle_base_get_cva(chk1, NULL) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toggle_base_process_event(chk1, NULL, 0.0) !=
             UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_toggle_base_get_component(chk1, &comp));
  ACCUM_ERR(failed, ui_toggle_base_get_cva(chk1, &cva));

  /* Disable/Enable */
  ACCUM_ERR(failed, ui_toggle_base_set_disabled(chk1, 1));
  ACCUM_ERR(failed, ui_toggle_base_set_disabled(chk1, 0));

  /* CVA methods */
  failed |= (cva.register_on_change(NULL, on_cva_change, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (cva.register_on_touched(NULL, on_cva_touched, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  /* Set cva_on_change to NULL then trigger check */
  cva.register_on_change(chk1, NULL, NULL);
  cva.write_value(chk1, (union ui_signal_payload){.bool_val = 1});
  cva.write_value(chk1, (union ui_signal_payload){.bool_val = 0});
  cva.register_on_touched(chk1, NULL, NULL);

  ACCUM_ERR(failed, cva.register_on_change(chk1, on_cva_change, NULL));
  ACCUM_ERR(failed, cva.register_on_touched(chk1, on_cva_touched, NULL));
  ACCUM_ERR(failed, cva.set_disabled_state(chk1, 1));

  /* Event processing while disabled */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ACCUM_ERR(failed, ui_toggle_base_process_event(chk1, &ev, 0.0));

  ev.type = UI_EVENT_MOUSE_UP;
  ACCUM_ERR(failed, ui_toggle_base_process_event(chk1, &ev, 0.1));
  ACCUM_ERR(failed, ui_toggle_base_is_checked(chk1, &is_checked));
  ACCUM_FAIL(failed, is_checked != 0);

  ACCUM_ERR(failed, cva.set_disabled_state(chk1, 0));

  /* Setup native on_change too (they both fire) */
  ACCUM_ERR(failed, ui_toggle_base_set_on_change(chk1, on_change, NULL));

  /* Event processing - tap -> changes checkbox state */
  g_change_called = 0;
  g_touched_called = 0;
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 0;
  ev.event_data.mouse.y = 0;
  ev.event_data.mouse.button = 0; /* UI_MOUSE_BUTTON_LEFT */
  ACCUM_ERR(failed, ui_toggle_base_process_event(chk1, &ev, 1.0));
  ev.type = UI_EVENT_MOUSE_UP;
  ACCUM_ERR(failed, ui_toggle_base_process_event(chk1, &ev, 1.1));

  ACCUM_FAIL(failed, g_change_called != 2); /* native + CVA */
  ACCUM_FAIL(failed, g_touched_called != 1);
  ACCUM_ERR(failed, ui_toggle_base_is_checked(chk1, &is_checked));
  ACCUM_FAIL(failed, is_checked != 1);

  /* CVA write value */
  union ui_signal_payload val;
  val.int_val = 0;
  ACCUM_ERR(failed, cva.write_value(chk1, val));
  ACCUM_ERR(failed, ui_toggle_base_is_checked(chk1, &is_checked));
  ACCUM_FAIL(failed, is_checked != 0);

  /* --- Radio Button logic --- */
  ACCUM_ERR(failed, ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &rad1));
  ACCUM_ERR(failed, ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &rad2));
  ACCUM_ERR(failed, ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &rad3));

  ACCUM_ERR(failed, ui_toggle_base_set_group_name(rad1, "grp"));
  ACCUM_ERR(failed, ui_toggle_base_set_group_name(rad2, "grp"));
  ACCUM_ERR(failed, ui_toggle_base_set_group_name(rad3, "grp_other"));

  ACCUM_ERR(failed, ui_toggle_base_set_on_change(rad1, on_change, NULL));
  ACCUM_ERR(failed, ui_toggle_base_set_on_change(rad2, on_change, NULL));

  /* Check rad1 */
  /* Remove on_change temporarily to hit branch */
  ui_toggle_base_set_on_change(rad1, NULL, NULL);
  ACCUM_ERR(failed, ui_toggle_base_set_checked(rad1, 1));
  ui_toggle_base_set_on_change(rad1, on_change, NULL);

  /* Process event on rad2 to tap it */
  g_change_called = 0;
  ev.type = UI_EVENT_MOUSE_DOWN;
  ACCUM_ERR(failed, ui_toggle_base_process_event(rad2, &ev, 2.0));
  ev.type = UI_EVENT_MOUSE_UP;
  ACCUM_ERR(failed, ui_toggle_base_process_event(rad2, &ev, 2.1));

  /* rad2 checked = 1 (on_change fired). rad1 checked = 0 (on_change fired). */
  ACCUM_ERR(failed, ui_toggle_base_is_checked(rad1, &is_checked));
  ACCUM_FAIL(failed, is_checked != 0);
  ACCUM_ERR(failed, ui_toggle_base_is_checked(rad2, &is_checked));
  ACCUM_FAIL(failed, is_checked != 1);

  /* Check rad3 (has different group) */
  ACCUM_ERR(failed, ui_toggle_base_set_checked(rad3, 1));

  /* Create rad4 without a group, and check it */
  struct ui_toggle_base *rad4;
  ACCUM_ERR(failed, ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &rad4));
  ACCUM_ERR(failed, ui_toggle_base_set_checked(rad4, 1));
  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(rad4);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Tap rad2 again (already checked, radio does nothing) */
  g_change_called = 0;
  ev.type = UI_EVENT_MOUSE_DOWN;
  ACCUM_ERR(failed, ui_toggle_base_process_event(rad2, &ev, 3.0));
  ev.type = UI_EVENT_MOUSE_UP;
  ACCUM_ERR(failed, ui_toggle_base_process_event(rad2, &ev, 3.1));
  ACCUM_FAIL(failed, g_change_called != 0);

  /* Test cb failures on checkbox */
  g_mock_cb_fail = 1;
  ui_toggle_base_set_checked(chk1, 0);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toggle_base_process_event(chk1, &ev, 5.0);
  ev.type = UI_EVENT_MOUSE_UP;
  failed |= (ui_toggle_base_process_event(chk1, &ev, 5.1) != UI_ERROR_UNKNOWN);

  g_mock_cb_fail = 2;
  ui_toggle_base_set_checked(chk1, 0);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toggle_base_process_event(chk1, &ev, 5.2);
  ev.type = UI_EVENT_MOUSE_UP;
  failed |= (ui_toggle_base_process_event(chk1, &ev, 5.3) != UI_ERROR_UNKNOWN);

  g_mock_cb_fail = 3;
  ui_toggle_base_set_checked(chk1, 0);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toggle_base_process_event(chk1, &ev, 5.4);
  ev.type = UI_EVENT_MOUSE_UP;
  failed |= (ui_toggle_base_process_event(chk1, &ev, 5.5) != UI_ERROR_UNKNOWN);
  g_mock_cb_fail = 0;

  /* Test cb failures on radio during exclusion updates */
  g_mock_cb_fail = 1;
  ui_toggle_base_set_checked(rad1, 0);
  ui_toggle_base_set_checked(rad2, 1);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toggle_base_process_event(rad1, &ev, 6.0);
  ev.type = UI_EVENT_MOUSE_UP;
  failed |= (ui_toggle_base_process_event(rad1, &ev, 6.1) != UI_ERROR_UNKNOWN);
  g_mock_cb_fail = 0;

  /* OOM branches for DOM updates */
  g_malloc_fail_countdown = 0; /* 0 means fail immediately at the mock point */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toggle_base_process_event(chk1, &ev, 7.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_toggle_base_process_event(
      chk1, &ev,
      7.1); /* Returns OOM internally but caught in process_event or updates */
  g_malloc_fail_countdown = -1;

  /* Check enforce_radio_exclusion returning error if DOM update fails */
  ui_toggle_base_set_checked(rad1, 0);
  ui_toggle_base_set_checked(rad2, 1);

  /* The mock block in enforce_radio_exclusion literally says `if
   * (g_malloc_fail_countdown == 0)`. But `ui_toggle_base_set_checked` does
   * `update_dom_state(rad1)`. `update_dom_state` calls
   * `ui_dom_node_set_attribute` which calls `malloc`. Try countdown values
   * exactly 1 and 2 to hit the explicit mock block in enforce_radio_exclusion.
   */
  g_malloc_fail_countdown = 1;
  failed |= (ui_toggle_base_set_checked(rad1, 1) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_toggle_base_set_checked(rad1, 0);
  ui_toggle_base_set_checked(rad2, 1);
  g_malloc_fail_countdown = 2;
  failed |= (ui_toggle_base_set_checked(rad1, 1) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_toggle_base_set_checked(rad1, 0);
  ui_toggle_base_set_checked(rad2, 1);
  g_malloc_fail_countdown = 3;
  failed |= (ui_toggle_base_set_checked(rad1, 1) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_toggle_base_set_checked(rad1, 0);
  ui_toggle_base_set_checked(rad2, 1);
  g_malloc_fail_countdown = 4;
  failed |= (ui_toggle_base_set_checked(rad1, 1) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_toggle_base_set_checked(rad1, 0);
  ui_toggle_base_set_checked(rad2, 1);
  g_malloc_fail_countdown = 5;
  failed |= (ui_toggle_base_set_checked(rad1, 1) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  /* Check DOM state updates independently */
  g_malloc_fail_countdown = 0;
  failed |= (ui_toggle_base_set_checked(rad1, 0) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  /* Test radio joining group when already checked */
  ui_toggle_base_set_checked(rad1, 0);
  struct ui_toggle_base *rad5;
  ACCUM_ERR(failed, ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &rad5));
  ui_toggle_base_set_checked(rad5, 1);
  ui_toggle_base_set_group_name(rad5, "g1"); /* Re-evaluates exclusion */
  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(rad5);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Change group name of rad1 to NULL */
  ACCUM_ERR(failed, ui_toggle_base_set_group_name(rad1, NULL));

  /* Additional event branch coverage */
  ev.type = UI_EVENT_KEY_UP;
  ui_toggle_base_process_event(chk1, &ev, 4.0);
  ev.type = UI_EVENT_TOUCH_END;
  ui_toggle_base_process_event(chk1, &ev, 5.0);

  /* Also test the FALSE branch for toggle->checked in
     ui_toggle_base_set_group_name: We need to call
     ui_toggle_base_set_group_name on an UNCHECKED toggle! */
  struct ui_toggle_base *chk_setgrp = NULL;
  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &chk_setgrp);
  ui_toggle_base_set_checked(chk_setgrp, 0);
  ui_toggle_base_set_group_name(chk_setgrp, "group");
  ui_toggle_base_destroy(chk_setgrp);

  /* Also test FALSE branch on toggle->type == UI_TOGGLE_TYPE_RADIO in
     ui_toggle_base_set_group_name: We need to call
     ui_toggle_base_set_group_name on a CHECKBOX that is CHECKED. */
  struct ui_toggle_base *chk_setgrp2 = NULL;
  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &chk_setgrp2);
  ui_toggle_base_set_checked(chk_setgrp2, 1);
  ui_toggle_base_set_group_name(chk_setgrp2, "chk_grp");
  ui_toggle_base_destroy(chk_setgrp2);

  /* Test enforce_radio_exclusion when an unchecked radio has NO on_change
     callback: We need to uncheck a radio that has NO on_change. */
  struct ui_toggle_base *r_ex_1 = NULL;
  struct ui_toggle_base *r_ex_2 = NULL;
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r_ex_1);
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r_ex_2);
  ui_toggle_base_set_group_name(r_ex_1, "ex_grp");
  ui_toggle_base_set_group_name(r_ex_2, "ex_grp");
  ui_toggle_base_set_checked(r_ex_1, 1);
  ui_toggle_base_set_checked(r_ex_2, 1);
  ui_toggle_base_destroy(r_ex_1);
  ui_toggle_base_destroy(r_ex_2);

  /* Also test the FALSE branch for toggle->checked inside
     ui_toggle_base_process_event: We need to tap an already-checked CHECKBOX.
   */
  struct ui_toggle_base *chk_uncheck_ev = NULL;
  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &chk_uncheck_ev);
  ui_toggle_base_set_checked(chk_uncheck_ev, 1);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toggle_base_process_event(chk_uncheck_ev, &ev, 12.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_toggle_base_process_event(chk_uncheck_ev, &ev, 12.1);
  ui_toggle_base_destroy(chk_uncheck_ev);

  /* Try to hit branch 0 on toggle->on_change inside process_event */
  struct ui_toggle_base *r_no_oc_ev = NULL;
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r_no_oc_ev);
  ui_toggle_base_set_on_change(r_no_oc_ev, NULL, NULL);
  ui_toggle_base_set_checked(r_no_oc_ev, 0);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toggle_base_process_event(r_no_oc_ev, &ev, 11.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_toggle_base_process_event(r_no_oc_ev, &ev, 11.1);
  ui_toggle_base_destroy(r_no_oc_ev);

  /* Also test the FALSE branch for toggle->on_change in
   * ui_toggle_base_set_checked */
  struct ui_toggle_base *chk_no_oc = NULL;
  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &chk_no_oc);
  ui_toggle_base_set_on_change(chk_no_oc, NULL, NULL);
  ui_toggle_base_set_checked(chk_no_oc, 1);
  ui_toggle_base_set_checked(chk_no_oc, 0);
  ui_toggle_base_destroy(chk_no_oc);

  /* Clean up */
  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(chk1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(rad1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(rad2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(rad3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return failed;
}

static int run_dom_oom(void);
static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_toggle_base *toggle;
  int i;
  ui_error_t err;

  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    err = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_toggle_base_destroy(toggle);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    err = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &toggle);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_toggle_base_destroy(toggle);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }
  g_malloc_fail_countdown = -1;

  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    err = ui_toggle_base_set_group_name(toggle, "grp");
    if (err == UI_ERROR_NONE) {
      ui_toggle_base_set_group_name(toggle, NULL);
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_toggle_base_set_disabled(toggle, 1);
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_toggle_base_set_disabled(toggle, 0);
  }
  g_malloc_fail_countdown = -1;

  ui_toggle_base_set_group_name(toggle, "grp");
  g_malloc_fail_countdown = 0;
  ui_toggle_base_set_group_name(toggle, NULL);
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_toggle_base_set_checked(toggle, 1);
  }
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_toggle_base_destroy(toggle);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
#endif

  ui_toggle_base_set_disabled(NULL, 1);
  ui_toggle_base_set_checked(NULL, 1);
  ui_toggle_base_set_group_name(NULL, "grp");
  ui_toggle_base_set_on_change(NULL, NULL, NULL);

  {
    struct ui_toggle_base *r1, *r2, *r3;
    ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r1);
    ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r2);
    ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r3);

    {
      ui_error_t rc_cleanup = ui_toggle_base_destroy(r2);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_toggle_base_destroy(r3);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_toggle_base_destroy(r1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_normal();
  failed |= test_oom();
  failed |= run_dom_oom();
  if (!failed) {
    printf("All ui_toggle_base tests passed.\n");
  }
  return failed;
}

static int run_dom_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_toggle_base *toggle;
  int i;
  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_toggle_base_set_checked(toggle, 1);
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_toggle_base_set_disabled(toggle, 1);
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    ui_toggle_base_set_disabled(toggle, 0);
  }
  g_malloc_fail_countdown = -1;

  ui_toggle_base_destroy(toggle);
#endif
  return failed;
}
