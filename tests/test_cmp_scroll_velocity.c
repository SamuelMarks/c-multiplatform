/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_scroll_velocity_create_destroy(void) {
  cmp_scroll_velocity_t *tracker = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_create(&tracker));
  ASSERT_NEQ(NULL, tracker);
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_destroy(tracker));

  PASS();
}

TEST test_scroll_velocity_tracking(void) {
  cmp_scroll_velocity_t *tracker = NULL;
  cmp_event_t event = {0};
  float vx = 0.0f, vy = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_create(&tracker));

  event.action = CMP_ACTION_DOWN;
  event.x = 100;
  event.y = 100;
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_push(tracker, &event, 0.0f));

  event.action = CMP_ACTION_MOVE;
  event.x = 100;
  event.y = 150;
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_push(tracker, &event, 16.0f));

  event.action = CMP_ACTION_MOVE;
  event.x = 100;
  event.y = 200;
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_push(tracker, &event, 16.0f));

  event.action = CMP_ACTION_UP;
  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_push(tracker, &event, 0.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_get(tracker, &vx, &vy));

  ASSERT_EQ(0.0f, vx);
  /* 100px total move over 32ms is approx 3125 px/sec */
  ASSERT(vy > 3000.0f && vy < 3200.0f);

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_destroy(tracker));
  PASS();
}

TEST test_scroll_velocity_edge_cases(void) {
  cmp_scroll_velocity_t *tracker = NULL;
  cmp_event_t event = {0};
  float vx, vy;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_velocity_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_velocity_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_create(&tracker));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_velocity_push(NULL, &event, 16.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_velocity_push(tracker, NULL, 16.0f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_scroll_velocity_get(NULL, &vx, &vy));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_velocity_get(tracker, NULL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_velocity_destroy(tracker));
  PASS();
}

TEST test_scroll_momentum_decay(void) {
  float init_v = 1000.0f;
  float platform_decay = 0.998f; /* iOS style */
  float offset = 0.0f;
  float new_v = 0.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_scroll_momentum_calculate(
                             init_v, 16.666f, platform_decay, &offset, &new_v));

  /* After exactly 1 frame duration, speed should be decayed by platform_decay
   * once */
  ASSERT(new_v < init_v);
  ASSERT(new_v > 990.0f && new_v < 999.0f);

  /* Validate edge cases */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_momentum_calculate(init_v, 16.0f, platform_decay, NULL,
                                          &new_v));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_scroll_momentum_calculate(init_v, 16.0f, platform_decay,
                                          &offset, NULL));

  PASS();
}

TEST test_overscroll_chaining(void) {
  /* No boundary hit = 1 (bubbling) */
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_AUTO, 0));
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_CONTAIN, 0));
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_NONE, 0));

  /* Boundary Hit */
  ASSERT_EQ(1, cmp_overscroll_evaluate(CMP_OVERSCROLL_AUTO, 1)); /* Chains */
  ASSERT_EQ(0,
            cmp_overscroll_evaluate(CMP_OVERSCROLL_CONTAIN, 1)); /* Trapped */
  ASSERT_EQ(0, cmp_overscroll_evaluate(CMP_OVERSCROLL_NONE, 1)); /* Trapped */

  PASS();
}

SUITE(cmp_scroll_velocity_suite) {
  RUN_TEST(test_scroll_velocity_create_destroy);
  RUN_TEST(test_scroll_velocity_tracking);
  RUN_TEST(test_scroll_velocity_edge_cases);
  RUN_TEST(test_scroll_momentum_decay);
  RUN_TEST(test_overscroll_chaining);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_scroll_velocity_suite);
  GREATEST_MAIN_END();
}
