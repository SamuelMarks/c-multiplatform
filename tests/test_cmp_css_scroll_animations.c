/* clang-format off */
#include "cmp_css_scroll_animations.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_scroll_anim_group(void) {
  cmp_prop_scroll_anim_group_t group;

  ASSERT_EQ(-1, cmp_prop_scroll_anim_group_init(NULL, NULL, NULL, NULL));

  ASSERT_EQ(0, cmp_prop_scroll_anim_group_init(&group, "--my-timeline",
                                               "cover 0%", "exit 100%"));
  ASSERT_STR_EQ("--my-timeline", group.timeline);
  ASSERT_STR_EQ("cover 0%", group.range_start);
  ASSERT_STR_EQ("exit 100%", group.range_end);

  ASSERT_EQ(0, cmp_prop_scroll_anim_group_free(&group));
  ASSERT_EQ(NULL, group.timeline);
  ASSERT_EQ(NULL, group.range_start);
  ASSERT_EQ(NULL, group.range_end);

  /* Partial init */
  ASSERT_EQ(0, cmp_prop_scroll_anim_group_init(&group, "none", NULL, NULL));
  ASSERT_STR_EQ("none", group.timeline);
  ASSERT_EQ(NULL, group.range_start);
  ASSERT_EQ(NULL, group.range_end);
  ASSERT_EQ(0, cmp_prop_scroll_anim_group_free(&group));

  ASSERT_EQ(-1, cmp_prop_scroll_anim_group_free(NULL));

  PASS();
}

TEST test_scroll_timeline(void) {
  cmp_prop_scroll_timeline_t group;

  ASSERT_EQ(-1, cmp_prop_scroll_timeline_init(NULL, NULL,
                                              CMP_SCROLL_TIMELINE_AXIS_BLOCK));

  ASSERT_EQ(0, cmp_prop_scroll_timeline_init(&group, "--my-scroll-timeline",
                                             CMP_SCROLL_TIMELINE_AXIS_INLINE));
  ASSERT_STR_EQ("--my-scroll-timeline", group.name);
  ASSERT_EQ(CMP_SCROLL_TIMELINE_AXIS_INLINE, group.axis);

  ASSERT_EQ(0, cmp_prop_scroll_timeline_free(&group));
  ASSERT_EQ(NULL, group.name);

  /* Partial init */
  ASSERT_EQ(0, cmp_prop_scroll_timeline_init(&group, NULL,
                                             CMP_SCROLL_TIMELINE_AXIS_BLOCK));
  ASSERT_EQ(NULL, group.name);
  ASSERT_EQ(CMP_SCROLL_TIMELINE_AXIS_BLOCK, group.axis);
  ASSERT_EQ(0, cmp_prop_scroll_timeline_free(&group));

  ASSERT_EQ(-1, cmp_prop_scroll_timeline_free(NULL));

  PASS();
}

TEST test_view_timeline(void) {
  cmp_prop_view_timeline_t group;

  ASSERT_EQ(-1, cmp_prop_view_timeline_init(
                    NULL, NULL, CMP_SCROLL_TIMELINE_AXIS_BLOCK, NULL));

  ASSERT_EQ(0,
            cmp_prop_view_timeline_init(&group, "--my-view-timeline",
                                        CMP_SCROLL_TIMELINE_AXIS_Y, "10% 20%"));
  ASSERT_STR_EQ("--my-view-timeline", group.name);
  ASSERT_EQ(CMP_SCROLL_TIMELINE_AXIS_Y, group.axis);
  ASSERT_STR_EQ("10% 20%", group.inset);

  ASSERT_EQ(0, cmp_prop_view_timeline_free(&group));
  ASSERT_EQ(NULL, group.name);
  ASSERT_EQ(NULL, group.inset);

  /* Partial init */
  ASSERT_EQ(0, cmp_prop_view_timeline_init(&group, "none",
                                           CMP_SCROLL_TIMELINE_AXIS_X, NULL));
  ASSERT_STR_EQ("none", group.name);
  ASSERT_EQ(CMP_SCROLL_TIMELINE_AXIS_X, group.axis);
  ASSERT_EQ(NULL, group.inset);
  ASSERT_EQ(0, cmp_prop_view_timeline_free(&group));

  ASSERT_EQ(-1, cmp_prop_view_timeline_free(NULL));

  PASS();
}

TEST test_timeline_scope(void) {
  cmp_prop_timeline_scope_t scope_prop;

  ASSERT_EQ(-1, cmp_prop_timeline_scope_init(NULL, NULL));

  ASSERT_EQ(0, cmp_prop_timeline_scope_init(&scope_prop, "--my-timeline"));
  ASSERT_STR_EQ("--my-timeline", scope_prop.scope);

  ASSERT_EQ(0, cmp_prop_timeline_scope_free(&scope_prop));
  ASSERT_EQ(NULL, scope_prop.scope);

  ASSERT_EQ(0, cmp_prop_timeline_scope_init(&scope_prop, NULL));
  ASSERT_EQ(NULL, scope_prop.scope);
  ASSERT_EQ(0, cmp_prop_timeline_scope_free(&scope_prop));

  ASSERT_EQ(-1, cmp_prop_timeline_scope_free(NULL));

  PASS();
}

TEST test_view_transition_name(void) {
  cmp_prop_view_transition_name_t name_prop;

  ASSERT_EQ(-1, cmp_prop_view_transition_name_init(NULL, NULL));

  ASSERT_EQ(0, cmp_prop_view_transition_name_init(&name_prop, "hero-image"));
  ASSERT_STR_EQ("hero-image", name_prop.name);

  ASSERT_EQ(0, cmp_prop_view_transition_name_free(&name_prop));
  ASSERT_EQ(NULL, name_prop.name);

  ASSERT_EQ(0, cmp_prop_view_transition_name_init(&name_prop, NULL));
  ASSERT_EQ(NULL, name_prop.name);
  ASSERT_EQ(0, cmp_prop_view_transition_name_free(&name_prop));

  ASSERT_EQ(-1, cmp_prop_view_transition_name_free(NULL));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_scroll_anim_group);
  RUN_TEST(test_scroll_timeline);
  RUN_TEST(test_view_timeline);
  RUN_TEST(test_timeline_scope);
  RUN_TEST(test_view_transition_name);
  GREATEST_MAIN_END();
}