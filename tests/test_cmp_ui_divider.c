/* clang-format off */
#include "cmp.h"
#include "cmp_ui_divider.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_divider_suite);

TEST test_ui_divider_lifecycle(void) {
  cmp_ui_divider_t *divider = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_divider_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_divider_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_divider_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_divider_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_divider_create(&divider));
  ASSERT_NEQ(NULL, divider);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_divider_get_node(divider, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_divider_get_node(divider, &node));
  ASSERT_NEQ(NULL, node);

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_divider_bind_a11y(divider, tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_divider_destroy(divider));

  PASS();
}

SUITE(cmp_ui_divider_suite) { RUN_TEST(test_ui_divider_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_divider_suite);
  GREATEST_MAIN_END();
}