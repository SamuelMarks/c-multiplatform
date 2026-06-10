/* clang-format off */
#include "greatest.h"
#include "cmp_css_images_shapes.h"
/* clang-format on */

TEST test_object_fit_group_init_free(void) {
  cmp_prop_object_fit_group_t group;
  ASSERT_EQ(0, cmp_prop_object_fit_group_init(&group, CMP_OBJECT_FIT_COVER,
                                              "top right"));
  ASSERT_EQ(CMP_OBJECT_FIT_COVER, group.object_fit);
  ASSERT_STR_EQ("top right", group.object_position);

  ASSERT_EQ(0, cmp_prop_object_fit_group_free(&group));
  ASSERT_EQ(NULL, group.object_position);
  PASS();
}

TEST test_image_rendering_init(void) {
  cmp_prop_image_rendering_t prop;
  ASSERT_EQ(
      0, cmp_prop_image_rendering_init(&prop, CMP_IMAGE_RENDERING_PIXELATED));
  ASSERT_EQ(CMP_IMAGE_RENDERING_PIXELATED, prop.rendering);
  PASS();
}

TEST test_image_resolution_init_free(void) {
  cmp_prop_image_resolution_t prop;
  ASSERT_EQ(0, cmp_prop_image_resolution_init(&prop, "300dpi"));
  ASSERT_STR_EQ("300dpi", prop.resolution);

  ASSERT_EQ(0, cmp_prop_image_resolution_free(&prop));
  ASSERT_EQ(NULL, prop.resolution);
  PASS();
}

TEST test_shape_group_init_free(void) {
  cmp_prop_shape_group_t group;
  ASSERT_EQ(0, cmp_prop_shape_group_init(&group, "circle(50%)", "10px", "0.5"));
  ASSERT_STR_EQ("circle(50%)", group.shape_outside);
  ASSERT_STR_EQ("10px", group.shape_margin);
  ASSERT_STR_EQ("0.5", group.shape_image_threshold);

  ASSERT_EQ(0, cmp_prop_shape_group_free(&group));
  ASSERT_EQ(NULL, group.shape_outside);
  ASSERT_EQ(NULL, group.shape_margin);
  ASSERT_EQ(NULL, group.shape_image_threshold);
  PASS();
}

SUITE(cmp_css_images_shapes_suite) {
  RUN_TEST(test_object_fit_group_init_free);
  RUN_TEST(test_image_rendering_init);
  RUN_TEST(test_image_resolution_init_free);
  RUN_TEST(test_shape_group_init_free);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_css_images_shapes_suite);
  GREATEST_MAIN_END();
}
