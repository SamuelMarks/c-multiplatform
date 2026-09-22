/* clang-format off */
#include "greatest.h"
#include "ui_toggle_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_component.h"
#include "ui_control_value_accessor.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_toggle_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  enum ui_toggle_type type;
  int checked;
  int disabled;
  char *group_name;
  ui_toggle_on_change_t on_change;
  void *user_data;
  ui_error_t (*cva_on_change)(union ui_signal_payload, void *);
  void *cva_on_change_user_data;
  ui_error_t (*cva_on_touched)(void *);
  void *cva_on_touched_user_data;
  struct ui_toggle_base *next;
  struct ui_toggle_base *prev;
  struct ui_signal *checked_signal;
};

extern int g_malloc_fail_countdown;
extern int g_toggle_mock_fail;
extern int g_toggle_mock_remove_attr_fail_target;
extern int g_toggle_mock_set_attr_fail_target;

static int g_change_called = 0;
static int g_change_val = -1;
static int g_touched_called = 0;
static int g_mock_cb_fail = 0;

static ui_error_t on_change(struct ui_toggle_base *toggle, int checked,
                            void *user) {
  int unused_c = checked;
  void *unused_u = user;
  struct ui_toggle_base *unused_t = toggle;
  toggle = unused_t;
  checked = unused_c;
  user = unused_u;
  if (g_mock_cb_fail == 1)
    return UI_ERROR_UNKNOWN;
  g_change_called++;
  g_change_val = checked;
  return UI_ERROR_NONE;
}

static ui_error_t on_cva_change(union ui_signal_payload val, void *user) {
  void *unused_u = user;
  user = unused_u;
  if (g_mock_cb_fail == 2)
    return UI_ERROR_UNKNOWN;
  g_change_called++;
  g_change_val = val.int_val;
  return UI_ERROR_NONE;
}

static ui_error_t on_cva_touched(void *user) {
  void *unused_u = user;
  user = unused_u;
  if (g_mock_cb_fail == 3)
    return UI_ERROR_UNKNOWN;
  g_touched_called++;
  return UI_ERROR_NONE;
}

TEST test_toggle_invalid_args(void) {
  struct ui_toggle_base *chk = NULL;
  struct ui_component *comp = NULL;
  struct ui_control_value_accessor cva;
  int is_checked = 0;
  struct ui_event ev;
  ui_error_t rc;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_toggle_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toggle_base_set_disabled(NULL, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toggle_base_is_checked(NULL, &is_checked));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toggle_base_set_checked(NULL, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toggle_base_set_group_name(NULL, "g1"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toggle_base_set_on_change(NULL, on_change, NULL));
  memset(&ev, 0, sizeof(ev));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toggle_base_process_event(NULL, &ev, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toggle_base_get_component(NULL, &comp));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toggle_base_get_cva(NULL, &cva));
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &chk);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toggle_base_is_checked(chk, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toggle_base_get_component(chk, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toggle_base_get_cva(chk, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toggle_base_process_event(chk, NULL, 0.0));

  rc = ui_toggle_base_destroy(chk);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_toggle_lifecycle_and_features(void) {
  struct ui_toggle_base *chk = NULL;
  struct ui_toggle_base *r1 = NULL;
  struct ui_toggle_base *r2 = NULL;
  struct ui_toggle_base *r3 = NULL;
  struct ui_component *comp = NULL;
  struct ui_control_value_accessor cva;
  union ui_signal_payload payload;
  int is_checked = 0;
  struct ui_event ev;
  ui_error_t rc;

  /* Checkbox operations */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &chk);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toggle_base_get_component(chk, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = ui_toggle_base_set_on_change(chk, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Initially unchecked */
  rc = ui_toggle_base_is_checked(chk, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_checked);

  /* Check and uncheck */
  rc = ui_toggle_base_set_checked(chk, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_is_checked(chk, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_checked);

  rc = ui_toggle_base_set_checked(chk, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_is_checked(chk, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_checked);

  /* Disable and enable */
  rc = ui_toggle_base_set_disabled(chk, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_disabled(chk, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Group name on checkbox (covers set and clear) */
  rc = ui_toggle_base_set_group_name(chk, "chkgroup");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(chk, "chkgroup2");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(chk, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA operations */
  rc = ui_toggle_base_get_cva(chk, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA NULL checks */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_change(NULL, on_cva_change, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_touched(NULL, on_cva_touched, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.set_disabled_state(NULL, 1));
  memset(&payload, 0, sizeof(payload));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.write_value(NULL, payload));

  rc = cva.register_on_change(chk, on_cva_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_touched(chk, on_cva_touched, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  payload.int_val = 1;
  rc = cva.write_value(chk, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cva.set_disabled_state(chk, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.set_disabled_state(chk, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Process events while disabled: should do nothing */
  rc = ui_toggle_base_set_disabled(chk, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(chk, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_disabled(chk, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Process TAP event on checkbox toggles it */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(chk, &ev, 10.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  g_change_called = 0;
  rc = ui_toggle_base_process_event(chk, &ev, 15.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toggle_base_destroy(chk);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Radio button exclusion and group management */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toggle_base_set_group_name(r1, "g1");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(r2, "g1");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(r3, "g1");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toggle_base_set_on_change(r1, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_on_change(r2, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Check r1 */
  rc = ui_toggle_base_set_checked(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_is_checked(r1, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_checked);

  /* Check r2 -> unchecks r1 */
  rc = ui_toggle_base_set_checked(r2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_is_checked(r1, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_checked);
  rc = ui_toggle_base_is_checked(r2, &is_checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_checked);

  /* TAP on unchecked radio checks it */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(r1, &ev, 20.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toggle_base_process_event(r1, &ev, 25.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* TAP on already checked radio does nothing */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(r1, &ev, 30.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toggle_base_process_event(r1, &ev, 35.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Check r3 while setting group name to g1 */
  rc = ui_toggle_base_set_group_name(r3, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(r3, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(r3, "g1");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy in order: r2 (middle), r3 (head), r1 (tail/only) to test all unlink
   * branches */
  rc = ui_toggle_base_destroy(r2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_destroy(r3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_destroy(r1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_toggle_error_branches(void) {
  struct ui_toggle_base *toggle = NULL;
  struct ui_toggle_base *r1 = NULL;
  struct ui_toggle_base *r2 = NULL;
  struct ui_control_value_accessor cva;
  struct ui_event ev;
  ui_error_t rc;
  int i;

  /* 1. Gesture event processing mock failure */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  g_toggle_mock_fail = 1;
  rc = ui_toggle_base_process_event(toggle, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_fail = 0;

  /* 2. Mock remove_attribute failure (checked, disabled, name) */
  rc = ui_toggle_base_set_checked(toggle, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_toggle_mock_remove_attr_fail_target = 1;
  rc = ui_toggle_base_set_checked(toggle, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_remove_attr_fail_target = 0;

  /* Mock set_attribute failure on line 208 (checked = 1, call 2) */
  g_toggle_mock_set_attr_fail_target = 2;
  rc = ui_toggle_base_set_checked(toggle, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_set_attr_fail_target = 0;

  /* mock remove_attribute target 2 (called twice to cover non-failing branch)
   */
  g_toggle_mock_remove_attr_fail_target = 2;
  rc = ui_toggle_base_set_disabled(toggle, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_disabled(toggle, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_disabled(toggle, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_disabled(toggle, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_remove_attr_fail_target = 0;

  rc = ui_toggle_base_set_disabled(toggle, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_toggle_mock_remove_attr_fail_target = 1;
  rc = ui_toggle_base_set_disabled(toggle, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_remove_attr_fail_target = 0;
  rc = ui_toggle_base_set_disabled(toggle, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toggle_base_set_group_name(toggle, "g1");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_toggle_mock_remove_attr_fail_target = 1;
  rc = ui_toggle_base_set_group_name(toggle, NULL);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_remove_attr_fail_target = 0;

  /* 3. Mock set_attribute failures in set_group_name and set_disabled */
  g_toggle_mock_set_attr_fail_target = 1;
  rc = ui_toggle_base_set_group_name(toggle, "g2");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_set_attr_fail_target = 0;

  for (i = 1; i <= 3; i++) {
    g_toggle_mock_set_attr_fail_target = i;
    rc = ui_toggle_base_set_disabled(toggle, 1);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_toggle_mock_set_attr_fail_target = 0;
  }
  for (i = 1; i <= 2; i++) {
    g_toggle_mock_set_attr_fail_target = i;
    rc = ui_toggle_base_set_disabled(toggle, 0);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_toggle_mock_set_attr_fail_target = 0;
  }

  /* 4. Tap on toggle without callbacks (covers false branches for callbacks) */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(toggle, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toggle_base_process_event(toggle, &ev, 5.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 5. Callback failures */
  rc = ui_toggle_base_set_on_change(toggle, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_get_cva(toggle, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_change(toggle, on_cva_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_touched(toggle, on_cva_touched, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_mock_cb_fail = 1; /* on_change fails */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(toggle, &ev, 10.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toggle_base_process_event(toggle, &ev, 15.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_cb_fail = 0;

  g_mock_cb_fail = 2; /* cva on_change fails */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(toggle, &ev, 20.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toggle_base_process_event(toggle, &ev, 25.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_cb_fail = 0;

  g_mock_cb_fail = 3; /* cva on_touched fails */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(toggle, &ev, 30.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toggle_base_process_event(toggle, &ev, 35.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_cb_fail = 0;

  rc = ui_toggle_base_destroy(toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 6. Creation cleanup failures */
  for (i = 2; i <= 4; i++) {
    g_toggle_mock_fail = i;
    g_toggle_mock_set_attr_fail_target = 1;
    rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_toggle_mock_set_attr_fail_target = 0;
    g_toggle_mock_fail = 0;
  }

  /* 7. Radio creation set_attr fail (targets radio branches in create) */
  for (i = 1; i <= 4; i++) {
    g_toggle_mock_set_attr_fail_target = i;
    rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &toggle);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_toggle_mock_set_attr_fail_target = 0;
  }

  /* 8. Destroy mock failures */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_toggle_mock_fail = 3; /* gesture destroy fails */
  rc = ui_toggle_base_destroy(toggle);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_fail = 0;

  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_toggle_mock_fail = 4; /* component destroy fails */
  rc = ui_toggle_base_destroy(toggle);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toggle_mock_fail = 0;

  /* 9. Enforce radio exclusion error branches */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toggle_base_set_group_name(r1, "ex_grp");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(r2, "ex_grp");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* r1 has failing on_change during exclusion */
  rc = ui_toggle_base_set_on_change(r1, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_mock_cb_fail = 1;
  rc = ui_toggle_base_set_checked(r2, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_cb_fail = 0;

  /* Exclusion fail on tap (line 613) */
  rc = ui_toggle_base_set_checked(r2, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_mock_cb_fail = 1;
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(r2, &ev, 10.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toggle_base_process_event(r2, &ev, 15.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_cb_fail = 0;

  /* Exclusion fail on set_group_name while checked (line 542) */
  rc = ui_toggle_base_set_on_change(r2, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(r2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(r1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_mock_cb_fail = 1;
  rc = ui_toggle_base_set_group_name(r1, "ex_grp");
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_mock_cb_fail = 0;

  /* Radio with NULL group_name and different group_name in registry */
  {
    struct ui_toggle_base *r_null = NULL;
    struct ui_toggle_base *r_diff = NULL;
    rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r_null);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r_diff);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_toggle_base_set_group_name(r_diff, "diff_grp");
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_toggle_base_set_checked(r2, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_toggle_base_destroy(r_null);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_toggle_base_destroy(r_diff);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Radio exclusion when other radio has NULL on_change callback */
  rc = ui_toggle_base_set_on_change(r1, NULL, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(r2, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Exclusion OOM test (hit line 247) */
  for (i = 0; i < 10; i++) {
    r1->checked = 1;
    r2->checked = 0;
    g_malloc_fail_countdown = i;
    ui_toggle_base_set_checked(r2, 1);
    g_malloc_fail_countdown = -1;
  }

  /* Checked checkbox set_group_name (covers should_enforce false for non-radio)
   */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_checked(toggle, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(toggle, "chk_grp");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_destroy(toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Checked radio set_group_name NULL (covers should_enforce false for
   * group_name NULL) */
  rc = ui_toggle_base_set_checked(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_set_group_name(r1, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy with NULL gesture_recognizer and NULL component */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_gesture_recognizer_destroy(toggle->gesture_recognizer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  toggle->gesture_recognizer = NULL;
  rc = ui_component_destroy(toggle->component);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  toggle->component = NULL;
  rc = ui_toggle_base_destroy(toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toggle_base_destroy(r1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toggle_base_destroy(r2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_toggle_oom(void) {
  struct ui_toggle_base *toggle = NULL;
  struct ui_event ev;
  ui_error_t rc;
  int i;

  /* Creation OOM loop */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    toggle = NULL;
    rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_toggle_base_destroy(toggle);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(toggle == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* set_group_name OOM */
  rc = ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = ui_toggle_base_set_group_name(toggle, "oom_group");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Event process tap OOM */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_toggle_base_process_event(toggle, &ev, 10.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  g_malloc_fail_countdown = 0;
  rc = ui_toggle_base_process_event(toggle, &ev, 15.0);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  rc = ui_toggle_base_destroy(toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_toggle_base_suite) {
  RUN_TEST(test_toggle_invalid_args);
  RUN_TEST(test_toggle_lifecycle_and_features);
  RUN_TEST(test_toggle_error_branches);
  RUN_TEST(test_toggle_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_toggle_base_suite);
  GREATEST_MAIN_END();
}
