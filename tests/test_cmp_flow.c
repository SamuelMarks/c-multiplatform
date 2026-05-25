/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_flow_suite);

TEST test_bfc_calculate(void) {
  cmp_layout_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  node->computed_rect.width = 100.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_bfc_calculate(node, 500.0f));
  ASSERT_EQ(500.0f, node->computed_rect.width);

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_destroy(node));
  PASS();
}

TEST test_ifc_calculate(void) {
  cmp_layout_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  node->computed_rect.width = 600.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ifc_calculate(node, 500.0f));
  ASSERT_EQ(500.0f, node->computed_rect.width);

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_destroy(node));
  PASS();
}

TEST test_float_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  float x = 10.0f, y = 20.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  node->computed_rect.width = 100.0f;
  node->computed_rect.height = 50.0f;

  ASSERT_EQ(CMP_SUCCESS, cmp_float_evaluate(node, 1, 0, &x, &y));
  ASSERT_EQ(100.0f, x);
  ASSERT_EQ(0.0f, y);

  x = 10.0f;
  y = 20.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_float_evaluate(node, 0, 1, &x, &y));
  ASSERT_EQ(0.0f, x);
  ASSERT_EQ(70.0f, y); /* 20.0 + 50.0 */

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_destroy(node));
  PASS();
}

TEST test_shape_outside_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_t float_rect = {0.0f, 0.0f, 50.0f, 50.0f};
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  /* Test 1: Rectangular wrapping */
  node->computed_rect.y = 10.0f;
  node->computed_rect.height = 10.0f;
  node->computed_rect.width = 200.0f;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shape_outside_evaluate(node, float_rect, 0.0f, 10.0f));
  ASSERT_EQ(140.0f, node->computed_rect.width); /* 200 - (50 + 10) */

  /* Test 2: Circular wrapping (intersects exactly at center y) */
  node->computed_rect.y = 25.0f;
  node->computed_rect.height = 0.0f;
  node->computed_rect.width = 200.0f;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_shape_outside_evaluate(node, float_rect, 25.0f, 5.0f));
  ASSERT_EQ(170.0f, node->computed_rect.width); /* 200 - (25 + 5) since y_dist =
                                                   0 -> x_dist = 25 - 0 = 25 */

  /* Test 3: Outside boundaries */
  node->computed_rect.y = 100.0f;
  node->computed_rect.height = 10.0f;
  node->computed_rect.width = 200.0f;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_shape_outside_evaluate(node, float_rect, 0.0f, 10.0f));
  ASSERT_EQ(200.0f, node->computed_rect.width); /* No intersection */

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_destroy(node));
  PASS();
}

TEST test_multicolumn_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  node->computed_rect.width = 320.0f;
  node->column_count = 3;
  node->column_gap = 10.0f;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_multicolumn_evaluate(node, CMP_COLUMN_FILL_BALANCE));
  ASSERT_EQ(100.0f, node->column_width); /* (320 - (2 * 10)) / 3 = 100 */

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_destroy(node));
  PASS();
}

TEST test_table_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  ASSERT_EQ(CMP_SUCCESS, cmp_table_evaluate(node, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_table_evaluate(node, 0));

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_destroy(node));
  PASS();
}

TEST test_table_border_collapse(void) {
  cmp_layout_node_t *node = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_create(&node));

  ASSERT_EQ(CMP_SUCCESS, cmp_table_border_collapse(node));

  ASSERT_EQ(CMP_SUCCESS, cmp_layout_node_destroy(node));
  PASS();
}

SUITE(cmp_flow_suite) {
  RUN_TEST(test_bfc_calculate);
  RUN_TEST(test_ifc_calculate);
  RUN_TEST(test_float_evaluate);
  RUN_TEST(test_shape_outside_evaluate);
  RUN_TEST(test_multicolumn_evaluate);
  RUN_TEST(test_table_evaluate);
  RUN_TEST(test_table_border_collapse);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_flow_suite);
  GREATEST_MAIN_END();
}
