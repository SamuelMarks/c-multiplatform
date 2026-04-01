/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_pointer_morphing(void) {
  cmp_pointer_region_t *ctx = NULL;
  float scale_x, scale_y;

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_region_create(&ctx));

  /* LIFT: Simulates pointer hiding and button scaling up slightly */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_pointer_region_set_style(ctx, CMP_POINTER_INTERACTION_LIFT));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_pointer_region_get_morph_scale(ctx, &scale_x, &scale_y));
  ASSERT_EQ(1.05f, scale_x);

  /* HIGHLIGHT: Simulates pointer snapping to button size, element stays same
   * size */
  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_region_set_style(
                             ctx, CMP_POINTER_INTERACTION_HIGHLIGHT));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_pointer_region_get_morph_scale(ctx, &scale_x, &scale_y));
  ASSERT_EQ(1.0f, scale_x);

  ASSERT_EQ(CMP_SUCCESS, cmp_pointer_region_destroy(ctx));
  PASS();
}

TEST test_keyboard_shortcuts(void) {
  cmp_ui_command_t *cmd = NULL;
  cmp_keyboard_shortcut_t *shortcut = NULL;

  /* Cmd-C mapped to 'copy' action */
  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_shortcut_create(
                             &shortcut, 'c', 1 << 0 /* Command flag */));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_command_create(&cmd, "Copy", "action.copy"));

  /* Ownership passes */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_command_set_shortcut(cmd, shortcut));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_command_destroy(cmd));
  PASS();
}

TEST test_key_repeat(void) {
  float interval;

  /* Just pressed, no repeat yet */
  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_calculate_key_repeat(100.0f, &interval));
  ASSERT_EQ(0.0f, interval);

  /* Passed threshold, repeating slowly */
  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_calculate_key_repeat(500.0f, &interval));
  ASSERT_EQ(100.0f, interval);

  /* Held for a while, repeating faster */
  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_calculate_key_repeat(1000.0f, &interval));
  ASSERT_EQ(60.0f, interval);

  /* Max speed */
  ASSERT_EQ(CMP_SUCCESS, cmp_keyboard_calculate_key_repeat(3000.0f, &interval));
  ASSERT_EQ(20.0f, interval);

  PASS();
}

TEST test_trackpad_gestures(void) {
  float px, py;
  ASSERT_EQ(CMP_SUCCESS, cmp_trackpad_evaluate_gesture(-50.0f, 0.0f, &px, &py));
  ASSERT_EQ(-50.0f, px); /* Pan left */
  PASS();
}

TEST test_null_args(void) {
  cmp_pointer_region_t *pr = NULL;
  cmp_keyboard_shortcut_t *ks = NULL;
  cmp_ui_command_t *cmd = NULL;
  float f;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pointer_region_create(NULL));
  cmp_pointer_region_create(&pr);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pointer_region_set_style(NULL, CMP_POINTER_INTERACTION_LIFT));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pointer_region_get_morph_scale(NULL, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pointer_region_get_morph_scale(pr, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pointer_region_get_morph_scale(pr, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keyboard_shortcut_create(NULL, 'a', 0));
  cmp_keyboard_shortcut_create(&ks, 'a', 0);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_command_create(NULL, "T", "I"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_command_create(&cmd, NULL, "I"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_command_create(&cmd, "T", NULL));
  cmp_ui_command_create(&cmd, "T", "I");

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_command_set_shortcut(NULL, ks));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_command_set_shortcut(cmd, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_keyboard_calculate_key_repeat(100.0f, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_trackpad_evaluate_gesture(1.0f, 1.0f, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_trackpad_evaluate_gesture(1.0f, 1.0f, &f, NULL));

  cmp_pointer_region_destroy(pr);
  cmp_ui_command_destroy(cmd);
  cmp_keyboard_shortcut_destroy(ks);
  PASS();
}

SUITE(keyboard_hig_suite) {
  RUN_TEST(test_pointer_morphing);
  RUN_TEST(test_keyboard_shortcuts);
  RUN_TEST(test_key_repeat);
  RUN_TEST(test_trackpad_gestures);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(keyboard_hig_suite);
  GREATEST_MAIN_END();
}
