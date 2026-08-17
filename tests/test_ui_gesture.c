/* clang-format off */
#include "../include/ui_gesture.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
/* clang-format on */

struct ui_gesture_recognizer {
  int is_tracking;
  int start_x;
  int start_y;
  double start_time_ms;
  int last_x;
  int last_y;
  double last_time_ms;
  int has_moved_significantly;
  int long_press_triggered;
  float velocity_x;
  float velocity_y;
  float drag_threshold;

  /* For pinch/rotation */
  float initial_distance;
  float initial_angle;
  float last_scale;
  float last_rotation;
};

extern int g_malloc_fail_countdown;

static void test_gesture_basic(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;
  ui_error_t rc;

  /* Invalid args */
  rc = ui_gesture_recognizer_create(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_gesture_recognizer_create(&r);
  assert(rc == UI_ERROR_NONE);

  rc = ui_gesture_recognizer_destroy(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_gesture_recognizer_process_event(NULL, &ev, 0.0, &gev);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_gesture_recognizer_process_event(r, NULL, 0.0, &gev);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_gesture_recognizer_process_event(r, &ev, 0.0, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_gesture_recognizer_update(NULL, 0.0, &gev);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_gesture_recognizer_update(r, 0.0, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Untouched update */
  rc = ui_gesture_recognizer_update(r, 0.0, &gev);
  assert(rc == UI_ERROR_NONE);

  /* Unrelated event */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  rc = ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);
  assert(rc == UI_ERROR_NONE);
  assert(gev.type == UI_GESTURE_NONE);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 1; /* Right click */
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 1; /* Right click */
  (void)ui_gesture_recognizer_process_event(r, &ev, 100.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 0;
  (void)ui_gesture_recognizer_process_event(r, &ev, 100.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 0;
  (void)ui_gesture_recognizer_process_event(r, &ev, 100.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  /* Untouched MOUSE_UP */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  (void)ui_gesture_recognizer_process_event(r, &ev, 150.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  /* Untouched TOUCH_CANCEL */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_CANCEL;
  (void)ui_gesture_recognizer_process_event(r, &ev, 160.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  (void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_tap_mouse(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  ev.type = UI_EVENT_MOUSE_UP;
  (void)ui_gesture_recognizer_process_event(r, &ev, 100.0, &gev);
  assert(gev.type == UI_GESTURE_TAP);
  assert(gev.state == UI_GESTURE_STATE_ENDED);
  assert(gev.x == 100);
  assert(gev.y == 100);

  (void)(void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_tap_touch(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 50;
  ev.event_data.touch.points[0].y = 50;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  ev.type = UI_EVENT_TOUCH_END;
  ev.event_data.touch.num_points = 0; /* no points means it uses last_x */
  (void)ui_gesture_recognizer_process_event(r, &ev, 100.0, &gev);
  assert(gev.type == UI_GESTURE_TAP);
  assert(gev.x == 50);

  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 50;
  ev.event_data.touch.points[0].y = 50;
  (void)ui_gesture_recognizer_process_event(r, &ev, 200.0, &gev);

  ev.type = UI_EVENT_TOUCH_END;
  ev.event_data.touch.num_points = 1; /* num_points > 0 branch */
  ev.event_data.touch.points[0].x = 60;
  ev.event_data.touch.points[0].y = 60;
  (void)ui_gesture_recognizer_process_event(r, &ev, 300.0, &gev);
  assert(gev.type == UI_GESTURE_TAP);
  assert(gev.x == 60);

  (void)(void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_long_press(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);

  (void)ui_gesture_recognizer_update(r, 600.0, &gev);
  assert(gev.type == UI_GESTURE_LONG_PRESS);
  assert(gev.state == UI_GESTURE_STATE_BEGAN);

  ev.type = UI_EVENT_MOUSE_UP;
  (void)ui_gesture_recognizer_process_event(r, &ev, 650.0, &gev);
  assert(gev.type == UI_GESTURE_LONG_PRESS);
  assert(gev.state == UI_GESTURE_STATE_ENDED);

  (void)(void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_pan(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);

  /* Move slightly, less than UI_PAN_MIN_DISTANCE (10) */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 105;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 10.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  ev.event_data.mouse.x = 120; /* > UI_PAN_MIN_DISTANCE */
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 100.0, &gev);
  assert(gev.type == UI_GESTURE_PAN);
  assert(gev.state == UI_GESTURE_STATE_BEGAN);

  ev.event_data.mouse.x = 130;
  (void)ui_gesture_recognizer_process_event(r, &ev, 200.0, &gev);
  assert(gev.type == UI_GESTURE_PAN);
  assert(gev.state == UI_GESTURE_STATE_CHANGED);
  assert(gev.delta_x == 10.0f);

  ev.type = UI_EVENT_MOUSE_UP;
  (void)ui_gesture_recognizer_process_event(r, &ev, 300.0, &gev);
  assert(gev.type ==
         UI_GESTURE_PAN); /* velocity is low, so end pan not swipe */
  assert(gev.state == UI_GESTURE_STATE_ENDED);

  (void)ui_gesture_recognizer_destroy(r);

  /* Test dt == 0.0 in PAN */
  (void)ui_gesture_recognizer_create(&r);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 120;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 10.0, &gev); /* move */

  ev.event_data.mouse.x = 130;
  (void)ui_gesture_recognizer_process_event(r, &ev, 10.0,
                                            &gev); /* 0 time delta move */

  (void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_swipe(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 200; /* Move 100px in 10ms -> 10000px/s */
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 10.0, &gev);
  assert(gev.type == UI_GESTURE_PAN);
  assert(gev.state == UI_GESTURE_STATE_BEGAN);

  ev.type = UI_EVENT_MOUSE_UP;
  (void)ui_gesture_recognizer_process_event(r, &ev, 20.0, &gev);
  assert(gev.type == UI_GESTURE_SWIPE);
  assert(gev.state == UI_GESTURE_STATE_ENDED);

  (void)(void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_pinch(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 2;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ev.event_data.touch.points[1].x = 200;
  ev.event_data.touch.points[1].y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);

  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 2;
  ev.event_data.touch.points[0].x = 50;
  ev.event_data.touch.points[0].y = 100;
  ev.event_data.touch.points[1].x = 250;
  ev.event_data.touch.points[1].y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 10.0, &gev);
  assert(gev.type == UI_GESTURE_PINCH);
  assert(gev.state == UI_GESTURE_STATE_BEGAN);

  ev.event_data.touch.points[0].x = 0;
  ev.event_data.touch.points[1].x = 300;
  (void)ui_gesture_recognizer_process_event(r, &ev, 20.0, &gev);
  assert(gev.type == UI_GESTURE_PINCH);
  assert(gev.state == UI_GESTURE_STATE_CHANGED);

  /* Force rotation, keeping distance the same as previous (250 - 50 = 200) to
   * avoid pinch */
  /* Previous points: (0,100) and (300,100). Distance = 300. */
  /* We need a distance of 300 to keep scale constant (scale = 300 / 100 = 3) */
  ev.event_data.touch.points[0].x = 150;
  ev.event_data.touch.points[0].y = -50;
  ev.event_data.touch.points[1].x = 150;
  ev.event_data.touch.points[1].y = 250;
  (void)ui_gesture_recognizer_process_event(r, &ev, 30.0, &gev);
  assert(gev.type == UI_GESTURE_ROTATION);
  assert(gev.state == UI_GESTURE_STATE_CHANGED);

  /* Force tiny rotation to hit false branch */
  ev.event_data.touch.points[0].x = 150;
  ev.event_data.touch.points[0].y = -50;
  ev.event_data.touch.points[1].x = 151; /* Very slight angle change */
  ev.event_data.touch.points[1].y = 250;
  (void)ui_gesture_recognizer_process_event(r, &ev, 40.0, &gev);

  /* Force zero delta time event */
  ev.event_data.touch.points[0].x = 152;
  ev.event_data.touch.points[0].y = -50;
  (void)ui_gesture_recognizer_process_event(r, &ev, 40.0, &gev);

  (void)ui_gesture_recognizer_destroy(r);

  /* Test untouched TOUCH_MOVE */
  (void)ui_gesture_recognizer_create(&r);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 2;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  ev.event_data.touch.points[1].x = 200;
  ev.event_data.touch.points[1].y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  (void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_pan_long_press(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  /* Start long press */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);
  (void)ui_gesture_recognizer_update(r, 600.0, &gev);

  /* Move */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 120;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 650.0, &gev);
  assert(gev.type == UI_GESTURE_NONE); /* Already long pressed, shouldn't pan */

  /* Move again */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 130;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 660.0, &gev);
  assert(gev.type == UI_GESTURE_NONE); /* Still shouldn't pan */

  /* Up */
  ev.type = UI_EVENT_MOUSE_UP;
  (void)ui_gesture_recognizer_process_event(r, &ev, 700.0, &gev);
  assert(
      gev.type ==
      UI_GESTURE_NONE); /* Long press ended, but we moved significantly, but
                           wait. If we moved significantly, we still don't swipe
                           or end pan because we are long press triggered */

  (void)ui_gesture_recognizer_destroy(r);

  /* --- Cancel after long press and move --- */
  (void)ui_gesture_recognizer_create(&r);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);
  (void)ui_gesture_recognizer_update(r, 600.0, &gev); /* triggers long press */
  printf("Update at 600.0 gev.type = %d\n", gev.type);

  /* Move */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 120;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(
      r, &ev, 650.0, &gev); /* sets has_moved_significantly */

  (void)ui_gesture_recognizer_update(
      r, 680.0, &gev); /* Hit branch where tracking but has moved */
  printf("After 680: tracking=%d moved=%d long_press=%d\n", r->is_tracking,
         r->has_moved_significantly, r->long_press_triggered);

  /* Cancel */
  ev.type = UI_EVENT_TOUCH_CANCEL;
  ui_error_t leave_rc =
      ui_gesture_recognizer_process_event(r, &ev, 700.0, &gev);
  printf("Cancel gev.type = %d rc=%d\n", gev.type, leave_rc);
  assert(gev.type == UI_GESTURE_LONG_PRESS);

  (void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_duration_no_tap(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);

  ev.type = UI_EVENT_MOUSE_UP;
  /* > 300ms but < 500ms (no long press, no tap) */
  (void)ui_gesture_recognizer_process_event(r, &ev, 400.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  (void)(void)ui_gesture_recognizer_destroy(r);
}

static void test_gesture_cancel(void) {
  struct ui_gesture_recognizer *r = NULL;
  struct ui_event ev;
  struct ui_gesture_event gev;

  (void)ui_gesture_recognizer_create(&r);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 0.0, &gev);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 150;
  (void)ui_gesture_recognizer_process_event(r, &ev, 10.0, &gev);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  (void)ui_gesture_recognizer_process_event(r, &ev, 20.0, &gev);
  assert(gev.type == UI_GESTURE_PAN);
  assert(gev.state == UI_GESTURE_STATE_CANCELLED);

  /* Cancel a long press */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 30.0, &gev);

  (void)ui_gesture_recognizer_update(r, 630.0, &gev);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  (void)ui_gesture_recognizer_process_event(r, &ev, 640.0, &gev);
  assert(gev.type == UI_GESTURE_LONG_PRESS);
  assert(gev.state == UI_GESTURE_STATE_CANCELLED);

  /* Cancel a basic tap (no move, no long press) to cover remaining branch */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ev.event_data.mouse.x = 100;
  ev.event_data.mouse.y = 100;
  (void)ui_gesture_recognizer_process_event(r, &ev, 700.0, &gev);

  ev.type = UI_EVENT_TOUCH_CANCEL;
  (void)ui_gesture_recognizer_process_event(r, &ev, 710.0, &gev);
  assert(gev.type == UI_GESTURE_NONE);

  /* ui_gesture_recognizer_update branches */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  (void)ui_gesture_recognizer_process_event(r, &ev, 800.0, &gev);
  (void)ui_gesture_recognizer_update(
      r, 810.0, &gev); /* duration < UI_LONG_PRESS_MIN_DURATION_MS */
  assert(gev.type == UI_GESTURE_NONE);

  (void)ui_gesture_recognizer_update(r, 1400.0, &gev); /* Triggers long press */
  (void)ui_gesture_recognizer_update(
      r, 1500.0, &gev); /* r->long_press_triggered is true */
  assert(gev.type == UI_GESTURE_NONE);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 200;
  (void)ui_gesture_recognizer_process_event(r, &ev, 1600.0, &gev);
  (void)ui_gesture_recognizer_update(
      r, 1610.0, &gev); /* r->has_moved_significantly is true */
  assert(gev.type == UI_GESTURE_NONE);

  (void)(void)ui_gesture_recognizer_destroy(r);
}

static void run_oom_tests_gesture(void) {
  struct ui_gesture_recognizer *r;
  int i;
  ui_error_t rc;

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_gesture_recognizer_create(&r);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      (void)(void)ui_gesture_recognizer_destroy(r);
      break;
    }
  }
}

int main(void) {
  test_gesture_basic();
  test_gesture_tap_mouse();
  test_gesture_tap_touch();
  test_gesture_long_press();
  test_gesture_pan();
  test_gesture_swipe();
  test_gesture_pinch();
  test_gesture_pan_long_press();
  test_gesture_duration_no_tap();
  test_gesture_cancel();
  run_oom_tests_gesture();
  printf("All ui_gesture tests passed.\n");
  return 0;
}
