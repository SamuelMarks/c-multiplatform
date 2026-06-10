/* clang-format off */
#include "cmp_css_animations.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_transition_group(void) {
  cmp_prop_transition_group_t group;

  ASSERT_EQ(-1, cmp_prop_transition_group_init(NULL, NULL, NULL, NULL, NULL,
                                               CMP_TRANSITION_BEHAVIOR_NORMAL));

  ASSERT_EQ(0, cmp_prop_transition_group_init(
                   &group, "width", "2s", "linear", "1s",
                   CMP_TRANSITION_BEHAVIOR_ALLOW_DISCRETE));
  ASSERT_STR_EQ("width", group.property);
  ASSERT_STR_EQ("2s", group.duration);
  ASSERT_STR_EQ("linear", group.timing_function);
  ASSERT_STR_EQ("1s", group.delay);
  ASSERT_EQ(CMP_TRANSITION_BEHAVIOR_ALLOW_DISCRETE, group.behavior);

  ASSERT_EQ(0, cmp_prop_transition_group_free(&group));
  ASSERT_EQ(NULL, group.property);
  ASSERT_EQ(NULL, group.duration);
  ASSERT_EQ(NULL, group.timing_function);
  ASSERT_EQ(NULL, group.delay);

  /* Partial init */
  ASSERT_EQ(0,
            cmp_prop_transition_group_init(&group, "color", "500ms", NULL, NULL,
                                           CMP_TRANSITION_BEHAVIOR_NORMAL));
  ASSERT_STR_EQ("color", group.property);
  ASSERT_STR_EQ("500ms", group.duration);
  ASSERT_EQ(NULL, group.timing_function);
  ASSERT_EQ(NULL, group.delay);
  ASSERT_EQ(0, cmp_prop_transition_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_transition_group_free(NULL));

  PASS();
}

TEST test_animation_group(void) {
  cmp_prop_animation_group_t group;

  ASSERT_EQ(
      -1, cmp_prop_animation_group_init(
              NULL, NULL, NULL, NULL, NULL, CMP_ANIMATION_DIRECTION_NORMAL,
              CMP_ANIMATION_PLAY_STATE_RUNNING, NULL,
              CMP_ANIMATION_FILL_MODE_NONE, CMP_ANIMATION_COMPOSITION_REPLACE));

  ASSERT_EQ(0, cmp_prop_animation_group_init(
                   &group, "slidein", "3s", "ease-in-out", "infinite",
                   CMP_ANIMATION_DIRECTION_ALTERNATE,
                   CMP_ANIMATION_PLAY_STATE_PAUSED, "0.5s",
                   CMP_ANIMATION_FILL_MODE_FORWARDS,
                   CMP_ANIMATION_COMPOSITION_ADD));
  ASSERT_STR_EQ("slidein", group.name);
  ASSERT_STR_EQ("3s", group.duration);
  ASSERT_STR_EQ("ease-in-out", group.timing_function);
  ASSERT_STR_EQ("infinite", group.iteration_count);
  ASSERT_EQ(CMP_ANIMATION_DIRECTION_ALTERNATE, group.direction);
  ASSERT_EQ(CMP_ANIMATION_PLAY_STATE_PAUSED, group.play_state);
  ASSERT_STR_EQ("0.5s", group.delay);
  ASSERT_EQ(CMP_ANIMATION_FILL_MODE_FORWARDS, group.fill_mode);
  ASSERT_EQ(CMP_ANIMATION_COMPOSITION_ADD, group.composition);

  ASSERT_EQ(0, cmp_prop_animation_group_free(&group));
  ASSERT_EQ(NULL, group.name);
  ASSERT_EQ(NULL, group.duration);
  ASSERT_EQ(NULL, group.timing_function);
  ASSERT_EQ(NULL, group.iteration_count);
  ASSERT_EQ(NULL, group.delay);

  /* Partial init */
  ASSERT_EQ(
      0, cmp_prop_animation_group_init(
             &group, "fadeout", NULL, NULL, "1", CMP_ANIMATION_DIRECTION_NORMAL,
             CMP_ANIMATION_PLAY_STATE_RUNNING, NULL,
             CMP_ANIMATION_FILL_MODE_NONE, CMP_ANIMATION_COMPOSITION_REPLACE));
  ASSERT_STR_EQ("fadeout", group.name);
  ASSERT_STR_EQ("1", group.iteration_count);
  ASSERT_EQ(NULL, group.duration);
  ASSERT_EQ(0, cmp_prop_animation_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_animation_group_free(NULL));

  PASS();
}

TEST test_at_rule_keyframes(void) {
  cmp_at_rule_keyframes_t rule;

  ASSERT_EQ(-1, cmp_at_rule_keyframes_init(NULL, NULL, NULL));

  ASSERT_EQ(0, cmp_at_rule_keyframes_init(
                   &rule, "bounce", "0% { top: 0; } 100% { top: 100px; }"));
  ASSERT_STR_EQ("bounce", rule.name);
  ASSERT_STR_EQ("0% { top: 0; } 100% { top: 100px; }", rule.content);

  ASSERT_EQ(0, cmp_at_rule_keyframes_free(&rule));
  ASSERT_EQ(NULL, rule.name);
  ASSERT_EQ(NULL, rule.content);

  ASSERT_EQ(0, cmp_at_rule_keyframes_init(&rule, NULL, "50% { opacity: 0; }"));
  ASSERT_EQ(NULL, rule.name);
  ASSERT_STR_EQ("50% { opacity: 0; }", rule.content);
  ASSERT_EQ(0, cmp_at_rule_keyframes_free(&rule));

  ASSERT_EQ(-1, cmp_at_rule_keyframes_free(NULL));

  PASS();
}

TEST test_at_rule_starting_style(void) {
  cmp_at_rule_starting_style_t rule;

  ASSERT_EQ(-1, cmp_at_rule_starting_style_init(NULL, NULL));

  ASSERT_EQ(0, cmp_at_rule_starting_style_init(
                   &rule, "opacity: 0; transform: scale(0.5);"));
  ASSERT_STR_EQ("opacity: 0; transform: scale(0.5);", rule.content);

  ASSERT_EQ(0, cmp_at_rule_starting_style_free(&rule));
  ASSERT_EQ(NULL, rule.content);

  ASSERT_EQ(0, cmp_at_rule_starting_style_init(&rule, NULL));
  ASSERT_EQ(NULL, rule.content);
  ASSERT_EQ(0, cmp_at_rule_starting_style_free(&rule));

  ASSERT_EQ(-1, cmp_at_rule_starting_style_free(NULL));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_transition_group);
  RUN_TEST(test_animation_group);
  RUN_TEST(test_at_rule_keyframes);
  RUN_TEST(test_at_rule_starting_style);
  GREATEST_MAIN_END();
}