#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_layout.h"
/* clang-format on */

TEST test_f2_stack_create(void) {
  cmp_ui_node_t *node = NULL;
  int res;

  /* Horizontal Stack */
  res = cmp_f2_stack_create(&node, 1, 8.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);
  ASSERT(node->layout != NULL);
  ASSERT_EQ(CMP_FLEX_ROW, node->layout->direction);
  ASSERT_EQ(CMP_FLEX_NOWRAP, node->layout->flex_wrap);
  ASSERT_EQ(8.0f, node->layout->column_gap);
  cmp_ui_node_destroy(node);

  /* Vertical Stack */
  res = cmp_f2_stack_create(&node, 0, 16.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);
  ASSERT_EQ(CMP_FLEX_COLUMN, node->layout->direction);
  ASSERT_EQ(CMP_FLEX_NOWRAP, node->layout->flex_wrap);
  ASSERT_EQ(16.0f, node->layout->column_gap);
  cmp_ui_node_destroy(node);

  PASS();
}

TEST test_f2_wrap_create(void) {
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_f2_wrap_create(&node, 4.0f, 8.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);
  ASSERT(node->layout != NULL);
  ASSERT_EQ(CMP_FLEX_ROW, node->layout->direction);
  ASSERT_EQ(CMP_FLEX_WRAP, node->layout->flex_wrap);
  ASSERT_EQ(4.0f, node->layout->column_gap);
  cmp_ui_node_destroy(node);

  PASS();
}

TEST test_f2_grid_create(void) {
  cmp_ui_node_t *node = NULL;
  int res;

  res = cmp_f2_grid_create(&node, 3, 12.0f, 16.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);
  ASSERT(node->layout != NULL);
  ASSERT_EQ(12.0f, node->layout->column_gap);
  /* The underlying cmp_ui_grid_view_create handles the track count logic */
  cmp_ui_node_destroy(node);

  PASS();
}

SUITE(cmp_f2_layout_suite) {
  RUN_TEST(test_f2_stack_create);
  RUN_TEST(test_f2_wrap_create);
  RUN_TEST(test_f2_grid_create);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_layout_suite);
  GREATEST_MAIN_END();
}
