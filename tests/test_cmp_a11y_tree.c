/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_a11y_tree_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, tree);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_a11y_tree_null_args(void) {
  int res = cmp_a11y_tree_create(NULL);
  char buf[64];
  cmp_a11y_tree_t *tree = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_add_node(NULL, 1, "button", "Submit");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_get_node_desc(NULL, 1, buf, 64);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_get_node_desc(tree, 1, NULL, 64);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_get_node_desc(tree, 1, buf, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_tree_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;
  char buf[128];

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_add_node(tree, 10, "button", "Submit");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_add_node(tree, 20, NULL, "Content");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_add_node(tree, 30, "dialog", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_get_node_desc(tree, 10, buf, 128);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("button: Submit", buf);

  res = cmp_a11y_tree_get_node_desc(tree, 20, buf, 128);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("unknown: Content", buf);

  res = cmp_a11y_tree_get_node_desc(tree, 30, buf, 128);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("dialog: unnamed", buf);

  res = cmp_a11y_tree_get_node_desc(tree, 999, buf, 128);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  /* Test bounds error */
  res = cmp_a11y_tree_get_node_desc(tree, 10, buf, 5);
  ASSERT_EQ(CMP_ERROR_BOUNDS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_tree_apple_compliance(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_add_node(tree, 42, "button", "Submit");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_label(tree, 42, "Localized Submit");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_hint(tree, 42, "Submits the form");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_value(tree, 42, "75%");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_traits(
      tree, 42, CMP_A11Y_TRAIT_BUTTON | CMP_A11Y_TRAIT_PLAYS_SOUND);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Test null args */
  res = cmp_a11y_tree_set_node_label(NULL, 42, "Label");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_hint(NULL, 42, "Hint");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_value(NULL, 42, "Value");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_traits(NULL, 42, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* Test unfound node */
  res = cmp_a11y_tree_set_node_label(tree, 99, "Label");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_hint(tree, 99, "Hint");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_value(tree, 99, "Value");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_traits(tree, 99, 0);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  /* Test overwriting string with NULL */
  res = cmp_a11y_tree_set_node_label(tree, 42, NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_tree_grouping_focus_rotors_actions(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_add_node(tree, 55, "group", "Group Node");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_grouped(tree, 55, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_add_node_custom_action(tree, 55, "Swipe Right");
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_a11y_tree_add_node_custom_action(tree, 55, "Swipe Left");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_add_node_custom_rotor(tree, 55, "Headings");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_focus_order(tree, 55, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Edge cases */
  res = cmp_a11y_tree_add_node_custom_action(NULL, 55, "Swipe Right");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_add_node_custom_action(tree, 55, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_add_node_custom_rotor(NULL, 55, "Headings");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_add_node_custom_rotor(tree, 55, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_focus_order(NULL, 55, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_grouped(NULL, 55, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* Not found */
  res = cmp_a11y_tree_add_node_custom_action(tree, 999, "Swipe");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_add_node_custom_rotor(tree, 999, "Rotors");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_focus_order(tree, 999, 1);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_grouped(tree, 999, 1);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_tree_braille_audio_pronunciation_touch_announcement(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_add_node(tree, 66, "textfield", "Input");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_braille_input(tree, 66, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_audio_description(
      tree, 66, "https://example.com/audio.mp3");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_pronunciation(tree, 66, "ih-n-p-uh-t");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_direct_touch(tree, 66, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_post_announcement(tree, "Loading complete.");
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Edge cases */
  res = cmp_a11y_tree_set_node_braille_input(NULL, 66, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_audio_description(NULL, 66, "x");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_pronunciation(NULL, 66, "x");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_direct_touch(NULL, 66, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_post_announcement(NULL, "x");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_post_announcement(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* Not found */
  res = cmp_a11y_tree_set_node_braille_input(tree, 999, 1);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_audio_description(tree, 999, "x");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_pronunciation(tree, 999, "x");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_direct_touch(tree, 999, 1);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_tree_cognitive_motion_sound(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_add_node(tree, 77, "button", "Submit");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_switch_control_anchor(tree, 77, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_voice_control_tag(tree, 77, "submit", 5);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_guided_access_disabled(tree, 77, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_cognitive_time_limit(tree, 77, 5000.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_set_node_sound_caption(tree, 77, "Ding!");
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Edge cases */
  res = cmp_a11y_tree_set_node_switch_control_anchor(NULL, 77, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_voice_control_tag(NULL, 77, "submit", 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_guided_access_disabled(NULL, 77, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_cognitive_time_limit(NULL, 77, 5000.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_set_node_sound_caption(NULL, 77, "Ding!");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* Not found */
  res = cmp_a11y_tree_set_node_switch_control_anchor(tree, 999, 1);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_voice_control_tag(tree, 999, "submit", 5);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_guided_access_disabled(tree, 999, 1);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_cognitive_time_limit(tree, 999, 5000.0f);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_a11y_tree_set_node_sound_caption(tree, 999, "Ding!");
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}
TEST test_a11y_theme_semantic_equivalence(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_ui_node_t *node = NULL;
  char buf1[256];
  char buf2[256];

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_button_create(&node, "Submit", -1));

  /* Mock Material 3 generation */
  node->design_language_override = 1;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_a11y_tree_serialize(tree, node, buf1, sizeof(buf1)));

  /* Mock Cupertino generation */
  node->design_language_override = 3;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_a11y_tree_serialize(tree, node, buf2, sizeof(buf2)));

  /* The layout theme should NEVER change the fundamental A11y roles and traits
   * announced to the OS */
  ASSERT_STR_EQ(buf1, buf2);

  cmp_ui_node_destroy(node);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(a11y_tree_suite) {
  RUN_TEST(test_a11y_tree_lifecycle);
  RUN_TEST(test_a11y_tree_null_args);
  RUN_TEST(test_a11y_theme_semantic_equivalence);
  RUN_TEST(test_a11y_tree_operations);
  RUN_TEST(test_a11y_tree_apple_compliance);
  RUN_TEST(test_a11y_tree_grouping_focus_rotors_actions);
  RUN_TEST(test_a11y_tree_braille_audio_pronunciation_touch_announcement);
  RUN_TEST(test_a11y_tree_cognitive_motion_sound);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(a11y_tree_suite);
  GREATEST_MAIN_END();
}
