/**
 * @file test_material2_shape.c
 * @brief Unit tests for Material Design 2 shape definitions.
 */

/* clang-format off */
#include "material2/md2_shape.h"
#include "ui_error.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_shape_get_corner_radius_success(void) {
  float radius = -1.0f;
  ui_error_t rc;

  rc = md2_shape_get_corner_radius(MD2_SHAPE_CATEGORY_SMALL_COMPONENT, &radius);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(4.0f, radius, "%f");

  rc =
      md2_shape_get_corner_radius(MD2_SHAPE_CATEGORY_MEDIUM_COMPONENT, &radius);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(4.0f, radius, "%f");

  rc = md2_shape_get_corner_radius(MD2_SHAPE_CATEGORY_LARGE_COMPONENT, &radius);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(0.0f, radius, "%f");

  PASS();
}

TEST test_md2_shape_get_corner_radius_invalid_args(void) {
  float radius;
  ui_error_t rc;

  rc = md2_shape_get_corner_radius(MD2_SHAPE_CATEGORY_SMALL_COMPONENT, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_shape_get_corner_radius((enum md2_shape_category)999, &radius);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  PASS();
}

SUITE(material2_shape_suite) {
  RUN_TEST(test_md2_shape_get_corner_radius_success);
  RUN_TEST(test_md2_shape_get_corner_radius_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_shape_suite);
  GREATEST_MAIN_END();
}
