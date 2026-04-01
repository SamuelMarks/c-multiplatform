/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_gamepad_navigation(void) {
  cmp_gamepad_t pad = {0};

  /* Sending dummy gamepad state */
  ASSERT_EQ(CMP_SUCCESS, cmp_gamepad_evaluate_focus_navigation(&pad, 16.0f));

  PASS();
}

TEST test_gamepad_rumble(void) {
  ASSERT_EQ(CMP_SUCCESS,
            cmp_gamepad_trigger_rumble(0, CMP_GAMEPAD_RUMBLE_MEDIUM));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_gamepad_trigger_rumble(1, CMP_GAMEPAD_RUMBLE_SUCCESS));

  /* Invalid index */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gamepad_trigger_rumble(-1, CMP_GAMEPAD_RUMBLE_HEAVY));
  PASS();
}

TEST test_gamepad_adaptive_triggers(void) {
  cmp_adaptive_trigger_config_t cfg;
  cfg.is_active = 1;
  cfg.start_position = 0.5f;
  cfg.resistance = 0.8f;

  ASSERT_EQ(CMP_SUCCESS, cmp_gamepad_set_adaptive_trigger(0, 1 /* L2 */, &cfg));
  ASSERT_EQ(CMP_SUCCESS, cmp_gamepad_set_adaptive_trigger(0, 0 /* R2 */, &cfg));

  /* Invalid args */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gamepad_set_adaptive_trigger(-1, 1, &cfg));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gamepad_set_adaptive_trigger(0, 1, NULL));
  PASS();
}

TEST test_null_args(void) {
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_gamepad_evaluate_focus_navigation(NULL, 16.0f));
  PASS();
}

SUITE(gamepad_hig_suite) {
  RUN_TEST(test_gamepad_navigation);
  RUN_TEST(test_gamepad_rumble);
  RUN_TEST(test_gamepad_adaptive_triggers);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(gamepad_hig_suite);
  GREATEST_MAIN_END();
}
