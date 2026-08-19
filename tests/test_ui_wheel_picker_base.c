/* clang-format off */
#include "ui_wheel_picker_base.h"
#include "ui_gesture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int change_count = 0;
static int last_selected_index = -1;
static int touched_count = 0;
static int cva_change_count = 0;
static int cva_last_selected_index = -1;

#define ASSERT_TRUE(cond)                                                      \
  if (!(cond)) {                                                               \
    printf("Fail at %s:%d\n", __FILE__, __LINE__);                             \
    failed = 1;                                                                \
  }
#define ASSERT_EQ(a, b)                                                        \
  do {                                                                         \
    int _a = (int)(a);                                                         \
    int _b = (int)(b);                                                         \
    if (_a != _b) {                                                            \
      printf("Fail at %s:%d: %d != %d\n", __FILE__, __LINE__, _a, _b);         \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

static ui_error_t on_change(struct ui_wheel_picker_base *picker,
                            int selected_index, void *user_data) {
  (void)picker;
  (void)user_data;
  change_count++;
  last_selected_index = selected_index;
  return UI_ERROR_NONE;
}

static ui_error_t on_change_error(struct ui_wheel_picker_base *picker,
                                  int selected_index, void *user_data) {
  (void)picker;
  (void)selected_index;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t cva_on_change(union ui_signal_payload new_value,
                                void *user_data) {
  (void)user_data;
  cva_change_count++;
  cva_last_selected_index = new_value.int_val;
  return UI_ERROR_NONE;
}

static ui_error_t cva_on_touched(void *user_data) {
  (void)user_data;
  touched_count++;
  return UI_ERROR_NONE;
}

struct ui_wheel_picker_base_mock {
  struct ui_component *component;
  struct ui_gesture_recognizer *gesture_recognizer;

  char **items;
  int item_count;
  int is_looping;

  int selected_index;
  float scroll_offset;
  float velocity;
  int is_dragging;

  ui_error_t (*on_change)(struct ui_wheel_picker_base *, int, void *);
  void *on_change_user_data;

  ui_error_t (*cva_on_change)(union ui_signal_payload, void *);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *);
  void *cva_on_touched_user_data;

  int is_disabled;
};

static int test_creation() {
  struct ui_wheel_picker_base *picker = NULL;
  struct ui_control_value_accessor cva;
  int i;
  ui_error_t rc;
  int failed = 0;

  rc = ui_wheel_picker_base_create(NULL, NULL);
  ASSERT_EQ(rc, UI_ERROR_INVALID_ARGUMENT);

  rc = ui_wheel_picker_base_create(&picker, &cva);
  ASSERT_EQ(rc, UI_ERROR_NONE);
  ASSERT_TRUE(picker != NULL);
  (void)ui_wheel_picker_base_destroy(picker);

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    picker = NULL;
    rc = ui_wheel_picker_base_create(&picker, NULL);
    ASSERT_EQ(rc, UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
  }
  return failed;
}

static int test_items() {
  struct ui_wheel_picker_base *picker = NULL;
  int failed = 0;
  int i;
  const char *items[] = {"A", "B", "C"};

  ui_wheel_picker_base_create(&picker, NULL);

  ASSERT_EQ(ui_wheel_picker_base_set_items(NULL, items, 3),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_wheel_picker_base_set_items(picker, NULL, 3),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_EQ(ui_wheel_picker_base_set_items(picker, items, 3), UI_ERROR_NONE);

  /* Setting items again to cover freeing old items */
  ASSERT_EQ(ui_wheel_picker_base_set_items(picker, items, 3), UI_ERROR_NONE);

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    ASSERT_EQ(ui_wheel_picker_base_set_items(picker, items, 3),
              UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
  }
  (void)ui_wheel_picker_base_destroy(picker);

  return failed;
}

static int test_cva_and_events() {
  struct ui_wheel_picker_base *picker = NULL;
  struct ui_control_value_accessor cva = {0};
  union ui_signal_payload payload = {0};
  int failed = 0;
  struct ui_event ev = {0};
  const char *items[] = {"A", "B", "C"};

  ui_wheel_picker_base_create(&picker, &cva);
  ui_wheel_picker_base_set_items(picker, items, 3);
  ui_wheel_picker_base_set_on_change(picker, on_change, NULL);

  /* CVA */
  ASSERT_EQ(cva.register_on_change(NULL, cva_on_change, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(cva.register_on_touched(NULL, cva_on_touched, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(cva.write_value(NULL, payload), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(cva.set_disabled_state(NULL, 1), UI_ERROR_INVALID_ARGUMENT);

  cva.register_on_change(picker, cva_on_change, NULL);
  cva.register_on_touched(picker, cva_on_touched, NULL);
  payload.int_val = 1;
  cva.write_value(picker, payload);
  ASSERT_EQ(last_selected_index, 1);

  /* Set disabled state */
  cva.set_disabled_state(picker, 1);
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  /* Should ignore due to being disabled */
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(last_selected_index, 1);

  cva.set_disabled_state(picker, 0);

  /* Keyboard Event */
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(last_selected_index, 0); /* 1 - 1 = 0 */
  ASSERT_EQ(touched_count,
            1); /* Write value shouldn't touch? wait, event touched */

  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ASSERT_EQ(last_selected_index, 1); /* 0 + 1 = 1 */

  (void)ui_wheel_picker_base_destroy(picker);
  return failed;
}

static int test_physics() {
  struct ui_wheel_picker_base *picker = NULL;
  struct ui_control_value_accessor cva = {0};
  int failed = 0;
  struct ui_event ev = {0};
  const char *items[] = {"A", "B", "C", "D", "E"};

  ui_wheel_picker_base_create(&picker, &cva);
  ui_wheel_picker_base_set_items(picker, items, 5);
  ui_wheel_picker_base_set_on_change(picker, on_change, NULL);
  cva.register_on_change(picker, cva_on_change, NULL);

  ASSERT_EQ(ui_wheel_picker_base_process_event(NULL, &ev, 0.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_wheel_picker_base_process_event(picker, NULL, 0.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_wheel_picker_base_on_tick(NULL, 0.0), UI_ERROR_INVALID_ARGUMENT);

  /* Start drag */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 0;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);

  /* Trigger BEGAN (move past threshold) */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 50;
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);

  /* Trigger CHANGED (actual drag) */
  ev.event_data.mouse.y = 0;
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);

  /* Trigger ENDED */
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.y = 0;
  ui_wheel_picker_base_process_event(picker, &ev, 48.0);

  /* Tick to integrate velocity */
  ui_wheel_picker_base_on_tick(picker, 16.0);

  /* Tick until velocity decays */
  {
    int i;
    for (i = 0; i < 1000; i++) {
      ui_wheel_picker_base_on_tick(picker, 16.0);
    }
  }

  ASSERT_TRUE(last_selected_index > 0); /* should have scrolled down */

  /* Loop */
  ui_wheel_picker_base_set_looping(picker, 1);

  /* Tick with negative scroll offset (out of bounds looping snap) */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 150; /* BEGAN */
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);

  ev.event_data.mouse.y = 200; /* CHANGED delta +50 */
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);

  ev.type = UI_EVENT_MOUSE_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 48.0);

  {
    int i;
    for (i = 0; i < 1000; i++) {
      ui_wheel_picker_base_on_tick(picker, 16.0);
    }
  }

  /* Test error prop in on_change */
  ui_wheel_picker_base_set_on_change(picker, on_change_error, NULL);
  ASSERT_EQ(ui_wheel_picker_base_set_selected_index(picker, 2),
            UI_ERROR_UNKNOWN);

  /* Test bound offset for non-looping when negative */
  ui_wheel_picker_base_set_looping(picker, 0);
  ui_wheel_picker_base_set_on_change(picker, NULL, NULL); /* Reset to no-op */
  ui_wheel_picker_base_set_selected_index(picker, 0);

  /* Set a negative offset manually via gesture */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 200; /* BEGAN */
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);
  ev.event_data.mouse.y = 500; /* CHANGED */
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 48.0);
  ui_wheel_picker_base_on_tick(picker, 16.0);

  /* Some other getters */
  ui_wheel_picker_base_set_looping(NULL, 1);
  /* Null checks */
  ui_wheel_picker_base_set_selected_index(NULL, 1);
  ui_wheel_picker_base_get_selected_index(NULL, NULL);
  ui_wheel_picker_base_get_selected_index(picker, NULL);
  ui_wheel_picker_base_set_on_change(NULL, NULL, NULL);
  ui_wheel_picker_base_set_items(NULL, items, 3);
  ui_wheel_picker_base_set_items(picker, NULL, 3);
  ui_wheel_picker_base_set_items(picker, NULL, 0);

  (void)ui_wheel_picker_base_destroy(picker);
  picker = NULL;

  return failed;
}

static int test_getters_and_misc() {
  struct ui_wheel_picker_base *picker = NULL;
  struct ui_component *comp = NULL;
  int failed = 0;
  int out_idx = -1;
  const char *items[] = {"A", "B", "C"};
  struct ui_event ev = {0};

  ui_wheel_picker_base_create(&picker, NULL);

  ASSERT_EQ(ui_wheel_picker_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_wheel_picker_base_get_component(picker, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_wheel_picker_base_get_component(picker, &comp), UI_ERROR_NONE);
  ASSERT_TRUE(comp != NULL);

  /* Get selected index success */
  ASSERT_EQ(ui_wheel_picker_base_get_selected_index(picker, &out_idx),
            UI_ERROR_NONE);
  ASSERT_EQ(out_idx, 0);

  last_selected_index = -1;
  ui_wheel_picker_base_set_on_change(picker, on_change, NULL);

  /* Set items reducing count to force index adjustment */
  ui_wheel_picker_base_set_items(picker, items, 3);
  ui_wheel_picker_base_set_selected_index(picker, 2);
  ui_wheel_picker_base_set_items(picker, items, 1);
  ASSERT_EQ(last_selected_index, 2); /* on_change wasn't bound, so it doesn't
                                        change, wait we just check out_idx */
  ui_wheel_picker_base_get_selected_index(picker, &out_idx);
  ASSERT_EQ(out_idx, 0);

  /* Setting items to 0 count */
  ui_wheel_picker_base_set_items(picker, items, 0);
  ui_wheel_picker_base_set_selected_index(picker, 2);
  ui_wheel_picker_base_get_selected_index(picker, &out_idx);
  ASSERT_EQ(out_idx, 0);

  /* Test programmatic selection bounds */
  ui_wheel_picker_base_set_items(picker, items, 3);
  ui_wheel_picker_base_set_looping(picker, 0);
  ui_wheel_picker_base_set_selected_index(picker, -5);
  ui_wheel_picker_base_get_selected_index(picker, &out_idx);
  ASSERT_EQ(out_idx, 0);

  ui_wheel_picker_base_set_selected_index(picker, 10);
  ui_wheel_picker_base_get_selected_index(picker, &out_idx);
  ASSERT_EQ(out_idx, 2);

  /* Looping bounds */
  /* Hit looping condition with 0 items */
  ui_wheel_picker_base_set_items(picker, items, 0);
  ui_wheel_picker_base_set_looping(picker, 1);
  ui_wheel_picker_base_set_selected_index(picker, 10);
  /* Fake a scroll offset > 0 to test target_index bounding */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 0;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 0;
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);
  ui_wheel_picker_base_on_tick(picker, 16.0);

  /* Hit looping condition with < 0 index (already done? let's do -16) */
  ui_wheel_picker_base_set_items(picker, items, 3);
  ui_wheel_picker_base_set_looping(picker, 1);
  ui_wheel_picker_base_set_selected_index(picker, -16);
  ui_wheel_picker_base_get_selected_index(picker, &out_idx);
  ASSERT_EQ(out_idx, 2);

  /* Trigger CVA without CVA bound */
  /* This is just hitting process_event without CVA registered, which tests
   * `trigger_cva_change` null check */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);

  /* Trigger gesture cancelled */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 50;
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);

  /* Fallthrough on key event */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 999;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);

  /* Snap with no items */
  ui_wheel_picker_base_set_looping(picker, 1);
  ui_wheel_picker_base_set_on_change(picker, NULL, NULL);
  ui_wheel_picker_base_set_items(picker, items, 0);
  ui_wheel_picker_base_set_looping(picker, 1);
  ui_wheel_picker_base_on_tick(picker, 1000.0);
  ui_wheel_picker_base_set_looping(picker, 0);
  ui_wheel_picker_base_set_selected_index(picker, 10);
  ui_wheel_picker_base_on_tick(picker, 1000.0);

  /* Looping with negative target index */
  ui_wheel_picker_base_set_items(picker, items, 3);
  ui_wheel_picker_base_set_looping(picker, 1);
  ui_wheel_picker_base_set_selected_index(picker, 0);
  ui_wheel_picker_base_set_selected_index(picker, -1);
  ui_wheel_picker_base_on_tick(picker, 1000.0);

  /* Trigger CVA change without CVA bound */
  ui_wheel_picker_base_set_looping(picker, 0);
  ui_wheel_picker_base_set_selected_index(picker, 0);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 80;
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);
  {
    int i;
    for (i = 0; i < 1000; i++) {
      ui_wheel_picker_base_on_tick(picker, 16.0);
    }
  }

  /* Trigger slow out of bounds drag to snap to < 0 */
  ui_wheel_picker_base_set_selected_index(picker, 0);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 150; /* BEGAN */
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 250; /* CHANGED delta +100 -> scroll_offset -100 */
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 250; /* CHANGED delta 0 -> velocity 0 */
  ui_wheel_picker_base_process_event(picker, &ev, 1000.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 2000.0);
  ui_wheel_picker_base_on_tick(picker,
                               16.0); /* will evaluate negative scroll_offset */

  /* Trigger slow out of bounds drag to snap to >= count */
  ui_wheel_picker_base_set_selected_index(picker, 2);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 50; /* BEGAN */
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = -50; /* CHANGED delta -100 -> scroll_offset +100 */
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = -50; /* CHANGED delta 0 -> velocity 0 */
  ui_wheel_picker_base_process_event(picker, &ev, 1000.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 2000.0);
  ui_wheel_picker_base_on_tick(picker, 16.0);

  /* Test error prop in on_change from tick */
  ui_wheel_picker_base_set_on_change(picker, on_change_error, NULL);
  ui_wheel_picker_base_set_selected_index(picker, 0);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 50; /* BEGAN */
  ui_wheel_picker_base_process_event(picker, &ev, 16.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = -50; /* CHANGED delta -100 */
  ui_wheel_picker_base_process_event(picker, &ev, 32.0);
  ui_wheel_picker_base_on_tick(picker, 16.0);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = -50; /* CHANGED zero velocity */
  ui_wheel_picker_base_process_event(picker, &ev, 1000.0);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_wheel_picker_base_process_event(picker, &ev, 2000.0);

  /* Send CANCEL to hit cancelled branch */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 0;
  ev.event_data.touch.points[0].y = 0;
  ev.event_data.touch.points[0].id = 0;
  ui_wheel_picker_base_process_event(picker, &ev, 2000.0);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].y = 100;
  ui_wheel_picker_base_process_event(picker, &ev, 2016.0);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_wheel_picker_base_process_event(picker, &ev, 2032.0);

  /* The next ticks should snap to 1 and call on_change, returning error */
  {
    int i;
    for (i = 0; i < 1000; i++) {
      if (ui_wheel_picker_base_on_tick(picker, 16.0) == UI_ERROR_UNKNOWN) {
        break;
      }
    }
  }

  /* Mock structure to hit NULL branches in static helpers */
  {
    struct ui_wheel_picker_base_mock *mpicker =
        (struct ui_wheel_picker_base_mock *)picker;
    ui_dom_node_destroy(mpicker->component->shadow_root);
    mpicker->component->shadow_root = NULL;
    /* update_dom_state now hits the null shadow_root branch */
    ui_wheel_picker_base_set_selected_index(picker, 1);

    ui_component_destroy(mpicker->component);
    mpicker->component = NULL;
    ui_wheel_picker_base_set_selected_index(picker, 0);

    /* hit update_dom_state(NULL) */
    /* Well, actually update_dom_state is internal, so we just set picker to
     * NULL in an API that calls it */
    ui_wheel_picker_base_set_selected_index(NULL, 0);

    /* cva_on_change == NULL branch */
    mpicker->cva_on_change = NULL;
    ui_wheel_picker_base_set_selected_index(picker, 1);

    /* cva_on_touched == NULL branch */
    mpicker->cva_on_touched = NULL;
    /* Simulate a touch end */
    ev.type = UI_EVENT_MOUSE_DOWN;
    ui_wheel_picker_base_process_event(picker, &ev, 0.0);
    ev.type = UI_EVENT_MOUSE_UP;
    ui_wheel_picker_base_process_event(picker, &ev, 16.0);

    /* gesture_recognizer == NULL branch in destroy */
    ui_gesture_recognizer_destroy(mpicker->gesture_recognizer);
    mpicker->gesture_recognizer = NULL;
  }

  /* Test destroy with shadow_root == NULL but component != NULL */
  {
    struct ui_wheel_picker_base *picker2 = NULL;
    ui_wheel_picker_base_create(&picker2, NULL);
    struct ui_wheel_picker_base_mock *mpicker2 =
        (struct ui_wheel_picker_base_mock *)picker2;
    if (mpicker2 && mpicker2->component && mpicker2->component->shadow_root) {
      ui_dom_node_destroy(mpicker2->component->shadow_root);
      mpicker2->component->shadow_root = NULL;
    }
    ui_wheel_picker_base_destroy(picker2);
  }

  (void)ui_wheel_picker_base_destroy(picker);
  (void)ui_wheel_picker_base_destroy(NULL);

  return failed;
}

int main(void) {
  int failed = 0;
  printf("test_creation\n");
  fflush(stdout);
  failed |= test_creation();
  printf("test_items\n");
  fflush(stdout);
  failed |= test_items();
  printf("test_cva_and_events\n");
  fflush(stdout);
  failed |= test_cva_and_events();
  printf("test_physics\n");
  fflush(stdout);
  failed |= test_physics();
  printf("test_getters_and_misc\n");
  fflush(stdout);
  failed |= test_getters_and_misc();

  if (!failed) {
    printf("test_ui_wheel_picker_base passed\n");
  } else {
    printf("test_ui_wheel_picker_base failed\n");
  }
  return failed;
}
