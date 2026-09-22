static int run_edge_cases(void);
static int run_mock_tests(void);
/* clang-format off */
#include "ui_spin_button_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_change_called = 0;
static double g_last_val = 0.0;
static int g_cva_change_called = 0;
static float g_cva_last_val = 0.0f;
static int g_cva_touched_called = 0;

static ui_error_t on_change(struct ui_spin_button_base *sb, double val,
                            void *user) {
  if (sb) {
  }
  if (user) {
  }
  g_change_called++;
  g_last_val = val;
  return UI_ERROR_NONE;
}

static ui_error_t on_cva_change(union ui_signal_payload val, void *user) {
  if (user) {
  }
  g_cva_change_called++;
  g_cva_last_val = val.float_val;
  return UI_ERROR_NONE;
}

static ui_error_t mock_action_cb_fail(struct ui_spin_button_base *sb,
                                      double val, void *user) {
  if (sb) {
  }
  if (val > 0.0) {
  }
  if (user) {
  }
  return UI_ERROR_UNKNOWN;
}

static ui_error_t on_cva_touched(void *user) {
  if (user) {
  }
  g_cva_touched_called++;
  return UI_ERROR_NONE;
}

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      {                                                                        \
        printf("Failed at %d\n", __LINE__);                                    \
        do {                                                                   \
          printf("Failed at %d\n", __LINE__);                                  \
          {                                                                    \
            printf("mock fail at line %d\n", __LINE__);                        \
            return 1;                                                          \
          }                                                                    \
        } while (0);                                                           \
      }                                                                        \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
    if (err != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             err);                                                             \
      {                                                                        \
        printf("Failed at %d\n", __LINE__);                                    \
        do {                                                                   \
          printf("Failed at %d\n", __LINE__);                                  \
          {                                                                    \
            printf("mock fail at line %d\n", __LINE__);                        \
            return 1;                                                          \
          }                                                                    \
        } while (0);                                                           \
      }                                                                        \
    }                                                                          \
  } while (0)

static int run_normal_tests(void) {
  struct ui_spin_button_base *sb = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *comp = NULL;
  union ui_signal_payload payload;
  double val;
  struct ui_event ev;

  /* Null checks */
  ASSERT_EQ(ui_spin_button_base_create(NULL, NULL), UI_ERROR_INVALID_ARGUMENT);
  {
    ui_error_t rc_cleanup = ui_spin_button_base_destroy(NULL);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  ASSERT_EQ(ui_spin_button_base_set_min(NULL, 0.0), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_set_max(NULL, 100.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_set_value(NULL, 10.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_get_value(NULL, &val),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_set_step(NULL, 1.0), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_set_disabled(NULL, 1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_set_on_change(NULL, on_change, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_increment(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_decrement(NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_start_continuous_increment(NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_start_continuous_decrement(NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_stop_continuous(NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_on_tick(NULL, 0.0), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_process_event(NULL, &ev),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);

  /* Create normal */
  ASSERT_SUCCESS(ui_spin_button_base_create(&sb, &cva));
  ASSERT_EQ(ui_spin_button_base_get_value(sb, NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_process_event(sb, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_get_component(sb, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_spin_button_base_get_component(sb, &comp));
  if (!comp) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  /* CVA functions */
  payload.float_val = 10.0f;
  ASSERT_EQ(cva.write_value(NULL, payload), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(cva.register_on_change(NULL, on_cva_change, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(cva.register_on_touched(NULL, on_cva_touched, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(cva.set_disabled_state(NULL, 1), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(cva.register_on_change(sb, on_cva_change, NULL));
  ASSERT_SUCCESS(cva.register_on_touched(sb, on_cva_touched, NULL));

  ASSERT_SUCCESS(ui_spin_button_base_set_on_change(sb, on_change, NULL));

  /* Test basic property sets */
  ASSERT_SUCCESS(ui_spin_button_base_set_min(sb, -10.0));
  ASSERT_SUCCESS(ui_spin_button_base_set_max(sb, 50.0));
  ASSERT_SUCCESS(ui_spin_button_base_set_step(sb, 2.5));

  /* Out of bounds clamping on prop set */
  ASSERT_SUCCESS(ui_spin_button_base_set_value(sb, 100.0)); /* clamps to 50 */
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 50.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  ASSERT_SUCCESS(ui_spin_button_base_set_value(sb, -50.0)); /* clamps to -10 */
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != -10.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  /* Modifying limits clamps value */
  ASSERT_SUCCESS(ui_spin_button_base_set_value(sb, 30.0));
  ASSERT_SUCCESS(ui_spin_button_base_set_max(sb, 20.0));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 20.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  ASSERT_SUCCESS(ui_spin_button_base_set_value(sb, 10.0));
  ASSERT_SUCCESS(ui_spin_button_base_set_min(sb, 15.0));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 15.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  /* CVA Write Value */
  payload.float_val = 18.0f;
  ASSERT_SUCCESS(cva.write_value(sb, payload));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 18.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  /* Increment / Decrement */
  g_change_called = 0;
  ASSERT_SUCCESS(ui_spin_button_base_increment(sb));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 20.0 || g_change_called != 1) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  ASSERT_SUCCESS(ui_spin_button_base_decrement(sb));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 17.5 || g_change_called != 2) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  /* Event processing */
  g_cva_touched_called = 0;
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ASSERT_SUCCESS(ui_spin_button_base_process_event(sb, &ev));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 20.0 || g_cva_touched_called != 1) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ASSERT_SUCCESS(ui_spin_button_base_process_event(sb, &ev));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 17.5 || g_cva_touched_called != 2) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ASSERT_SUCCESS(ui_spin_button_base_process_event(sb, &ev));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 15.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  ev.event_data.keyboard.key_code = UI_KEY_END;
  ASSERT_SUCCESS(ui_spin_button_base_process_event(sb, &ev));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 20.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  /* Unhandled event */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ASSERT_SUCCESS(ui_spin_button_base_process_event(sb, &ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_LEFT; /* Unhandled key */
  ASSERT_SUCCESS(ui_spin_button_base_process_event(sb, &ev));

  /* Continuous increment/decrement */
  ASSERT_SUCCESS(ui_spin_button_base_set_value(sb, 15.0));
  ASSERT_SUCCESS(ui_spin_button_base_on_tick(
      sb, 100.0)); /* None active, should do nothing */

  ASSERT_SUCCESS(ui_spin_button_base_start_continuous_increment(sb));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 17.5) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  ASSERT_SUCCESS(ui_spin_button_base_on_tick(sb, 400.0)); /* total 400 < 500 */
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 17.5) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  ASSERT_SUCCESS(
      ui_spin_button_base_on_tick(sb, 100.0)); /* total 500 >= 500, repeats */
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 20.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  ASSERT_SUCCESS(ui_spin_button_base_on_tick(sb, 50.0)); /* >= 50 repeat rate */
  /* clamped to 20 */
  ASSERT_SUCCESS(ui_spin_button_base_stop_continuous(sb));

  ASSERT_SUCCESS(ui_spin_button_base_start_continuous_decrement(sb));
  ASSERT_SUCCESS(ui_spin_button_base_on_tick(sb, 500.0));
  ASSERT_SUCCESS(ui_spin_button_base_get_value(sb, &val));
  if (val != 15.0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  ASSERT_SUCCESS(ui_spin_button_base_stop_continuous(sb));

  /* Disabled state */
  ASSERT_SUCCESS(cva.set_disabled_state(sb, 1));
  ASSERT_EQ(ui_spin_button_base_increment(sb), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_decrement(sb), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_start_continuous_increment(sb),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_start_continuous_decrement(sb),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_spin_button_base_on_tick(sb, 500.0), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_spin_button_base_process_event(
      sb, &ev)); /* Does nothing and returns none */

  ASSERT_SUCCESS(cva.set_disabled_state(sb, 0));

  /* Edge case update_aria fail */
  ASSERT_SUCCESS(ui_spin_button_base_set_value(
      sb, 16.0)); /* update_aria fails silently but we test logic */
  ASSERT_SUCCESS(ui_spin_button_base_set_min(sb, 14.0));
  ASSERT_SUCCESS(ui_spin_button_base_set_max(sb, 22.0));

  {
    ui_error_t rc_cleanup = ui_spin_button_base_destroy(sb);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_spin_button_base *sb = NULL;
  struct ui_control_value_accessor cva;

  g_malloc_fail_countdown = 0;
  if (ui_spin_button_base_create(&sb, &cva) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  g_malloc_fail_countdown = 1;
  if (ui_spin_button_base_create(&sb, &cva) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  g_malloc_fail_countdown = 2;
  if (ui_spin_button_base_create(&sb, &cva) != UI_ERROR_OUT_OF_MEMORY)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  g_malloc_fail_countdown = 3;
  if (ui_spin_button_base_create(&sb, &cva) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

static int run_error_bubbles(void);

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  if (run_edge_cases() != 0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  if (run_oom_tests() != 0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
  if (run_error_bubbles() != 0) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }
#ifdef UI_TEST_MOCK_ALLOC
  if (run_mock_tests() != 0) {
    printf("Failed at %d\n", __LINE__);
    {
      printf("mock fail at line %d\n", __LINE__);
      return 1;
    }
  }
#endif
  printf("All ui_spin_button_base tests passed.\n");
  return 0;
}

struct ui_spin_button_base {
  struct ui_component *component;
};

static int run_error_bubbles(void) {
  struct ui_spin_button_base *sb = NULL;
  struct ui_control_value_accessor cva;
  struct ui_event ev;
  union ui_signal_payload payload;
  ui_spin_button_base_create(&sb, &cva);

  /* 1. cva_set_disabled_state error bubbling */
  {
    struct ui_component *orig = sb->component;
    sb->component = NULL;
    if (cva.set_disabled_state(sb, 1) != UI_ERROR_INVALID_ARGUMENT)
      do {
        printf("Failed at %d\n", __LINE__);
        {
          printf("mock fail at line %d\n", __LINE__);
          return 1;
        }
      } while (0);
    sb->component = orig;
  }

  /* Set up failing on_change */
  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);

  if (ui_spin_button_base_set_value(sb, 1.0) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  payload.float_val = 2.0f;
  if (cva.write_value(sb, payload) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);

  /* The min/max setters trigger set_value with the boundary if they modify the
   * boundaries */
  ui_spin_button_base_set_on_change(sb, on_change, NULL);
  ui_spin_button_base_set_value(sb, 0.0);
  ui_spin_button_base_set_min(sb, -10.0);
  ui_spin_button_base_set_max(sb, 10.0);

  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);
  /* Triggers a value change to 5.0 (min) */
  if (ui_spin_button_base_set_min(sb, 5.0) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  /* Value is now 5.0. Change max to -5.0 triggers value change to -5.0 */
  if (ui_spin_button_base_set_max(sb, -5.0) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);

  ui_spin_button_base_set_on_change(sb, on_change, NULL);
  ui_spin_button_base_set_min(sb, -10.0);
  ui_spin_button_base_set_max(sb, 10.0);
  ui_spin_button_base_set_value(sb, 0.0);
  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);

  if (ui_spin_button_base_increment(sb) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  if (ui_spin_button_base_decrement(sb) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);

  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  if (ui_spin_button_base_process_event(sb, &ev) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  if (ui_spin_button_base_process_event(sb, &ev) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);

  ui_spin_button_base_set_on_change(sb, on_change, NULL);
  ui_spin_button_base_set_value(sb, 0.0);
  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  if (ui_spin_button_base_process_event(sb, &ev) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  ev.event_data.keyboard.key_code = UI_KEY_END;
  if (ui_spin_button_base_process_event(sb, &ev) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);

  ui_spin_button_base_set_on_change(sb, on_change, NULL);
  ui_spin_button_base_set_value(sb, 0.0);
  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);

  ui_spin_button_base_start_continuous_increment(sb);
  if (ui_spin_button_base_on_tick(sb, 1000.0) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  ui_spin_button_base_stop_continuous(sb);

  ui_spin_button_base_set_on_change(sb, on_change, NULL);
  ui_spin_button_base_set_value(sb, 0.0);
  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);

  ui_spin_button_base_start_continuous_decrement(sb);
  if (ui_spin_button_base_on_tick(sb, 1000.0) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  ui_spin_button_base_stop_continuous(sb);

  /* continuous min/max tick clamp errors */
  ui_spin_button_base_set_on_change(sb, on_change, NULL);
  ui_spin_button_base_set_value(sb, -4.0); /* close to max -5.0 */
  ui_spin_button_base_start_continuous_increment(sb);
  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);
  if (ui_spin_button_base_on_tick(sb, 1000.0) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  ui_spin_button_base_stop_continuous(sb);

  ui_spin_button_base_set_on_change(sb, on_change, NULL);
  ui_spin_button_base_set_value(sb, 6.0); /* close to min 5.0 */
  ui_spin_button_base_start_continuous_decrement(sb);
  ui_spin_button_base_set_on_change(sb, mock_action_cb_fail, NULL);
  if (ui_spin_button_base_on_tick(sb, 1000.0) != UI_ERROR_UNKNOWN)
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  ui_spin_button_base_stop_continuous(sb);

  ui_spin_button_base_destroy(sb);
  return 0;
}

static int run_edge_cases(void) {
  struct ui_spin_button_base *sb = NULL;
  ui_spin_button_base_create(&sb, NULL);
  if (!sb) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  struct ui_component *comp;
  ui_spin_button_base_get_component(sb, &comp);

  if (comp && comp->shadow_root) {
    struct ui_dom_node *orig_root = comp->shadow_root;
    comp->shadow_root = NULL;
    ui_spin_button_base_set_value(
        sb, 5.0); /* triggers !shadow_root branch in update_aria */
    comp->shadow_root = orig_root;

    struct ui_component *orig_comp = sb->component;
    sb->component = NULL;
    ui_spin_button_base_set_value(
        sb, 6.0); /* triggers !component branch in update_aria */
    sb->component = orig_comp;

    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(orig_root);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
    comp->shadow_root = NULL;
  }

  ui_spin_button_base_set_min(sb, 10.0);
  if (ui_spin_button_base_set_disabled(sb, 1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    do {
      printf("Failed at %d\n", __LINE__);
      {
        printf("mock fail at line %d\n", __LINE__);
        return 1;
      }
    } while (0);
  }

  {
    ui_error_t rc_cleanup = ui_spin_button_base_destroy(sb);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  return 0;
}

#ifdef UI_TEST_MOCK_ALLOC
static ui_error_t mock_cva_on_change_fail(union ui_signal_payload val,
                                          void *user) {
  float unused;
  void *unused_u;
  unused = val.float_val;
  val.float_val = unused;
  unused_u = user;
  user = unused_u;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t mock_cva_on_touched_fail(void *user) {
  void *unused_u;
  unused_u = user;
  user = unused_u;
  return UI_ERROR_UNKNOWN;
}

static int run_mock_tests(void) {
  extern int g_spin_button_mock_set_attr_fail_target;
  extern int g_spin_button_mock_remove_attr_fail_target;
  extern int g_spin_button_mock_create_comp_fail;
  extern int g_spin_button_mock_create_node_fail;
  extern int g_spin_button_mock_set_tag_fail;
  extern int g_spin_button_mock_stop_cont_fail;
  extern int g_spin_button_set_attr_counter;
  extern int g_spin_button_remove_attr_counter;
  struct ui_spin_button_base *sb = NULL;
  struct ui_control_value_accessor cva;
  struct ui_event ev;
  int target;

  /* 1. Test create failures */
  g_spin_button_mock_create_comp_fail = 1;
  if (ui_spin_button_base_create(&sb, NULL) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_create_comp_fail = 0;

  g_spin_button_mock_create_node_fail = 1;
  if (ui_spin_button_base_create(&sb, NULL) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_create_node_fail = 0;

  g_spin_button_mock_set_tag_fail = 1;
  if (ui_spin_button_base_create(&sb, NULL) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_set_tag_fail = 0;

  for (target = 1; target <= 5; target++) {
    g_spin_button_set_attr_counter = 0;
    g_spin_button_mock_set_attr_fail_target = target;
    if (ui_spin_button_base_create(&sb, NULL) != UI_ERROR_UNKNOWN) {
      printf("mock fail at line %d\n", __LINE__);
      return 1;
    }
    g_spin_button_mock_set_attr_fail_target = 0;
  }

  /* 2. Test set_disabled failures */
  g_spin_button_set_attr_counter = 0;
  if (ui_spin_button_base_create(&sb, &cva) != UI_ERROR_NONE) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }

  g_spin_button_set_attr_counter = 0;
  g_spin_button_mock_set_attr_fail_target = 1;
  if (ui_spin_button_base_set_disabled(sb, 1) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_set_attr_fail_target = 0;

  g_spin_button_remove_attr_counter = 0;
  g_spin_button_mock_remove_attr_fail_target = 1;
  if (ui_spin_button_base_set_disabled(sb, 1) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_remove_attr_fail_target = 0;

  g_spin_button_set_attr_counter = 0;
  g_spin_button_remove_attr_counter = 0;
  g_spin_button_mock_stop_cont_fail = 1;
  if (ui_spin_button_base_set_disabled(sb, 1) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_stop_cont_fail = 0;

  /* Now disable succeeds */
  g_spin_button_set_attr_counter = 0;
  g_spin_button_remove_attr_counter = 0;
  if (ui_spin_button_base_set_disabled(sb, 1) != UI_ERROR_NONE) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }

  /* Now enable failures */
  g_spin_button_remove_attr_counter = 0;
  g_spin_button_mock_remove_attr_fail_target = 1;
  if (ui_spin_button_base_set_disabled(sb, 0) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_remove_attr_fail_target = 0;

  g_spin_button_set_attr_counter = 0;
  g_spin_button_mock_set_attr_fail_target = 1;
  if (ui_spin_button_base_set_disabled(sb, 0) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_set_attr_fail_target = 0;

  /* 3. Test set_value update_aria failure */
  g_spin_button_set_attr_counter = 0;
  g_spin_button_mock_set_attr_fail_target = 1;
  if (ui_spin_button_base_set_value(sb, 42.0) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  g_spin_button_mock_set_attr_fail_target = 0;

  /* 4. Test trigger_cva_change failure */
  cva.register_on_change(sb, mock_cva_on_change_fail, NULL);
  if (ui_spin_button_base_set_value(sb, 43.0) != UI_ERROR_UNKNOWN) {
    printf("mock fail at line %d\n", __LINE__);
    return 1;
  }
  cva.register_on_change(sb, NULL, NULL);

  /* 5. Test trigger_cva_touched failure in process_event */
  cva.register_on_touched(sb, mock_cva_on_touched_fail, NULL);
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  if (ui_spin_button_base_process_event(sb, &ev) != UI_ERROR_UNKNOWN)
    return 1;
  cva.register_on_touched(sb, NULL, NULL);

  /* Exercise remove_attr target > 1 so counter != target branch is taken */
  g_spin_button_remove_attr_counter = 0;
  g_spin_button_mock_remove_attr_fail_target = 2;
  ui_spin_button_base_set_disabled(sb, 1);
  ui_spin_button_base_set_disabled(sb, 0);
  g_spin_button_mock_remove_attr_fail_target = 0;

  /* Exercise update_aria NULL checks */
  {
    extern ui_error_t ui_test_sb_update_aria(struct ui_spin_button_base * sb);
    struct ui_component comp_no_root;
    struct ui_component *orig_comp = sb->component;
    memset(&comp_no_root, 0, sizeof(comp_no_root));

    if (ui_test_sb_update_aria(NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;

    sb->component = NULL;
    if (ui_test_sb_update_aria(sb) != UI_ERROR_INVALID_ARGUMENT)
      return 1;

    sb->component = &comp_no_root;
    if (ui_test_sb_update_aria(sb) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    sb->component = orig_comp;
  }

  /* Exercise destroy with active continuous scroll */
  ui_spin_button_base_start_continuous_increment(sb);

  ui_spin_button_base_destroy(sb);
  return 0;
}
#endif
