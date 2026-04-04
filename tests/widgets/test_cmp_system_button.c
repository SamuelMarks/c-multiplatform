/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_button_events_and_state(void) {
  cmp_button_t *ctx = NULL;
  cmp_event_t evt;
  cmp_button_style_t style;
  int is_pressed;

  ASSERT_EQ(CMP_SUCCESS, cmp_button_create(&ctx));

  /* Initial State */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_get_state(ctx, &style, &is_pressed));
  ASSERT_EQ(CMP_BUTTON_STYLE_PLAIN, style);
  ASSERT_EQ(0, is_pressed);

  /* Set Style */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_set_style(ctx, CMP_BUTTON_STYLE_FILLED));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_get_state(ctx, &style, &is_pressed));
  ASSERT_EQ(CMP_BUTTON_STYLE_FILLED, style);

  /* Mouse Down */
  evt.type = 1; /* MOUSE */
  evt.action = CMP_ACTION_DOWN;
  ASSERT_EQ(CMP_SUCCESS, cmp_button_handle_event(ctx, &evt));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_get_state(ctx, NULL, &is_pressed));
  ASSERT_EQ(1, is_pressed);

  /* Update Tick */
  ASSERT_EQ(CMP_SUCCESS, cmp_button_update(ctx, 16.6f));

  /* Mouse Up */
  evt.action = CMP_ACTION_UP;
  ASSERT_EQ(CMP_SUCCESS, cmp_button_handle_event(ctx, &evt));
  ASSERT_EQ(CMP_SUCCESS, cmp_button_get_state(ctx, NULL, &is_pressed));
  ASSERT_EQ(0, is_pressed);

  ASSERT_EQ(CMP_SUCCESS, cmp_button_destroy(ctx));
  PASS();
}

TEST test_toggle_events_and_state(void) {
  cmp_toggle_t *ctx = NULL;
  cmp_event_t evt;
  int is_on;

  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_create(&ctx));

  /* Initial OFF State */
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_get_state(ctx, &is_on));
  ASSERT_EQ(0, is_on);

  /* Programmatic Set ON */
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_set_state(ctx, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_get_state(ctx, &is_on));
  ASSERT_EQ(1, is_on);

  /* Click Event toggles it OFF */
  evt.type = 1;
  evt.action = CMP_ACTION_UP; /* Toggle on release typically */
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_handle_event(ctx, &evt));
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_get_state(ctx, &is_on));
  ASSERT_EQ(0, is_on);

  /* Update Tick */
  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_update(ctx, 16.6f));

  ASSERT_EQ(CMP_SUCCESS, cmp_toggle_destroy(ctx));
  PASS();
}

TEST test_null_args(void) {
  cmp_event_t evt;
  evt.type = 1;
  evt.action = CMP_ACTION_DOWN;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_button_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_button_set_style(NULL, CMP_BUTTON_STYLE_PLAIN));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_button_handle_event(NULL, &evt));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_button_update(NULL, 16.6f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_button_get_state(NULL, NULL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toggle_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toggle_set_state(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toggle_handle_event(NULL, &evt));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toggle_update(NULL, 16.6f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_toggle_get_state(NULL, NULL));
  PASS();
}

SUITE(system_button_suite) {
  RUN_TEST(test_button_events_and_state);
  RUN_TEST(test_toggle_events_and_state);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(system_button_suite);
  GREATEST_MAIN_END();
}