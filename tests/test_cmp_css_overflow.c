/* clang-format off */
#include "greatest.h"
#include "cmp_css_overflow.h"
/* clang-format on */

TEST test_overflow_group_init_free(void) {
  cmp_prop_overflow_group_t group;
  ASSERT_EQ(0, cmp_prop_overflow_group_init(&group, CMP_OVERFLOW_HIDDEN,
                                            CMP_OVERFLOW_AUTO, "10px"));
  ASSERT_EQ(CMP_OVERFLOW_HIDDEN, group.overflow_x);
  ASSERT_EQ(CMP_OVERFLOW_AUTO, group.overflow_y);
  ASSERT_STR_EQ("10px", group.overflow_clip_margin);

  ASSERT_EQ(0, cmp_prop_overflow_group_free(&group));
  ASSERT_EQ(NULL, group.overflow_clip_margin);
  PASS();
}

TEST test_overscroll_init(void) {
  cmp_prop_overscroll_t prop;
  ASSERT_EQ(0, cmp_prop_overscroll_init(&prop, CMP_OVERSCROLL_BEHAVIOR_CONTAIN,
                                        CMP_OVERSCROLL_BEHAVIOR_NONE));
  ASSERT_EQ(CMP_OVERSCROLL_BEHAVIOR_CONTAIN, prop.overscroll_x);
  ASSERT_EQ(CMP_OVERSCROLL_BEHAVIOR_NONE, prop.overscroll_y);
  PASS();
}

TEST test_scroll_behavior_init(void) {
  cmp_prop_scroll_behavior_t prop;
  ASSERT_EQ(0,
            cmp_prop_scroll_behavior_init(&prop, CMP_SCROLL_BEHAVIOR_SMOOTH));
  ASSERT_EQ(CMP_SCROLL_BEHAVIOR_SMOOTH, prop.behavior);
  PASS();
}

TEST test_scroll_snap_group_init_free(void) {
  cmp_prop_scroll_snap_group_t group;
  ASSERT_EQ(0, cmp_prop_scroll_snap_group_init(&group, "x mandatory", "start",
                                               CMP_SCROLL_SNAP_STOP_ALWAYS));
  ASSERT_STR_EQ("x mandatory", group.type.snap_type);
  ASSERT_STR_EQ("start", group.align.snap_align);
  ASSERT_EQ(CMP_SCROLL_SNAP_STOP_ALWAYS, group.stop);

  ASSERT_EQ(0, cmp_prop_scroll_snap_group_free(&group));
  ASSERT_EQ(NULL, group.type.snap_type);
  ASSERT_EQ(NULL, group.align.snap_align);
  PASS();
}

TEST test_scroll_padding_margin_init_free(void) {
  cmp_prop_scroll_padding_margin_t prop;
  ASSERT_EQ(
      0, cmp_prop_scroll_padding_margin_init(&prop, "10px 20px", "5px auto"));
  ASSERT_STR_EQ("10px 20px", prop.scroll_padding);
  ASSERT_STR_EQ("5px auto", prop.scroll_margin);

  ASSERT_EQ(0, cmp_prop_scroll_padding_margin_free(&prop));
  ASSERT_EQ(NULL, prop.scroll_padding);
  ASSERT_EQ(NULL, prop.scroll_margin);
  PASS();
}

TEST test_scrollbar_gutter_init(void) {
  cmp_prop_scrollbar_gutter_t prop;
  ASSERT_EQ(0,
            cmp_prop_scrollbar_gutter_init(&prop, CMP_SCROLLBAR_GUTTER_STABLE));
  ASSERT_EQ(CMP_SCROLLBAR_GUTTER_STABLE, prop.gutter);
  PASS();
}

TEST test_scrollbar_styling_init_free(void) {
  cmp_prop_scrollbar_styling_t prop;
  ASSERT_EQ(0, cmp_prop_scrollbar_styling_init(&prop, "thin", "red blue", 1));
  ASSERT_STR_EQ("thin", prop.scrollbar_width);
  ASSERT_STR_EQ("red blue", prop.scrollbar_color);
  ASSERT_EQ(1, prop.has_webkit_scrollbar);

  ASSERT_EQ(0, cmp_prop_scrollbar_styling_free(&prop));
  ASSERT_EQ(NULL, prop.scrollbar_width);
  ASSERT_EQ(NULL, prop.scrollbar_color);
  PASS();
}

SUITE(cmp_css_overflow_suite) {
  RUN_TEST(test_overflow_group_init_free);
  RUN_TEST(test_overscroll_init);
  RUN_TEST(test_scroll_behavior_init);
  RUN_TEST(test_scroll_snap_group_init_free);
  RUN_TEST(test_scroll_padding_margin_init_free);
  RUN_TEST(test_scrollbar_gutter_init);
  RUN_TEST(test_scrollbar_styling_init_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_overflow_suite);
  GREATEST_MAIN_END();
}
