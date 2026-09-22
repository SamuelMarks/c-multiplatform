/* clang-format off */
#include "greatest.h"
#include "ui_wheel_picker_base.h"
#include "ui_gesture.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_error.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_wheel_mock_fail;

static int change_count = 0;
static int last_selected_index = -1;
static int touched_count = 0;
static int cva_change_count = 0;
static int cva_last_selected_index = -1;
static int g_cb_fail_on_change = 0;
static int g_cb_fail_cva_change = 0;
static int g_cb_fail_touched = 0;

static ui_error_t on_change(struct ui_wheel_picker_base *picker,
                            int selected_index, void *user_data) {
  void *unused_u = user_data;
  struct ui_wheel_picker_base *unused_p = picker;
  picker = unused_p;
  user_data = unused_u;
  if (g_cb_fail_on_change)
    return UI_ERROR_UNKNOWN;
  change_count++;
  last_selected_index = selected_index;
  return UI_ERROR_NONE;
}

static ui_error_t cva_on_change(union ui_signal_payload new_value,
                                void *user_data) {
  void *unused_u = user_data;
  user_data = unused_u;
  if (g_cb_fail_cva_change)
    return UI_ERROR_UNKNOWN;
  cva_change_count++;
  cva_last_selected_index = new_value.int_val;
  return UI_ERROR_NONE;
}

static ui_error_t cva_on_touched(void *user_data) {
  void *unused_u = user_data;
  user_data = unused_u;
  if (g_cb_fail_touched)
    return UI_ERROR_UNKNOWN;
  touched_count++;
  return UI_ERROR_NONE;
}

struct ui_wheel_picker_base {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;
  char **items;
  int item_count;
  int is_looping;
  int selected_index;
  float scroll_offset;
  float velocity;
  int is_dragging;
  ui_wheel_picker_on_change_t on_change;
  void *on_change_user_data;
  ui_error_t (*cva_on_change)(union ui_signal_payload, void *);
  void *cva_on_change_user_data;
  ui_error_t (*cva_on_touched)(void *);
  void *cva_on_touched_user_data;
  int is_disabled;
};

TEST test_wheel_invalid_args(void) {
  struct ui_wheel_picker_base *picker = NULL;
  struct ui_component *comp = NULL;
  int idx = 0;
  struct ui_event ev;
  ui_error_t rc;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_wheel_picker_base_create(NULL, NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_wheel_picker_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_set_items(NULL, NULL, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_set_looping(NULL, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_set_selected_index(NULL, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_get_selected_index(NULL, &idx));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_set_on_change(NULL, NULL, NULL));
  memset(&ev, 0, sizeof(ev));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_process_event(NULL, &ev, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_on_tick(NULL, 16.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_get_component(NULL, &comp));

  rc = ui_wheel_picker_base_create(&picker, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_set_items(picker, NULL, 5));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_get_selected_index(picker, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_process_event(picker, NULL, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_wheel_picker_base_get_component(picker, NULL));

  rc = ui_wheel_picker_base_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_wheel_lifecycle_and_scrolling(void) {
  struct ui_wheel_picker_base *picker = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *comp = NULL;
  const char *items[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
  union ui_signal_payload sp;
  struct ui_event ev;
  int idx = -1;
  ui_error_t rc;

  rc = ui_wheel_picker_base_create(&picker, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_wheel_picker_base_get_component(picker, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = cva.register_on_change(picker, cva_on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.register_on_touched(picker, cva_on_touched, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_wheel_picker_base_set_on_change(picker, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set items */
  rc = ui_wheel_picker_base_set_items(picker, items, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Non-looping selection */
  rc = ui_wheel_picker_base_set_looping(picker, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_wheel_picker_base_set_selected_index(picker, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_get_selected_index(picker, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  /* Same index no-op */
  rc = ui_wheel_picker_base_set_selected_index(picker, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Clamp negative index */
  rc = ui_wheel_picker_base_set_selected_index(picker, -5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_get_selected_index(picker, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  /* Clamp excessive index */
  rc = ui_wheel_picker_base_set_selected_index(picker, 100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_get_selected_index(picker, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, idx);

  /* Looping selection */
  rc = ui_wheel_picker_base_set_looping(picker, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_wheel_picker_base_set_selected_index(picker, 6); /* 6 % 5 = 1 */
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_get_selected_index(picker, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = ui_wheel_picker_base_set_selected_index(picker, -1); /* -1 -> 4 */
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_get_selected_index(picker, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, idx);

  /* Keyboard events */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_get_selected_index(picker, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, idx);

  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_get_selected_index(picker, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, idx);

  /* CVA write value and disable */
  sp.int_val = 0;
  rc = cva.write_value(picker, sp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, picker->selected_index);

  rc = cva.set_disabled_state(picker, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva.set_disabled_state(picker, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick physics: spring snapping and final snap */
  picker->velocity = 100.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  picker->velocity = 0.05f; /* below threshold -> snap */
  picker->scroll_offset =
      42.0f; /* near index 1 (40.0f), diff = -2.0f (> 0.5) */
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  picker->scroll_offset = 40.2f; /* diff = -0.2f (<= 0.5) -> final snap */
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(40.0f, picker->scroll_offset);
  ASSERT_EQ(1, picker->selected_index);

  /* Non-looping offset bounding */
  rc = ui_wheel_picker_base_set_looping(picker, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->scroll_offset = -10.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, picker->scroll_offset);

  picker->scroll_offset = 500.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(160.0f, picker->scroll_offset); /* 4 * 40.0f */

  /* CVA null checks */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.write_value(NULL, sp));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_change(NULL, cva_on_change, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_touched(NULL, cva_on_touched, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.set_disabled_state(NULL, 1));

  /* Gesture PAN and SWIPE events */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;

  g_wheel_mock_fail = 7; /* PAN began */
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, picker->is_dragging);

  g_wheel_mock_fail = 8; /* PAN changed */
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_wheel_mock_fail = 9; /* SWIPE ended */
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, picker->is_dragging);

  g_wheel_mock_fail = 10; /* PAN ended */
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_wheel_mock_fail = 0;

  /* Normal gesture process event call (covers line 102) */
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick while is_dragging is 1 (line 653 false branch) */
  picker->is_dragging = 1;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->is_dragging = 0;

  /* set_items when selected_index >= count and count > 0 (line 509 true branch)
   */
  picker->selected_index = 10;
  rc = ui_wheel_picker_base_set_items(picker, items, 3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, picker->selected_index);

  /* Set items overwrite with count 0 */
  rc = ui_wheel_picker_base_set_items(picker, NULL, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, picker->item_count);

  /* Operations without CVA callbacks (covers lines 207, 217) */
  picker->cva_on_change = NULL;
  picker->cva_on_touched = NULL;
  picker->on_change = NULL;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Re-add items for snapping tests without callbacks */
  rc = ui_wheel_picker_base_set_items(picker, items, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->on_change = NULL;
  picker->cva_on_change = NULL;
  picker->selected_index = 0;
  picker->scroll_offset = 40.1f; /* snap to 1 */
  picker->velocity = 0.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, picker->selected_index);

  /* Tick when target_index == selected_index (line 672 false) */
  picker->scroll_offset = 40.1f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Looping tick with negative target_index (lines 688, 691, 699) */
  rc = ui_wheel_picker_base_set_looping(picker, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->selected_index = 0;
  picker->scroll_offset = -39.9f;
  picker->velocity = 0.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, picker->selected_index);
  rc = ui_wheel_picker_base_set_looping(picker, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Non-looping target_index >= item_count clamp in on_tick (line 653) */
  picker->scroll_offset = 300.1f;
  picker->velocity = 0.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, picker->selected_index);

  /* Non-looping target_index < 0 clamp in on_tick (line 650) */
  picker->scroll_offset = -40.1f;
  picker->velocity = 0.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, picker->selected_index);

  /* Non-looping offset in valid range (line 720 false) */
  picker->scroll_offset = 40.0f;
  picker->velocity = 0.0f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* set_selected_index with on_change == NULL (line 553 false) */
  rc = ui_wheel_picker_base_set_selected_index(picker, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* update_dom_state when component is NULL (line 239) */
  {
    struct ui_component *saved_comp = picker->component;
    picker->component = NULL;
    picker->selected_index = 0;
    rc = ui_wheel_picker_base_set_selected_index(picker, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    picker->component = saved_comp;
  }

  /* Destroy with shadow_root == NULL, component == NULL, gesture_recognizer ==
   * NULL */
  rc = ui_dom_node_destroy(picker->component->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->component->shadow_root = NULL;
  rc = ui_wheel_picker_base_set_selected_index(picker, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_gesture_recognizer_destroy(picker->gesture_recognizer);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->gesture_recognizer = NULL;
  rc = ui_component_destroy(picker->component);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->component = NULL;

  rc = ui_wheel_picker_base_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Operations on picker with item_count == 0 to cover remaining branches */
  {
    struct ui_wheel_picker_base *p_zero = NULL;
    rc = ui_wheel_picker_base_create(&p_zero, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* Unhandled key code falls through (line 615) */
    memset(&ev, 0, sizeof(ev));
    ev.type = UI_EVENT_KEY_DOWN;
    ev.event_data.keyboard.key_code = UI_KEY_LEFT;
    rc = ui_wheel_picker_base_process_event(p_zero, &ev, 0.0);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* set_selected_index when item_count == 0 non-looping (branch 509) */
    rc = ui_wheel_picker_base_set_selected_index(p_zero, 5);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT_EQ(0, p_zero->selected_index);

    /* set_selected_index when item_count == 0 looping (branches 539, 540) */
    rc = ui_wheel_picker_base_set_looping(p_zero, 1);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_wheel_picker_base_set_selected_index(p_zero, 5);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* on_tick when item_count == 0 looping (branches 688, 714) */
    rc = ui_wheel_picker_base_on_tick(p_zero, 16.0);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* on_tick when item_count == 0 non-looping (branch 653) */
    rc = ui_wheel_picker_base_set_looping(p_zero, 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_wheel_picker_base_on_tick(p_zero, 16.0);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    /* set_items when count == 0 and selected_index >= count (branch 454) */
    p_zero->selected_index = 0;
    rc = ui_wheel_picker_base_set_items(p_zero, NULL, 0);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = ui_wheel_picker_base_destroy(p_zero);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Destroy with shadow_root == NULL and component != NULL (covers line 454) */
  {
    struct ui_wheel_picker_base *p_sh = NULL;
    rc = ui_wheel_picker_base_create(&p_sh, NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_destroy(p_sh->component->shadow_root);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    p_sh->component->shadow_root = NULL;
    rc = ui_wheel_picker_base_destroy(p_sh);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  PASS();
}

TEST test_wheel_error_branches(void) {
  struct ui_wheel_picker_base *picker = NULL;
  const char *items[] = {"A", "B"};
  struct ui_control_value_accessor cva;
  struct ui_event ev;
  ui_error_t rc;
  int i;

  /* 1. Gesture process_event mock failure */
  rc = ui_wheel_picker_base_create(&picker, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_wheel_picker_base_set_items(picker, items, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  g_wheel_mock_fail = 4;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  /* 2. on_change error in set_selected_index (line 406) */
  rc = ui_wheel_picker_base_set_on_change(picker, on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cb_fail_on_change = 1;
  rc = ui_wheel_picker_base_set_selected_index(picker, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_fail_on_change = 0;

  /* 3. cva_on_touched error in process_event (line 458) */
  rc = cva.register_on_touched(picker, cva_on_touched, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cb_fail_touched = 1;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_fail_touched = 0;

  /* 4. roundf fallback mock failure in on_tick (line 521) */
  g_wheel_mock_fail = 5;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  /* 5. on_change and cva_on_change error in on_tick (lines 552, 561) */
  rc = cva.register_on_change(picker, cva_on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->scroll_offset = 40.2f; /* snap to 1 */
  picker->selected_index = 0;

  g_cb_fail_on_change = 1;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_fail_on_change = 0;

  picker->scroll_offset = 40.2f; /* snap to 1 */
  picker->selected_index = 0;
  g_cb_fail_cva_change = 1;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_fail_cva_change = 0;

  /* update_dom_state failure in on_tick */
  picker->scroll_offset = 40.2f;
  picker->selected_index = 0;
  g_wheel_mock_fail = 6;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  /* Looping tick with negative offset (line 691) */
  picker->is_looping = 1;
  picker->scroll_offset = -40.2f;
  rc = ui_wheel_picker_base_on_tick(picker, 16.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->is_looping = 0;

  /* UI_KEY_UP error in process_event (line 611) */
  picker->selected_index = 1;
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  g_cb_fail_on_change = 1;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_fail_on_change = 0;

  /* UI_KEY_DOWN error in process_event (line 615) */
  picker->selected_index = 0;
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  g_cb_fail_on_change = 1;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cb_fail_on_change = 0;

  /* update_dom_state failure in set_selected_index (line 552) */
  picker->selected_index = 0;
  g_wheel_mock_fail = 6;
  rc = ui_wheel_picker_base_set_selected_index(picker, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  /* 6. Destroy mock failures */
  g_wheel_mock_fail = 3; /* gesture destroy fails */
  rc = ui_wheel_picker_base_destroy(picker);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  rc = ui_wheel_picker_base_create(&picker, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_wheel_mock_fail = 2; /* dom_node_destroy fails */
  rc = ui_wheel_picker_base_destroy(picker);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  rc = ui_wheel_picker_base_create(&picker, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_wheel_mock_fail = 1; /* component_destroy fails */
  rc = ui_wheel_picker_base_destroy(picker);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  /* 7. Creation cleanup failures */
  for (i = 11; i <= 15; i++) {
    g_wheel_mock_fail = i;
    rc = ui_wheel_picker_base_create(&picker, NULL);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_wheel_mock_fail = 0;
  }
  g_wheel_mock_fail = 6;
  rc = ui_wheel_picker_base_create(&picker, NULL);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_wheel_mock_fail = 0;

  PASS();
}

TEST test_wheel_oom(void) {
  struct ui_wheel_picker_base *picker = NULL;
  const char *items[] = {"A", "B", "C"};
  ui_error_t rc;
  int i;

  /* Creation OOM */
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    picker = NULL;
    rc = ui_wheel_picker_base_create(&picker, NULL);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_wheel_picker_base_destroy(picker);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(picker == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* set_items OOM (array malloc fail) */
  rc = ui_wheel_picker_base_create(&picker, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = ui_wheel_picker_base_set_items(picker, items, 3);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* set_items OOM (item strdup fail on item 0) */
  g_malloc_fail_countdown = 1;
  rc = ui_wheel_picker_base_set_items(picker, items, 3);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* set_items OOM (item strdup fail on item 1, covers lines 497-498) */
  g_malloc_fail_countdown = 2;
  rc = ui_wheel_picker_base_set_items(picker, items, 3);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  rc = ui_wheel_picker_base_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_wheel_picker_base_suite) {
  RUN_TEST(test_wheel_invalid_args);
  RUN_TEST(test_wheel_lifecycle_and_scrolling);
  RUN_TEST(test_wheel_error_branches);
  RUN_TEST(test_wheel_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_wheel_picker_base_suite);
  GREATEST_MAIN_END();
}
