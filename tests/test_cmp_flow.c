/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_flow_suite);

TEST test_bfc_calculate(void) {
  cmp_layout_node_t *node = NULL;
  cmp_layout_node_create(&node);

  node->computed_rect.width = 100.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_bfc_calculate(node, 500.0f));
  ASSERT_EQ(500.0f, node->computed_rect.width);

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_ifc_calculate(void) {
  cmp_layout_node_t *node = NULL;
  cmp_layout_node_create(&node);

  node->computed_rect.width = 600.0f;
  ASSERT_EQ(CMP_SUCCESS, cmp_ifc_calculate(node, 500.0f));
  ASSERT_EQ(500.0f, node->computed_rect.width);

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_float_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  float x = 10.0f, y = 20.0f;
  cmp_layout_node_create(&node);

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

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_shape_outside_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_t float_rect = {0.0f, 0.0f, 50.0f, 50.0f};
  cmp_layout_node_create(&node);

  node->computed_rect.width = 200.0f;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_shape_outside_evaluate(node, float_rect, 0.0f, 10.0f));
  ASSERT_EQ(140.0f, node->computed_rect.width); /* 200 - (50 + 10) */

  node->computed_rect.width = 200.0f;
  ASSERT_EQ(CMP_SUCCESS,
            cmp_shape_outside_evaluate(node, float_rect, 25.0f, 5.0f));
  ASSERT_EQ(170.0f, node->computed_rect.width); /* 200 - (25 + 5) */

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_multicolumn_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  cmp_layout_node_create(&node);

  node->computed_rect.width = 320.0f;
  node->column_count = 3;
  node->column_gap = 10.0f;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_multicolumn_evaluate(node, CMP_COLUMN_FILL_BALANCE));
  ASSERT_EQ(100.0f, node->column_width); /* (320 - (2 * 10)) / 3 = 100 */

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_table_evaluate(void) {
  cmp_layout_node_t *node = NULL;
  cmp_layout_node_create(&node);

  ASSERT_EQ(CMP_SUCCESS, cmp_table_evaluate(node, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_table_evaluate(node, 0));

  cmp_layout_node_destroy(node);
  PASS();
}

TEST test_table_border_collapse(void) {
  cmp_layout_node_t *node = NULL;
  cmp_layout_node_create(&node);

  ASSERT_EQ(CMP_SUCCESS, cmp_table_border_collapse(node));

  cmp_layout_node_destroy(node);
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
