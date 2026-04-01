/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_complex_gesture_create_destroy(void) {
  cmp_complex_gesture_t *gesture = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_complex_gesture_create(&gesture));
  ASSERT_NEQ(NULL, gesture);
  ASSERT_EQ(CMP_GESTURE_STATE_POSSIBLE, cmp_complex_gesture_get_state(gesture));

  ASSERT_EQ(CMP_SUCCESS, cmp_complex_gesture_destroy(gesture));
  PASS();
}

TEST test_complex_gesture_state_machine_and_deltas(void) {
  cmp_complex_gesture_t *gesture = NULL;
  cmp_event_t event_down = {0};
  cmp_event_t event_move = {0};
  int callback_count = 0;

  float pan_x, pan_y, scale, rot;

  event_down.action = CMP_ACTION_DOWN;
  event_down.x = 100;
  event_down.y = 100;

  event_move.action = CMP_ACTION_MOVE;
  event_move.x = 150;
  event_move.y = 50;

  (void)callback_count; /* Suppress unused variable warning */

  ASSERT_EQ(CMP_SUCCESS, cmp_complex_gesture_create(&gesture));
  /* Can't easily set callback via generic `cmp_gesture_set_callback` since
     structures are mocked opaquely without casting trickery. We'll directly
     invoke process_event and read states/deltas. */

  ASSERT_EQ(CMP_SUCCESS,
            cmp_complex_gesture_process_event(gesture, &event_down));
  ASSERT_EQ(CMP_GESTURE_STATE_BEGAN, cmp_complex_gesture_get_state(gesture));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_complex_gesture_process_event(gesture, &event_move));
  ASSERT_EQ(CMP_GESTURE_STATE_CHANGED, cmp_complex_gesture_get_state(gesture));

  ASSERT_EQ(CMP_SUCCESS, cmp_complex_gesture_get_deltas(gesture, &pan_x, &pan_y,
                                                        &scale, &rot));

  /* Validate pan logic mapping (150 - 100 = 50, 50 - 100 = -50) */
  ASSERT_EQ(50.0f, pan_x);
  ASSERT_EQ(-50.0f, pan_y);
  ASSERT_EQ(1.0f, scale); /* Unmocked in this simple state progression */
  ASSERT_EQ(0.0f, rot);   /* Unmocked */

  ASSERT_EQ(CMP_SUCCESS, cmp_complex_gesture_destroy(gesture));
  PASS();
}

TEST test_complex_gesture_edge_cases(void) {
  cmp_complex_gesture_t *gesture = NULL;
  cmp_event_t event = {0};
  float pan_x, pan_y, scale, rot;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_complex_gesture_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_complex_gesture_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_complex_gesture_create(&gesture));

  ASSERT_EQ(CMP_GESTURE_STATE_POSSIBLE, cmp_complex_gesture_get_state(NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_process_event(NULL, &event));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_process_event(gesture, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_complex_gesture_get_deltas(NULL, &pan_x, &pan_y, &scale, &rot));

  /* NULL output pointers should not crash */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_complex_gesture_get_deltas(gesture, NULL, NULL, NULL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_complex_gesture_destroy(gesture));
  PASS();
}

SUITE(cmp_complex_gesture_suite) {
  RUN_TEST(test_complex_gesture_create_destroy);
  RUN_TEST(test_complex_gesture_state_machine_and_deltas);
  RUN_TEST(test_complex_gesture_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_complex_gesture_suite);
  GREATEST_MAIN_END();
}
