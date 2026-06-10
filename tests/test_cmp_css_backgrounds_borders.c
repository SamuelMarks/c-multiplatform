/* clang-format off */
#include "cmp_css_backgrounds_borders.h"
#include "greatest.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_bg_group(void) {
  cmp_prop_bg_group_t bg;

  ASSERT_EQ(-1, cmp_prop_bg_group_init(NULL));
  ASSERT_EQ(0, cmp_prop_bg_group_init(&bg));

  ASSERT_EQ(CMP_BG_REPEAT_REPEAT, bg.repeat_x);
  ASSERT_EQ(CMP_BG_ATTACHMENT_SCROLL, bg.attachment);
  ASSERT_EQ(CMP_PROP_SIZE_AUTO, bg.position_x.type);
  ASSERT_EQ(CMP_BG_BOX_BORDER_BOX, bg.clip);

  ASSERT_EQ(-1, cmp_prop_bg_group_free(NULL));
  ASSERT_EQ(0, cmp_prop_bg_group_free(&bg));

  PASS();
}

TEST test_border_group(void) {
  cmp_prop_border_group_t border;

  ASSERT_EQ(-1, cmp_prop_border_group_init(NULL));
  ASSERT_EQ(0, cmp_prop_border_group_init(&border));

  ASSERT_EQ(CMP_BORDER_STYLE_NONE, border.style);
  ASSERT_EQ(CMP_PROP_SIZE_LENGTH_PHYS, border.width.type);

  ASSERT_EQ(-1, cmp_prop_border_group_free(NULL));
  ASSERT_EQ(0, cmp_prop_border_group_free(&border));

  PASS();
}

TEST test_border_radius(void) {
  cmp_prop_border_radius_t radius;

  ASSERT_EQ(-1, cmp_prop_border_radius_init(NULL));
  ASSERT_EQ(0, cmp_prop_border_radius_init(&radius));

  ASSERT_EQ(CMP_PROP_SIZE_LENGTH_PHYS, radius.top_left_x.type);
  ASSERT_EQ(0.0f, radius.bottom_right_y.value.phys.value);

  PASS();
}

TEST test_border_image(void) {
  cmp_prop_border_image_group_t img;

  ASSERT_EQ(-1, cmp_prop_border_image_group_init(NULL));
  ASSERT_EQ(0, cmp_prop_border_image_group_init(&img));

  ASSERT_EQ(CMP_BORDER_IMAGE_REPEAT_STRETCH, img.repeat_x);
  ASSERT_EQ(CMP_BORDER_IMAGE_REPEAT_STRETCH, img.repeat_y);

  PASS();
}

TEST test_box_shadow(void) {
  cmp_prop_box_shadow_t shadow;

  ASSERT_EQ(-1, cmp_prop_box_shadow_init(NULL));
  ASSERT_EQ(0, cmp_prop_box_shadow_init(&shadow));

  ASSERT_EQ(0, shadow.inset);
  ASSERT_EQ(CMP_PROP_SIZE_LENGTH_PHYS, shadow.offset_x.type);

  ASSERT_EQ(-1, cmp_prop_box_shadow_free(NULL));
  ASSERT_EQ(0, cmp_prop_box_shadow_free(&shadow));

  PASS();
}

TEST test_outline(void) {
  cmp_prop_outline_group_t outline;

  ASSERT_EQ(-1, cmp_prop_outline_group_init(NULL));
  ASSERT_EQ(0, cmp_prop_outline_group_init(&outline));

  ASSERT_EQ(CMP_BORDER_STYLE_NONE, outline.style);
  ASSERT_EQ(CMP_PROP_SIZE_LENGTH_PHYS, outline.width.type);

  ASSERT_EQ(-1, cmp_prop_outline_group_free(NULL));
  ASSERT_EQ(0, cmp_prop_outline_group_free(&outline));

  PASS();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_TEST(test_bg_group);
  RUN_TEST(test_border_group);
  RUN_TEST(test_border_radius);
  RUN_TEST(test_border_image);
  RUN_TEST(test_box_shadow);
  RUN_TEST(test_outline);
  GREATEST_MAIN_END();
}