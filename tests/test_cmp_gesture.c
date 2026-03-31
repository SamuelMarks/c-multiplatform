/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

static void gesture_callback(cmp_gesture_t *gesture, cmp_ui_node_t *node,
                             void *user_data) {
  (void)gesture;
  (void)node;
  if (user_data) {
    int *counter = (int *)user_data;
    (*counter)++;
  }
}

TEST test_gesture_create_destroy(void) {
  cmp_gesture_t *gesture = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_create(&gesture));
  ASSERT_NEQ(NULL, gesture);
  ASSERT_EQ(CMP_GESTURE_STATE_POSSIBLE, cmp_gesture_get_state(gesture));

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_destroy(gesture));
  PASS();
}

TEST test_gesture_state_machine(void) {
  cmp_gesture_t *gesture = NULL;
  cmp_event_t event_down = {0};
  cmp_event_t event_move = {0};
  cmp_event_t event_up = {0};
  cmp_event_t event_cancel = {0};
  int callback_count = 0;

  event_down.action = CMP_ACTION_DOWN;
  event_move.action = CMP_ACTION_MOVE;
  event_up.action = CMP_ACTION_UP;
  event_cancel.action = CMP_ACTION_CANCEL;

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_create(&gesture));
  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_set_callback(gesture, gesture_callback,
                                                  &callback_count));

  /* Validate progression to ENDED */
  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_process_event(gesture, &event_down));
  ASSERT_EQ(CMP_GESTURE_STATE_BEGAN, cmp_gesture_get_state(gesture));

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_process_event(gesture, &event_move));
  ASSERT_EQ(CMP_GESTURE_STATE_CHANGED, cmp_gesture_get_state(gesture));

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_process_event(gesture, &event_up));
  ASSERT_EQ(CMP_GESTURE_STATE_POSSIBLE,
            cmp_gesture_get_state(gesture)); /* Loops to idle */

  ASSERT_EQ(3, callback_count); /* Began, Changed, Ended */

  /* Validate CANCELLED */
  callback_count = 0;
  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_process_event(gesture, &event_down));
  ASSERT_EQ(CMP_GESTURE_STATE_BEGAN, cmp_gesture_get_state(gesture));

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_process_event(gesture, &event_cancel));
  ASSERT_EQ(CMP_GESTURE_STATE_POSSIBLE, cmp_gesture_get_state(gesture));
  ASSERT_EQ(2, callback_count); /* Began, Cancelled */

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_destroy(gesture));
  PASS();
}

TEST test_gesture_edge_cases(void) {
  cmp_gesture_t *gesture = NULL;
  cmp_event_t event = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_gesture_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_gesture_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_create(&gesture));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gesture_set_callback(NULL, gesture_callback, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gesture_set_callback(gesture, NULL, NULL));

  ASSERT_EQ(CMP_GESTURE_STATE_POSSIBLE, cmp_gesture_get_state(NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_gesture_process_event(NULL, &event));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_gesture_process_event(gesture, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_gesture_destroy(gesture));
  PASS();
}

SUITE(cmp_gesture_suite) {
  RUN_TEST(test_gesture_create_destroy);
  RUN_TEST(test_gesture_state_machine);
  RUN_TEST(test_gesture_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_gesture_suite);
  GREATEST_MAIN_END();
}
