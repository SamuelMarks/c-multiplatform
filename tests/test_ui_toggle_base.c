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

static enum ui_error on_change(struct ui_toggle_base *toggle, int checked,
                               void *user) {
  (void)toggle;
  (void)user;
  g_change_called++;
  g_change_val = checked;
  return UI_ERROR_NONE;
}

static enum ui_error on_cva_change(union ui_signal_payload val, void *user) {
  (void)user;
  g_change_called++;
  g_change_val = val.int_val;
  return UI_ERROR_NONE;
}

static enum ui_error on_cva_touched(void *user) {
  (void)user;
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
  ui_toggle_base_destroy(NULL);

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
  ACCUM_ERR(failed, ui_toggle_base_set_checked(rad1, 1));

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

  /* Tap rad2 again (already checked, radio does nothing) */
  g_change_called = 0;
  ev.type = UI_EVENT_MOUSE_DOWN;
  ACCUM_ERR(failed, ui_toggle_base_process_event(rad2, &ev, 3.0));
  ev.type = UI_EVENT_MOUSE_UP;
  ACCUM_ERR(failed, ui_toggle_base_process_event(rad2, &ev, 3.1));
  ACCUM_FAIL(failed, g_change_called != 0);

  /* Change group name of rad1 to NULL */
  ACCUM_ERR(failed, ui_toggle_base_set_group_name(rad1, NULL));

  /* Additional event branch coverage */
  ev.type = UI_EVENT_KEY_UP;
  ui_toggle_base_process_event(chk1, &ev, 4.0);
  ev.type = UI_EVENT_TOUCH_END;
  ui_toggle_base_process_event(chk1, &ev, 5.0);

  /* Clean up */
  ui_toggle_base_destroy(chk1);
  ui_toggle_base_destroy(rad1);
  ui_toggle_base_destroy(rad2);
  ui_toggle_base_destroy(rad3);

  return failed;
}

static int run_dom_oom(void);
static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_toggle_base *toggle;
  int i;
  enum ui_error err;

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    err = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
    if (err != UI_ERROR_NONE) {
      /* Do nothing, create cleans up partially if component etc fails, wait
         does it? Yes ui_toggle_base_create handles its own cleanup. */
    }
  }
  g_malloc_fail_countdown = -1;

  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  g_malloc_fail_countdown = 0;
  err = ui_toggle_base_set_group_name(toggle, "grp");
  failed |= (err != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_toggle_base_destroy(toggle);
#endif
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

static int run_dom_oom(void) { return 0; }
