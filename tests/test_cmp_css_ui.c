/* clang-format off */
#include "greatest.h"
#include "cmp_css_ui.h"
/* clang-format on */

TEST test_cursor_init_free(void) {
  cmp_prop_cursor_t prop;
  ASSERT_EQ(0, cmp_prop_cursor_init(&prop, "pointer"));
  ASSERT_STR_EQ("pointer", prop.cursor);
  ASSERT_EQ(0, cmp_prop_cursor_free(&prop));
  ASSERT_EQ(NULL, prop.cursor);
  PASS();
}

TEST test_pointer_touch_group_init_free(void) {
  cmp_prop_pointer_touch_group_t group;
  ASSERT_EQ(0,
            cmp_prop_pointer_touch_group_init(&group, "none", "manipulation"));
  ASSERT_STR_EQ("none", group.pointer_events);
  ASSERT_STR_EQ("manipulation", group.touch_action);
  ASSERT_EQ(0, cmp_prop_pointer_touch_group_free(&group));
  ASSERT_EQ(NULL, group.pointer_events);
  ASSERT_EQ(NULL, group.touch_action);
  PASS();
}

TEST test_user_select_init(void) {
  cmp_prop_user_select_t prop;
  ASSERT_EQ(0, cmp_prop_user_select_init(&prop, CMP_USER_SELECT_NONE));
  ASSERT_EQ(CMP_USER_SELECT_NONE, prop.select);
  PASS();
}

TEST test_appearance_init_free(void) {
  cmp_prop_appearance_t prop;
  ASSERT_EQ(0, cmp_prop_appearance_init(&prop, "button"));
  ASSERT_STR_EQ("button", prop.appearance);
  ASSERT_EQ(0, cmp_prop_appearance_free(&prop));
  ASSERT_EQ(NULL, prop.appearance);
  PASS();
}

TEST test_resize_init(void) {
  cmp_prop_resize_t prop;
  ASSERT_EQ(0, cmp_prop_resize_init(&prop, CMP_RESIZE_BOTH));
  ASSERT_EQ(CMP_RESIZE_BOTH, prop.resize);
  PASS();
}

TEST test_zoom_init_free(void) {
  cmp_prop_zoom_t prop;
  ASSERT_EQ(0, cmp_prop_zoom_init(&prop, "150%"));
  ASSERT_STR_EQ("150%", prop.zoom);
  ASSERT_EQ(0, cmp_prop_zoom_free(&prop));
  ASSERT_EQ(NULL, prop.zoom);
  PASS();
}

TEST test_caret_color_init_free(void) {
  cmp_prop_caret_color_t prop;
  cmp_prop_color_t color;
  color.type = CMP_PROP_COLOR_LEGACY;
  color.value.legacy.r = 255;
  color.value.legacy.g = 0;
  color.value.legacy.b = 0;
  color.value.legacy.a = 1.0f;

  ASSERT_EQ(0, cmp_prop_caret_color_init(&prop, &color));
  ASSERT_EQ(CMP_PROP_COLOR_LEGACY, prop.color.type);
  ASSERT_EQ(255, prop.color.value.legacy.r);

  ASSERT_EQ(0, cmp_prop_caret_color_free(&prop));
  PASS();
}

TEST test_caret_shape_init(void) {
  cmp_prop_caret_shape_t prop;
  ASSERT_EQ(0, cmp_prop_caret_shape_init(&prop, CMP_CARET_SHAPE_BLOCK));
  ASSERT_EQ(CMP_CARET_SHAPE_BLOCK, prop.shape);
  PASS();
}

TEST test_accent_color_init_free(void) {
  cmp_prop_accent_color_t prop;
  ASSERT_EQ(0, cmp_prop_accent_color_init(&prop, NULL));
  ASSERT_EQ(CMP_PROP_COLOR_CURRENTCOLOR, prop.color.type);

  ASSERT_EQ(0, cmp_prop_accent_color_free(&prop));
  PASS();
}

TEST test_spatial_nav_init_free(void) {
  cmp_prop_spatial_nav_t group;
  ASSERT_EQ(0,
            cmp_prop_spatial_nav_init(&group, "auto", "#id1", "none", "auto"));
  ASSERT_STR_EQ("auto", group.nav_up);
  ASSERT_STR_EQ("#id1", group.nav_down);
  ASSERT_STR_EQ("none", group.nav_left);
  ASSERT_STR_EQ("auto", group.nav_right);

  ASSERT_EQ(0, cmp_prop_spatial_nav_free(&group));
  ASSERT_EQ(NULL, group.nav_up);
  ASSERT_EQ(NULL, group.nav_down);
  PASS();
}

SUITE(cmp_css_ui_suite) {
  RUN_TEST(test_cursor_init_free);
  RUN_TEST(test_pointer_touch_group_init_free);
  RUN_TEST(test_user_select_init);
  RUN_TEST(test_appearance_init_free);
  RUN_TEST(test_resize_init);
  RUN_TEST(test_zoom_init_free);
  RUN_TEST(test_caret_color_init_free);
  RUN_TEST(test_caret_shape_init);
  RUN_TEST(test_accent_color_init_free);
  RUN_TEST(test_spatial_nav_init_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_ui_suite);
  GREATEST_MAIN_END();
}
