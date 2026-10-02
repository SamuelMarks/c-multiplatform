/**
 * @file test_cupertino_gesture.c
 * @brief Unit tests for Cupertino edge-swipe interactive pop gesture and
 * accessibility two-finger "Z" scrub gesture.
 */

/* clang-format off */
#include "cupertino/cupertino_gesture.h"
#include "greatest.h"
#include "ui_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_cupertino_pop_gesture_ltr(void) {
  struct cupertino_pop_gesture gesture;
  int began = 0;
  int should_pop = 0;
  float progress = 0.0f;
  float offset_x = 0.0f;
  ui_error_t rc;

  /* Invalid arguments */
  rc = cupertino_pop_gesture_init(NULL, 375.0f, UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_pop_gesture_init(&gesture, 0.0f, UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_pop_gesture_init(&gesture, 375.0f, (enum ui_bidi_direction)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_pop_gesture_init(&gesture, 375.0f, UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_INACTIVE, gesture.state);

  /* Touch outside edge slop (> 20pt) */
  rc = cupertino_pop_gesture_touch_down(&gesture, 30.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, began);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_INACTIVE, gesture.state);

  /* Touch within edge slop (10pt) */
  rc = cupertino_pop_gesture_touch_down(&gesture, 10.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, began);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_TRACKING, gesture.state);

  /* Touch move: pan to 100pt */
  rc = cupertino_pop_gesture_touch_move(&gesture, 110.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.2f && progress < 0.3f); /* 100 / 375 = ~0.266 */

  /* Underlying view parallax offset */
  rc = cupertino_pop_gesture_get_underlying_offset(&gesture, &offset_x);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(offset_x < 0.0f); /* In LTR, underlying offset is negative */

  /* Touch up with low progress and low velocity -> should cancel */
  rc = cupertino_pop_gesture_touch_up(&gesture, 100.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, should_pop);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_CANCELLED, gesture.state);

  /* New gesture: pan past 50% screen width -> should pop */
  rc = cupertino_pop_gesture_touch_down(&gesture, 5.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, began);

  rc = cupertino_pop_gesture_touch_move(&gesture, 250.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.5f);

  rc = cupertino_pop_gesture_touch_up(&gesture, 0.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, should_pop);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_COMPLETED, gesture.state);

  /* High velocity fling (>= 500 pt/s) even at low progress -> should pop */
  rc = cupertino_pop_gesture_touch_down(&gesture, 5.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_move(&gesture, 50.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_up(&gesture, 600.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, should_pop);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_COMPLETED, gesture.state);

  PASS();
}

TEST test_cupertino_pop_gesture_rtl(void) {
  struct cupertino_pop_gesture gesture;
  int began = 0;
  int should_pop = 0;
  float progress = 0.0f;
  float offset_x = 0.0f;
  ui_error_t rc;

  /* Viewport 375pt wide in RTL: right edge is 375 - 20 = 355 to 375 */
  rc = cupertino_pop_gesture_init(&gesture, 375.0f, UI_BIDI_DIR_RTL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Touch outside right edge slop */
  rc = cupertino_pop_gesture_touch_down(&gesture, 340.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, began);

  /* Touch inside right edge */
  rc = cupertino_pop_gesture_touch_down(&gesture, 370.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, began);

  /* Pan leftwards (decreasing X) */
  rc = cupertino_pop_gesture_touch_move(&gesture, 270.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(progress > 0.2f);

  /* In RTL, underlying offset starts positive */
  rc = cupertino_pop_gesture_get_underlying_offset(&gesture, &offset_x);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(offset_x > 0.0f);

  /* High negative velocity in RTL -> pop */
  rc = cupertino_pop_gesture_touch_up(&gesture, -600.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, should_pop);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_COMPLETED, gesture.state);

  PASS();
}

TEST test_cupertino_scrub_gesture(void) {
  struct cupertino_scrub_gesture scrub;
  int recognized = 0;
  ui_error_t rc;

  /* Invalid args */
  rc = cupertino_scrub_gesture_init(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrub_gesture_touch_down(NULL, 2, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrub_gesture_touch_down(&scrub, 0, 0.0f, 0.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrub_gesture_touch_move(NULL, 10.0f, 10.0f, &recognized);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrub_gesture_touch_move(&scrub, 10.0f, 10.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_scrub_gesture_reset(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_scrub_gesture_init(&scrub);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Single finger down -> fails */
  rc = cupertino_scrub_gesture_touch_down(&scrub, 1, 100.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_FAILED, scrub.state);

  /* Two fingers down */
  rc = cupertino_scrub_gesture_touch_down(&scrub, 2, 100.0f, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_1, scrub.state);

  /* Segment 1: Swipe right to (150, 105) */
  rc = cupertino_scrub_gesture_touch_move(&scrub, 150.0f, 105.0f, &recognized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, recognized);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_2, scrub.state);

  /* Segment 2: Swipe diagonally down-left to (110, 150) */
  rc = cupertino_scrub_gesture_touch_move(&scrub, 110.0f, 150.0f, &recognized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, recognized);
  ASSERT_EQ(CUPERTINO_SCRUB_RECOGNIZED, scrub.state);

  /* Subsequent moves remain recognized */
  rc = cupertino_scrub_gesture_touch_move(&scrub, 130.0f, 150.0f, &recognized);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, recognized);

  /* Reset */
  rc = cupertino_scrub_gesture_reset(&scrub);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_IDLE, scrub.state);

  PASS();
}

TEST test_cupertino_gesture_forward_event(void) {
  struct cupertino_pop_gesture pop;
  struct cupertino_scrub_gesture scrub;
  struct ui_event ev;
  int handled = 0;
  ui_error_t rc;

  rc = cupertino_pop_gesture_init(&pop, 375.0f, UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&scrub, 0, sizeof(scrub));

  /* Mouse down at edge (x=10, y=50) */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 10;
  ev.event_data.mouse.y = 50;

  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_TRACKING, pop.state);

  /* Mouse move to (x=120, y=50) */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 120;
  ev.event_data.mouse.y = 50;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);

  /* Mouse up */
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 120;
  ev.event_data.mouse.y = 50;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);

  /* Touch start with 2 fingers for Z scrub */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 2;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 100;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_1, scrub.state);

  /* Touch move segment 1 */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 150;
  ev.event_data.touch.points[0].y = 105;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_2, scrub.state);

  /* Touch move segment 2 */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.points[0].x = 110;
  ev.event_data.touch.points[0].y = 150;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);
  ASSERT_EQ(CUPERTINO_SCRUB_RECOGNIZED, scrub.state);

  /* Touch end */
  ev.type = UI_EVENT_TOUCH_END;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_IDLE, scrub.state);

  /* Null checks */
  rc = cupertino_gesture_forward_event(&pop, &scrub, NULL, &handled);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_cupertino_gesture_mock_touch_down_fail;
extern int g_cupertino_gesture_mock_touch_move_fail;
extern int g_cupertino_gesture_mock_touch_up_fail;
extern int g_cupertino_gesture_mock_scrub_down_fail;
extern int g_cupertino_gesture_mock_scrub_move_fail;
extern int g_cupertino_gesture_mock_scrub_reset_fail;
#endif

TEST test_cupertino_gesture_coverage_branches(void) {
  struct cupertino_pop_gesture pop;
  struct cupertino_scrub_gesture scrub;
  struct ui_event ev;
  float progress = 0.0f;
  float offset = 0.0f;
  int began = 0;
  int should_pop = 0;
  int handled = 0;
  ui_error_t rc;

  /* pop_gesture null checks */
  rc = cupertino_pop_gesture_touch_down(NULL, 10.0f, &began);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_pop_gesture_touch_down(&pop, 10.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_pop_gesture_touch_move(NULL, 10.0f, &progress);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_pop_gesture_touch_move(&pop, 10.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_pop_gesture_touch_up(NULL, 0.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_pop_gesture_touch_up(&pop, 0.0f, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_pop_gesture_get_underlying_offset(NULL, &offset);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_pop_gesture_get_underlying_offset(&pop, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Touch move when not tracking -> out_progress = 0 */
  rc = cupertino_pop_gesture_init(&pop, 375.0f, UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_move(&pop, 50.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, progress);

  /* Touch up when not tracking -> out_should_pop = 0 */
  rc = cupertino_pop_gesture_touch_up(&pop, 0.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, should_pop);

  /* LTR negative delta (swipe leftwards after starting) -> delta clamped to 0
   */
  rc = cupertino_pop_gesture_touch_down(&pop, 10.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, began);
  rc = cupertino_pop_gesture_touch_move(&pop, 5.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, progress);

  /* LTR large delta -> prog clamped to 1.0 */
  rc = cupertino_pop_gesture_touch_move(&pop, 500.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, progress);

  /* RTL negative delta and prog clamped to 1.0 */
  rc = cupertino_pop_gesture_init(&pop, 375.0f, UI_BIDI_DIR_RTL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_down(&pop, 370.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, began);
  /* Moving further right (x > start_x) in RTL -> delta < 0 clamped to 0 */
  rc = cupertino_pop_gesture_touch_move(&pop, 375.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0.0f, progress);
  /* Moving far left in RTL -> prog clamped to 1.0 */
  rc = cupertino_pop_gesture_touch_move(&pop, -100.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1.0f, progress);

  /* RTL touch up with progress >= 0.5f and low velocity -> should_pop = 1 */
  rc = cupertino_pop_gesture_touch_up(&pop, 0.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, should_pop);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_COMPLETED, pop.state);

  /* RTL touch up with low progress and low velocity -> cancel */
  rc = cupertino_pop_gesture_touch_down(&pop, 370.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_move(&pop, 360.0f, &progress);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_up(&pop, 0.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, should_pop);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_CANCELLED, pop.state);

  /* Scrub gesture state branches in touch_move */
  rc = cupertino_scrub_gesture_init(&scrub);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Idle state */
  rc = cupertino_scrub_gesture_touch_move(&scrub, 10.0f, 10.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, should_pop);

  /* Failed state */
  scrub.state = CUPERTINO_SCRUB_FAILED;
  rc = cupertino_scrub_gesture_touch_move(&scrub, 10.0f, 10.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, should_pop);

  /* Segment 1 but dx <= 30 while dy is small */
  scrub.state = CUPERTINO_SCRUB_SEGMENT_1;
  scrub.start_x = 100.0f;
  scrub.start_y = 100.0f;
  rc = cupertino_scrub_gesture_touch_move(&scrub, 120.0f, 100.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_1, scrub.state);

  /* Segment 1 but dx > 30 while dy is large (>= 20) */
  rc = cupertino_scrub_gesture_touch_move(&scrub, 140.0f, 130.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_1, scrub.state);

  /* Segment 2 but dx >= -25 while dy > 25 */
  scrub.state = CUPERTINO_SCRUB_SEGMENT_2;
  scrub.last_x = 150.0f;
  scrub.last_y = 100.0f;
  rc = cupertino_scrub_gesture_touch_move(&scrub, 140.0f, 130.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_2, scrub.state);

  /* Segment 2 but dx < -25 while dy <= 25 */
  rc = cupertino_scrub_gesture_touch_move(&scrub, 120.0f, 110.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCRUB_SEGMENT_2, scrub.state);

  /* Scrub with unknown / out-of-range state to hit switch default */
  scrub.state = (enum cupertino_scrub_state)999;
  rc = cupertino_scrub_gesture_touch_move(&scrub, 10.0f, 10.0f, &should_pop);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, should_pop);

  /* LTR negative touch_x (x < 0) -> not in edge */
  rc = cupertino_pop_gesture_init(&pop, 375.0f, UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_down(&pop, -5.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, began);

  /* LTR touch_x > edge_slop -> not in edge */
  rc = cupertino_pop_gesture_touch_down(&pop, 50.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, began);

  /* RTL touch_x > screen_width -> not in edge */
  rc = cupertino_pop_gesture_init(&pop, 375.0f, UI_BIDI_DIR_RTL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_pop_gesture_touch_down(&pop, 400.0f, &began);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, began);

  /* forward_event: mouse_move with scrub recognizing -> handled = 1 */
  rc = cupertino_scrub_gesture_init(&scrub);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  scrub.state = CUPERTINO_SCRUB_RECOGNIZED;
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);

  /* forward_event: TOUCH_START with num_points == 0 */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 0;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);

  /* forward_event: TOUCH_MOVE with num_points == 0 */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 0;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);

  /* forward_event: pop_gesture is INACTIVE during MOUSE_MOVE and MOUSE_UP */
  pop.state = CUPERTINO_POP_GESTURE_INACTIVE;
  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* forward_event: pop_gesture is INACTIVE during TOUCH_MOVE and TOUCH_END */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ev.event_data.touch.points[0].y = 50;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_END;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* forward_event: scrub_gesture is IDLE during MOUSE_MOVE and TOUCH_MOVE */
  scrub.state = CUPERTINO_SCRUB_IDLE;
  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 1;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* forward_event: unknown event type (default branch) */
  memset(&ev, 0, sizeof(ev));
  ev.type = (enum ui_event_type)999;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);

  /* forward_event: TOUCH_START with pop_gesture */
  rc = cupertino_pop_gesture_init(&pop, 375.0f, UI_BIDI_DIR_LTR);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_scrub_gesture_init(&scrub);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 10;
  ev.event_data.touch.points[0].y = 50;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);
  ASSERT_EQ(CUPERTINO_POP_GESTURE_TRACKING, pop.state);

  /* forward_event: TOUCH_MOVE with pop_gesture tracking */
  ev.type = UI_EVENT_TOUCH_MOVE;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 100;
  ev.event_data.touch.points[0].y = 50;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);

  /* forward_event: TOUCH_CANCEL with pop_gesture tracking */
  ev.type = UI_EVENT_TOUCH_CANCEL;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, handled);

  /* forward_event: mouse down outside edge -> handled = 0 */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 200;
  ev.event_data.mouse.y = 50;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);

  /* forward_event: touch start outside edge -> handled = 0 */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  ev.event_data.touch.points[0].x = 200;
  ev.event_data.touch.points[0].y = 50;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, handled);

  /* forward_event: error percolation tests using mocks */
#ifdef UI_TEST_MOCK_ALLOC
  /* MOUSE_DOWN pop fail */
  ev.type = UI_EVENT_MOUSE_DOWN;
  g_cupertino_gesture_mock_touch_down_fail = 1;
  rc = cupertino_gesture_forward_event(&pop, &scrub, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_touch_down_fail = 0;

  /* MOUSE_DOWN scrub fail */
  g_cupertino_gesture_mock_scrub_down_fail = 1;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_scrub_down_fail = 0;

  /* MOUSE_MOVE pop fail */
  pop.state = CUPERTINO_POP_GESTURE_TRACKING;
  ev.type = UI_EVENT_MOUSE_MOVE;
  g_cupertino_gesture_mock_touch_move_fail = 1;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_touch_move_fail = 0;

  /* MOUSE_MOVE scrub fail */
  scrub.state = CUPERTINO_SCRUB_SEGMENT_1;
  g_cupertino_gesture_mock_scrub_move_fail = 1;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_scrub_move_fail = 0;

  /* MOUSE_UP pop fail */
  pop.state = CUPERTINO_POP_GESTURE_TRACKING;
  ev.type = UI_EVENT_MOUSE_UP;
  g_cupertino_gesture_mock_touch_up_fail = 1;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_touch_up_fail = 0;

  /* MOUSE_UP scrub fail */
  g_cupertino_gesture_mock_scrub_reset_fail = 1;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_scrub_reset_fail = 0;

  /* TOUCH_START pop fail */
  ev.type = UI_EVENT_TOUCH_START;
  ev.event_data.touch.num_points = 1;
  g_cupertino_gesture_mock_touch_down_fail = 1;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_touch_down_fail = 0;

  /* TOUCH_START scrub fail */
  g_cupertino_gesture_mock_scrub_down_fail = 1;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_scrub_down_fail = 0;

  /* TOUCH_MOVE pop fail */
  pop.state = CUPERTINO_POP_GESTURE_TRACKING;
  ev.type = UI_EVENT_TOUCH_MOVE;
  g_cupertino_gesture_mock_touch_move_fail = 1;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_touch_move_fail = 0;

  /* TOUCH_MOVE scrub fail */
  scrub.state = CUPERTINO_SCRUB_SEGMENT_1;
  g_cupertino_gesture_mock_scrub_move_fail = 1;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_scrub_move_fail = 0;

  /* TOUCH_END pop fail */
  pop.state = CUPERTINO_POP_GESTURE_TRACKING;
  ev.type = UI_EVENT_TOUCH_END;
  g_cupertino_gesture_mock_touch_up_fail = 1;
  rc = cupertino_gesture_forward_event(&pop, NULL, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_touch_up_fail = 0;

  /* TOUCH_END scrub fail */
  g_cupertino_gesture_mock_scrub_reset_fail = 1;
  rc = cupertino_gesture_forward_event(NULL, &scrub, &ev, &handled);
  ASSERT_NEQ(UI_ERROR_NONE, rc);
  g_cupertino_gesture_mock_scrub_reset_fail = 0;
#endif

  PASS();
}

SUITE(cupertino_gesture_suite) {
  RUN_TEST(test_cupertino_pop_gesture_ltr);
  RUN_TEST(test_cupertino_pop_gesture_rtl);
  RUN_TEST(test_cupertino_scrub_gesture);
  RUN_TEST(test_cupertino_gesture_forward_event);
  RUN_TEST(test_cupertino_gesture_coverage_branches);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_gesture_suite);
  GREATEST_MAIN_END();
}
