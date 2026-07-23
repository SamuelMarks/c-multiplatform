/* clang-format off */
#include "ui_split_pane_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    enum ui_error err = (expr);                                                \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    enum ui_error err = (expr);                                                \
    if (err != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             err);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int run_normal_tests(void) {
  struct ui_split_pane_base *pane = NULL;
  int pos;
  enum ui_split_pane_orientation ori;
  struct ui_event ev;

  /* Null checks */
  ASSERT_EQ(ui_split_pane_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);
  ui_split_pane_base_destroy(NULL);

  ASSERT_EQ(ui_split_pane_base_set_orientation(
                NULL, UI_SPLIT_PANE_ORIENTATION_VERTICAL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_get_orientation(NULL, &ori),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_set_position(NULL, 100),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_get_position(NULL, &pos),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_set_bounds(NULL, 0, 100),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_process_event(NULL, &ev),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_bind_data(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_split_pane_base_create(&pane));
  ASSERT_EQ(ui_split_pane_base_get_orientation(pane, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_get_position(pane, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_split_pane_base_process_event(pane, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  /* Set & Get Orientation */
  ASSERT_SUCCESS(ui_split_pane_base_set_orientation(
      pane, UI_SPLIT_PANE_ORIENTATION_VERTICAL));
  ASSERT_SUCCESS(ui_split_pane_base_get_orientation(pane, &ori));
  if (ori != UI_SPLIT_PANE_ORIENTATION_VERTICAL)
    return 1;

  ASSERT_SUCCESS(ui_split_pane_base_set_orientation(
      pane, UI_SPLIT_PANE_ORIENTATION_HORIZONTAL));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));
  ev.type = UI_EVENT_KEY_UP;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ASSERT_SUCCESS(ui_split_pane_base_get_orientation(pane, &ori));
  if (ori != UI_SPLIT_PANE_ORIENTATION_HORIZONTAL)
    return 1;

  /* Bounds and Clamping */
  ASSERT_SUCCESS(ui_split_pane_base_set_bounds(pane, 100, 300));

  /* Bounds inverted should swap internally */
  ASSERT_SUCCESS(ui_split_pane_base_set_bounds(pane, 300, 100));

  ASSERT_SUCCESS(ui_split_pane_base_set_position(pane, 50)); /* Clamps to 100 */
  ASSERT_SUCCESS(ui_split_pane_base_get_position(pane, &pos));
  if (pos != 100)
    return 1;

  ASSERT_SUCCESS(
      ui_split_pane_base_set_position(pane, 400)); /* Clamps to 300 */
  ASSERT_SUCCESS(ui_split_pane_base_get_position(pane, &pos));
  if (pos != 300)
    return 1;

  ASSERT_SUCCESS(ui_split_pane_base_set_position(pane, 150));
  ASSERT_SUCCESS(ui_split_pane_base_get_position(pane, &pos));
  if (pos != 150)
    return 1;

  /* Bind data */
  ASSERT_SUCCESS(ui_split_pane_base_bind_data(pane, NULL));

  /* Horizontal Mouse Events */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 100;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 150;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));
  ASSERT_SUCCESS(ui_split_pane_base_get_position(pane, &pos));
  if (pos != 200)
    return 1; /* 150 (initial) + 50 (delta) */

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 150;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  /* Move after up should do nothing */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 200;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));
  ASSERT_SUCCESS(ui_split_pane_base_get_position(pane, &pos));
  if (pos != 200)
    return 1;

  /* Vertical Touch Events */
  ASSERT_SUCCESS(ui_split_pane_base_set_orientation(
      pane, UI_SPLIT_PANE_ORIENTATION_VERTICAL));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].y = 50;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].y = 20;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));
  ASSERT_SUCCESS(ui_split_pane_base_get_position(pane, &pos));
  if (pos != 170)
    return 1; /* 200 - 30 */

  ev.type = UI_EVENT_TOUCH_END;
  ev.event_data.touch.points[0].y = 20;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ASSERT_SUCCESS(ui_split_pane_base_set_orientation(
      pane, UI_SPLIT_PANE_ORIENTATION_VERTICAL));
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.y = 10;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  /* Unhandled events or no touch points */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ev.event_data.touch.num_points = 0;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_KEY_DOWN;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 0;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));
  ev.type = UI_EVENT_MOUSE_MOVE;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ASSERT_SUCCESS(ui_split_pane_base_set_orientation(
      pane, UI_SPLIT_PANE_ORIENTATION_HORIZONTAL));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));
  ev.type = UI_EVENT_KEY_UP;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ev.event_data.touch.num_points = 0;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_KEY_DOWN;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 0;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));
  ev.type = UI_EVENT_MOUSE_MOVE;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_KEY_DOWN;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 0;
  ASSERT_SUCCESS(ui_split_pane_base_process_event(pane, &ev));

  ui_split_pane_base_destroy(pane);
  return 0;
}

static int run_oom_tests(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_split_pane_base *pane = NULL;
  g_malloc_fail_countdown = 0;
  if (ui_split_pane_base_create(&pane) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

int main(void) {
  if (run_normal_tests() != 0)
    return 1;
  if (run_oom_tests() != 0)
    return 1;
  printf("All ui_split_pane_base tests passed.\n");
  return 0;
}
