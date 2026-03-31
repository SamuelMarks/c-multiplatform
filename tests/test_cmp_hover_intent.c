/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_hover_intent_create_destroy(void) {
  cmp_hover_intent_t *intent = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_create(&intent));
  ASSERT_NEQ(NULL, intent);
  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_destroy(intent));

  PASS();
}

TEST test_hover_intent_process_success(void) {
  cmp_hover_intent_t *intent = NULL;
  cmp_event_t event_move = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_create(&intent));

  event_move.action = CMP_ACTION_MOVE;
  event_move.x = 100;
  event_move.y = 100;

  /* Initial registration (not hovered yet) */
  ASSERT_EQ(0, cmp_hover_intent_process(intent, &event_move, 0.0f));

  /* Move slightly within tolerance over time */
  event_move.x = 102;
  ASSERT_EQ(0, cmp_hover_intent_process(intent, &event_move, 50.0f));

  event_move.y = 102;
  ASSERT_EQ(0, cmp_hover_intent_process(intent, &event_move, 50.0f));

  /* Exceed the time threshold! */
  ASSERT_EQ(1, cmp_hover_intent_process(intent, &event_move,
                                        60.0f)); /* 160ms total */

  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_destroy(intent));
  PASS();
}

TEST test_hover_intent_process_reset(void) {
  cmp_hover_intent_t *intent = NULL;
  cmp_event_t event = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_create(&intent));

  event.action = CMP_ACTION_MOVE;
  event.x = 100;
  event.y = 100;

  ASSERT_EQ(0, cmp_hover_intent_process(intent, &event, 0.0f));
  ASSERT_EQ(
      0, cmp_hover_intent_process(intent, &event, 100.0f)); /* Almost there */

  /* Large movement resets timer (x jumps out of tolerance radius 5px) */
  event.x = 150;
  ASSERT_EQ(0, cmp_hover_intent_process(intent, &event, 50.0f));

  /* Validate it didn't trigger because of the reset */
  event.x = 151;
  ASSERT_EQ(0, cmp_hover_intent_process(intent, &event, 10.0f));

  /* Explicit cancel */
  event.action = CMP_ACTION_CANCEL;
  ASSERT_EQ(0, cmp_hover_intent_process(intent, &event, 0.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_destroy(intent));
  PASS();
}

TEST test_hover_intent_edge_cases(void) {
  cmp_hover_intent_t *intent = NULL;
  cmp_event_t event = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_hover_intent_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_hover_intent_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_create(&intent));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_hover_intent_process(NULL, &event, 0.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_hover_intent_process(intent, NULL, 0.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_hover_intent_destroy(intent));
  PASS();
}

SUITE(cmp_hover_intent_suite) {
  RUN_TEST(test_hover_intent_create_destroy);
  RUN_TEST(test_hover_intent_process_success);
  RUN_TEST(test_hover_intent_process_reset);
  RUN_TEST(test_hover_intent_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_hover_intent_suite);
  GREATEST_MAIN_END();
}
