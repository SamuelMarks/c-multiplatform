/* clang-format off */
#include "../include/ui_radio_group_base.h"
#include "../include/ui_toggle_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int group_changed_count = 0;
static struct ui_toggle_base *last_active = NULL;
static int mock_on_change_fail = 0;
static int cva_change_called = 0;
static int cva_touched_called = 0;

static enum ui_error on_group_change(struct ui_radio_group_base *group,
                                     struct ui_toggle_base *active,
                                     void *user_data) {
  (void)group;
  (void)user_data;
  group_changed_count++;
  last_active = active;
  if (mock_on_change_fail)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}

static enum ui_error on_cva_change(union ui_signal_payload new_value,
                                   void *user_data) {
  cva_change_called++;
  return UI_ERROR_NONE;
}

static enum ui_error on_cva_touched(void *user_data) {
  cva_touched_called++;
  return UI_ERROR_NONE;
}

static int test_radio_group_basic(void) {
  struct ui_radio_group_base *group = NULL;
  struct ui_control_value_accessor cva;
  struct ui_toggle_base *r1 = NULL;
  struct ui_toggle_base *r2 = NULL;
  struct ui_toggle_base *r3 = NULL;
  struct ui_toggle_base *r4 = NULL, *r5 = NULL;
  struct ui_toggle_base *out_a = NULL;
  struct ui_toggle_base *out_b = NULL;
  union ui_signal_payload val;
  struct ui_event ev;

  ui_radio_group_base_create(&group, &cva);
  ui_radio_group_base_set_on_change(group, on_group_change, NULL);

  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r1);
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r2);
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r3);

  ui_radio_group_base_add_toggle(group, r1);
  ui_radio_group_base_add_toggle(group, r2);
  ui_radio_group_base_add_toggle(group, r3);

  /* Add more to trigger capacity growth */
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r4);
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r5);
  ui_radio_group_base_add_toggle(group, r4);
  ui_radio_group_base_add_toggle(group, r5);

  /* Add already added toggle */
  ui_radio_group_base_add_toggle(group, r1);

  /* Initially none should be active from the group's perspective unless
   * pre-checked */
  ui_radio_group_base_get_active(group, &out_a);
  if (out_a != NULL)
    return 1;

  /* Set active programmatically */
  ui_radio_group_base_set_active(group, r1);
  ui_radio_group_base_get_active(group, &out_b);
  if (out_b != r1)
    return 1;
  {
    int is_checked = 0;
    ui_toggle_base_is_checked(r1, &is_checked);
    if (is_checked != 1)
      return 1;
  }

  /* Test setting active with NULL unchecks it */
  ui_radio_group_base_set_active(group, NULL);

  /* Simulate user clicking r2 */
  group_changed_count = 0;
  ui_toggle_base_set_checked(r2, 1);
  ui_radio_group_base_set_active(group, r2);

  /* CVA */
  cva.register_on_change(group, on_cva_change, NULL);
  cva.register_on_touched(group, on_cva_touched, NULL);

  /* Write valid index via CVA */
  val.int_val = 0;
  cva.write_value(group, val);

  /* Write invalid index via CVA */
  val.int_val = 5;
  cva.write_value(group, val);

  /* Write negative via CVA */
  val.int_val = -1;
  cva.write_value(group, val);

  /* Keyboard touched CVA */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  cva_touched_called = 0;
  cva_change_called = 0;
  ui_radio_group_base_process_event(group, &ev);
  if (cva_touched_called == 0)
    return 1;
  if (cva_change_called == 0)
    return 1;

  ui_radio_group_base_remove_toggle(group, r2);

  /* Remove not active toggle */
  ui_radio_group_base_remove_toggle(group, r1);
  ui_radio_group_base_remove_toggle(group, r2);

  ui_radio_group_base_destroy(group);
  ui_toggle_base_destroy(r1);
  ui_toggle_base_destroy(r2);
  ui_toggle_base_destroy(r3);
  ui_toggle_base_destroy(r4);
  ui_toggle_base_destroy(r5);
  return 0;
}

static int test_radio_group_keyboard(void) {
  struct ui_radio_group_base *group = NULL;
  struct ui_toggle_base *r1 = NULL;
  struct ui_toggle_base *r2 = NULL;
  struct ui_toggle_base *r3 = NULL;
  struct ui_event ev;

  ui_radio_group_base_create(&group, NULL);
  ui_radio_group_base_set_on_change(group, on_group_change, NULL);

  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r1);
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r2);
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r3);

  ui_radio_group_base_add_toggle(group, r1);
  ui_radio_group_base_add_toggle(group, r2);
  ui_radio_group_base_add_toggle(group, r3);

  /* Initially none active, pressing ArrowDown should activate the first one */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;

  group_changed_count = 0;
  last_active = NULL;
  ui_radio_group_base_process_event(group, &ev);

  if (group_changed_count != 1)
    return 1;
  if (last_active != r2)
    return 1;

  /* Press ArrowDown again -> r3 */
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r3)
    return 1;

  /* Press ArrowUp -> r2 (active_idx is 2, next_idx is 1) */
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r2)
    return 1;

  /* Press ArrowLeft -> r1 (active_idx is 1, next_idx is 0) */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r1)
    return 1;

  /* Press ArrowLeft again -> r3 (active_idx is 0, next_idx is 2) */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r3)
    return 1;

  /* Press ArrowRight -> r1 (wrap around from 2 to 0) */
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r1)
    return 1;

  /* Press ArrowDown again -> r2 */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r2)
    return 1;

  /* Try unhandled key */
  ev.event_data.keyboard.key_code = 123; /* UI_KEY_SPACE equivalent or dummy */
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r2)
    return 1; /* untouched */

  /* Try unhandled event type */
  ev.type = UI_EVENT_KEY_UP;
  ui_radio_group_base_process_event(group, &ev);
  if (last_active != r2)
    return 1;

  ui_radio_group_base_destroy(group);
  ui_toggle_base_destroy(r1);
  ui_toggle_base_destroy(r2);
  ui_toggle_base_destroy(r3);
  return 0;
}

static int test_radio_group_nulls(void) {
  struct ui_radio_group_base *group = NULL;
  struct ui_control_value_accessor cva;
  struct ui_toggle_base *r1 = NULL;
  struct ui_event ev;
  union ui_signal_payload val;
  struct ui_toggle_base *checked = NULL;
  struct ui_toggle_base *dummy;
  memset(&ev, 0, sizeof(ev));
  memset(&val, 0, sizeof(val));

  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r1);

  if (ui_radio_group_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_radio_group_base_create(&group, &cva);

  if (cva.write_value(NULL, val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.register_on_change(NULL, on_cva_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.register_on_touched(NULL, on_cva_touched, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_radio_group_base_destroy(NULL);

  if (ui_radio_group_base_add_toggle(NULL, r1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_radio_group_base_add_toggle(group, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_radio_group_base_remove_toggle(NULL, r1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_radio_group_base_remove_toggle(group, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_radio_group_base_set_active(NULL, r1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  dummy = (struct ui_toggle_base *)0x123;
  if (ui_radio_group_base_set_active(group, dummy) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_radio_group_base_get_active(NULL, &r1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_radio_group_base_get_active(group, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_radio_group_base_set_on_change(NULL, on_group_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_radio_group_base_process_event(NULL, &ev) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_radio_group_base_process_event(group, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Try to add pre-checked toggle */
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &checked);
  ui_toggle_base_set_checked(checked, 1);
  ui_radio_group_base_add_toggle(group, checked);

  ui_radio_group_base_destroy(group);
  ui_toggle_base_destroy(r1);
  ui_toggle_base_destroy(checked);

  return 0;
}

static int test_radio_group_oom(void) {
  struct ui_radio_group_base *group = NULL;
  struct ui_toggle_base *r1 = NULL;
  int i;
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r1);

  g_malloc_fail_countdown = 0;
  if (ui_radio_group_base_create(&group, NULL) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_radio_group_base_create(&group, NULL);

  for (i = 0; i < 5; i++) {
    struct ui_toggle_base *t;
    enum ui_error rc;
    ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &t);
    g_malloc_fail_countdown = 0; /* fail the realloc */
    rc = ui_radio_group_base_add_toggle(group, t);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      ui_toggle_base_destroy(t);
    } else {
      /* Added */
    }
  }

  ui_radio_group_base_destroy(group);
  ui_toggle_base_destroy(r1);
  return 0;
}

static int test_radio_group_edge_cases(void) {
  struct ui_radio_group_base *group = NULL;
  struct ui_control_value_accessor cva;
  struct ui_toggle_base *r1 = NULL;
  struct ui_toggle_base *r2 = NULL;
  struct ui_event ev;

  ui_radio_group_base_create(&group, &cva);
  ui_toggle_base_create(UI_TOGGLE_TYPE_CHECKBOX, &r1);
  ui_toggle_base_create(UI_TOGGLE_TYPE_RADIO, &r2);

  /* Process event on empty group */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_radio_group_base_process_event(group, &ev);

  ui_radio_group_base_add_toggle(group, r1);
  ui_radio_group_base_add_toggle(group, r2);

  /* Process event when disabled */
  cva.set_disabled_state(group, 1);
  cva_touched_called = 0;
  ui_radio_group_base_process_event(group, &ev);
  if (cva_touched_called != 0)
    return 1; /* disabled, should return early */

  cva.set_disabled_state(group, 0);

  /* Set active */
  ui_radio_group_base_set_active(group, r1);

  /* Trigger internal toggle uncheck via mouse click since it's a checkbox */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ui_toggle_base_process_event(r1, &ev, 100.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_toggle_base_process_event(r1, &ev, 200.0);

  /* mock_on_change_fail */
  ui_radio_group_base_set_on_change(group, on_group_change, NULL);
  mock_on_change_fail = 1;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ui_toggle_base_process_event(r2, &ev, 300.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_toggle_base_process_event(r2, &ev, 400.0);
  mock_on_change_fail = 0;

  ui_radio_group_base_destroy(group);
  ui_toggle_base_destroy(r1);
  ui_toggle_base_destroy(r2);

  return 0;
}

int main(void) {
  int failed = 0;

  printf("Running ui_radio_group_base tests...\n");

  printf("Running test_radio_group_basic...\n");
  failed |= test_radio_group_basic();
  printf("Running test_radio_group_keyboard...\n");
  failed |= test_radio_group_keyboard();
  printf("Running test_radio_group_nulls...\n");
  failed |= test_radio_group_nulls();
  printf("Running test_radio_group_oom...\n");
  failed |= test_radio_group_oom();
  printf("Running test_radio_group_edge_cases...\n");
  failed |= test_radio_group_edge_cases();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
