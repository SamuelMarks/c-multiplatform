/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_system_geometry(void) {
  cmp_system_geometry_t *ctx = NULL;
  cmp_rect_t safe_area;
  float leading, trailing, max_w;

  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_create(&ctx));

  /* Safe Areas (Mobile) */
  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_get_safe_area(ctx, 0, &safe_area));
  ASSERT_EQ(47.0f, safe_area.y);      /* Notch/Island */
  ASSERT_EQ(34.0f, safe_area.height); /* Home Indicator */
  ASSERT_EQ(0.0f, safe_area.x);

  /* Safe Areas (tvOS Overscan) */
  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_get_safe_area(ctx, 1, &safe_area));
  ASSERT_EQ(90.0f, safe_area.x);
  ASSERT_EQ(60.0f, safe_area.y);

  /* Standard Margins */
  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_get_layout_margins(
                             ctx, 1, &leading, &trailing));
  ASSERT_EQ(16.0f, leading); /* Compact */

  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_get_layout_margins(
                             ctx, 0, &leading, &trailing));
  ASSERT_EQ(20.0f, leading); /* Regular */

  /* Readable Content Guide */
  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_get_readable_content_guide(
                             ctx, 1024.0f, &max_w));
  ASSERT_EQ(672.0f, max_w); /* Caps out on iPad */

  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_get_readable_content_guide(
                             ctx, 375.0f, &max_w));
  ASSERT_EQ(335.0f, max_w); /* iPhone scales down (375 - 40) */

  ASSERT_EQ(CMP_SUCCESS, cmp_system_geometry_destroy(ctx));
  PASS();
}

TEST test_layout_flags(void) {
  cmp_layout_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_set_corner_curve_continuous(node, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_enforce_pixel_alignment(node, 1));

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_null_args(void) {
  cmp_system_geometry_t *ctx = NULL;
  cmp_rect_t r;
  float f;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_geometry_create(NULL));
  cmp_system_geometry_create(&ctx);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_layout_set_corner_curve_continuous(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_layout_enforce_pixel_alignment(NULL, 1));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_geometry_get_safe_area(NULL, 0, &r));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_geometry_get_safe_area(ctx, 0, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_geometry_get_layout_margins(NULL, 0, &f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_geometry_get_layout_margins(ctx, 0, NULL, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_geometry_get_layout_margins(ctx, 0, &f, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_geometry_get_readable_content_guide(NULL, 0.0f, &f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_system_geometry_get_readable_content_guide(ctx, 0.0f, NULL));

  cmp_system_geometry_destroy(ctx);
  PASS();
}

SUITE(geometry_suite) {
  RUN_TEST(test_system_geometry);
  RUN_TEST(test_layout_flags);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(geometry_suite);
  GREATEST_MAIN_END();
}
