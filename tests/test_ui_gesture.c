/* clang-format off */
#include "ui_gesture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define EXPECT_TRUE(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("Failed at %d: %s\n", __LINE__, #cond);                           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define EXPECT_EQ_INT(a, b)                                                    \
  do {                                                                         \
    int _a = (a);                                                              \
    int _b = (b);                                                              \
    if (_a != _b) {                                                            \
      printf("Failed at %d: %s == %s (%d != %d)\n", __LINE__, #a, #b, _a, _b); \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define EXPECT_EQ_FLOAT(a, b)                                                  \
  do {                                                                         \
    float _a = (a);                                                            \
    float _b = (b);                                                            \
    if (fabs(_a - _b) > 0.001f) {                                              \
      printf("Failed at %d: %s == %s (%f != %f)\n", __LINE__, #a, #b, _a, _b); \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int test_create_destroy(void) {
  struct ui_gesture_recognizer *r = NULL;
  enum ui_error err;

  err = ui_gesture_recognizer_create(NULL);
  EXPECT_EQ_INT(err, UI_ERROR_INVALID_ARGUMENT);

  err = ui_gesture_recognizer_create(&r);
  EXPECT_EQ_INT(err, UI_ERROR_NONE);
  EXPECT_TRUE(r != NULL);

  err = ui_gesture_recognizer_destroy(NULL);
  EXPECT_EQ_INT(err, UI_ERROR_INVALID_ARGUMENT);

  err = ui_gesture_recognizer_destroy(r);
  EXPECT_EQ_INT(err, UI_ERROR_NONE);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_gesture_recognizer *r = NULL;
  enum ui_error err;

  g_malloc_fail_countdown = 0;
  err = ui_gesture_recognizer_create(&r);
  g_malloc_fail_countdown = -1;
  EXPECT_EQ_INT(err, UI_ERROR_OUT_OF_MEMORY);
  return 0;
}

static int test_null_args_process(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  memset(&ev, 0, sizeof(ev));
  memset(&ge, 0, sizeof(ge));
  ui_gesture_recognizer_create(&r);

  EXPECT_EQ_INT(ui_gesture_recognizer_process_event(NULL, &ev, 0, &ge),
                UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ_INT(ui_gesture_recognizer_process_event(r, NULL, 0, &ge),
                UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ_INT(ui_gesture_recognizer_process_event(r, &ev, 0, NULL),
                UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ_INT(ui_gesture_recognizer_update(NULL, 0, &ge),
                UI_ERROR_INVALID_ARGUMENT);
  EXPECT_EQ_INT(ui_gesture_recognizer_update(r, 0, NULL),
                UI_ERROR_INVALID_ARGUMENT);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_unrelated_event(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  memset(&ev, 0, sizeof(ev));
  memset(&ge, 0, sizeof(ge));
  ui_gesture_recognizer_create(&r);

  ev.type = UI_EVENT_KEY_DOWN; /* not a pointer event */
  ui_gesture_recognizer_process_event(r, &ev, 0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_NONE);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_mouse_tap(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  memset(&ev, 0, sizeof(ev));
  memset(&ge, 0, sizeof(ge));
  ui_gesture_recognizer_create(&r);

  /* MOUSE_DOWN */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  /* MOUSE_UP (quick tap) */
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 50.0, &ge);

  EXPECT_EQ_INT(ge.type, UI_GESTURE_TAP);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_ENDED);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_mouse_long_press(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  /* MOUSE_DOWN */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  /* Update to trigger long press */
  ui_gesture_recognizer_update(r, 400.0, &ge);
  ui_gesture_recognizer_update(r, 600.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_LONG_PRESS);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_BEGAN);

  /* MOUSE_UP */
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 700.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_LONG_PRESS);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_ENDED);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_mouse_pan_and_swipe(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  /* MOUSE_DOWN */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  /* MOUSE_MOVE -> triggers pan began */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 115; /* > UI_PAN_MIN_DISTANCE */
  ev.event_data.mouse.y = 115;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_PAN);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_BEGAN);

  /* MOUSE_MOVE again -> triggers pan changed */
  ev.event_data.mouse.x = 120;
  ev.event_data.mouse.y = 120;
  ui_gesture_recognizer_process_event(r, &ev, 20.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_PAN);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_CHANGED);

  /* MOUSE_UP -> triggers swipe (velocity is high enough) */
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 120;
  ev.event_data.mouse.y = 120;
  ui_gesture_recognizer_process_event(r, &ev, 21.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_SWIPE);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_ENDED);

  ui_gesture_recognizer_destroy(r);

  /* test pan without swipe (low velocity) */
  ui_gesture_recognizer_create(&r);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 115;
  ev.event_data.mouse.y = 115;
  ui_gesture_recognizer_process_event(r, &ev, 1000.0, &ge);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 115;
  ev.event_data.mouse.y = 115;
  ui_gesture_recognizer_process_event(r, &ev, 1001.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_PAN);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_ENDED);
  ui_gesture_recognizer_destroy(r);

  return 0;
}

static int test_touch_cancel(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  /* TOUCH_START */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  /* Move enough to trigger pan */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 120;
  ev.event_data.touch.points[0].y = 120;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  /* Cancel */
  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_gesture_recognizer_process_event(r, &ev, 20.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_PAN);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_CANCELLED);

  ui_gesture_recognizer_destroy(r);

  /* Cancel after long press */
  ui_gesture_recognizer_create(&r);
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);
  ui_gesture_recognizer_update(r, 400.0, &ge);
  ui_gesture_recognizer_update(r, 600.0, &ge);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_gesture_recognizer_process_event(r, &ev, 700.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_LONG_PRESS);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_CANCELLED);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_touch_multi(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  /* TOUCH_START */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  /* Second touch down (processed as TOUCH_MOVE with 2 points in this naive
   * model) */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 2;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ev.event_data.touch.points[1].x = 200;
  ev.event_data.touch.points[1].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  EXPECT_EQ_INT(ge.type, UI_GESTURE_PINCH);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_BEGAN);

  /* Pinch further */
  ev.event_data.touch.points[1].x = 300;
  ui_gesture_recognizer_process_event(r, &ev, 20.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_PINCH);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_CHANGED);

  /* Rotate */
  ev.event_data.touch.points[1].x = 100;
  ev.event_data.touch.points[1].y = 300;
  ui_gesture_recognizer_process_event(r, &ev, 30.0, &ge);
  EXPECT_EQ_INT(ge.type, UI_GESTURE_ROTATION);
  EXPECT_EQ_INT(ge.state, UI_GESTURE_STATE_CHANGED);

  /* TOUCH_END */
  ev.type = UI_EVENT_TOUCH_END;
  ev.event_data.touch.num_points = 0; /* naive */
  ui_gesture_recognizer_process_event(r, &ev, 40.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_mouse_other_button(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  memset(&ge, 0, sizeof(ge));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 1;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_MOUSE_UP;
  ui_gesture_recognizer_process_event(r, &ev, 50.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_touch_zero_points(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  memset(&ge, 0, sizeof(ge));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 0;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_touch_end_with_points(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_TOUCH_END;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 50.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_move_zero_dt(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 150;
  ev.event_data.mouse.y = 150;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_move_after_long_press(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ui_gesture_recognizer_update(r, 400.0, &ge);
  ui_gesture_recognizer_update(r, 600.0, &ge);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 150;
  ev.event_data.mouse.y = 150;
  ui_gesture_recognizer_process_event(r, &ev, 650.0, &ge);

  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  ui_gesture_recognizer_process_event(r, &ev, 660.0, &ge);

  ev.type = UI_EVENT_MOUSE_UP;
  ui_gesture_recognizer_process_event(r, &ev, 670.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_up_no_tap_no_long_press(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  /* Up after 400ms (too slow for tap, too fast for long press since update
   * wasn't called) */
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 400.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_multi_touch_small_changes(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 2;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ev.event_data.touch.points[1].x = 200;
  ev.event_data.touch.points[1].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  /* Move second point very slightly (scale and rotation diff < 0.05) */
  ev.event_data.touch.points[1].x = 201;
  ev.event_data.touch.points[1].y = 101;
  ui_gesture_recognizer_process_event(r, &ev, 20.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_cancel_neither(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_update_untracking(void) {
  struct ui_gesture_recognizer *r;
  struct ui_gesture_event ge;
  struct ui_event ev;

  ui_gesture_recognizer_create(&r);

  /* Not tracking */
  ui_gesture_recognizer_update(r, 100.0, &ge);

  /* Tracking but has moved */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 200;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  ui_gesture_recognizer_update(r, 400.0, &ge);
  ui_gesture_recognizer_update(r, 600.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_cancel_after_move_and_long_press(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ui_gesture_recognizer_update(r, 600.0, &ge);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 150;
  ev.event_data.touch.points[0].y = 150;
  ui_gesture_recognizer_process_event(r, &ev, 650.0, &ge);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_gesture_recognizer_process_event(r, &ev, 700.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_untracked_events(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);
  memset(&ev, 0, sizeof(ev));
  memset(&ge, 0, sizeof(ge));

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_gesture_recognizer_process_event(r, &ev, 20.0, &ge);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 2;
  ui_gesture_recognizer_process_event(r, &ev, 30.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

static int test_tiny_move_and_double_update(void) {
  struct ui_gesture_recognizer *r;
  struct ui_event ev;
  struct ui_gesture_event ge;

  ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  ui_gesture_recognizer_process_event(r, &ev, 0.0, &ge);

  /* Tiny move < PAN_MIN_DISTANCE */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 105;
  ev.event_data.mouse.y = 105;
  ui_gesture_recognizer_process_event(r, &ev, 10.0, &ge);

  /* Double update to hit long_press_triggered == true */
  ui_gesture_recognizer_update(r, 600.0, &ge);
  ui_gesture_recognizer_update(r, 700.0, &ge);

  ui_gesture_recognizer_destroy(r);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_create_destroy();
  failed |= run_oom_tests();
  failed |= test_null_args_process();
  failed |= test_unrelated_event();
  failed |= test_mouse_tap();
  failed |= test_mouse_long_press();
  failed |= test_mouse_pan_and_swipe();
  failed |= test_touch_cancel();
  failed |= test_touch_multi();
  failed |= test_mouse_other_button();
  failed |= test_touch_zero_points();
  failed |= test_touch_end_with_points();
  failed |= test_move_zero_dt();
  failed |= test_move_after_long_press();
  failed |= test_up_no_tap_no_long_press();
  failed |= test_multi_touch_small_changes();
  failed |= test_cancel_neither();
  failed |= test_update_untracking();
  failed |= test_cancel_after_move_and_long_press();
  failed |= test_untracked_events();
  failed |= test_tiny_move_and_double_update();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All test_ui_gesture passed.\n");
  return 0;
}
