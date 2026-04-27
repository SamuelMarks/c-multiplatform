/* clang-format off */
#include "cmp.h"
#include "cmp_ui_fab.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_fab_suite);

TEST test_ui_fab_lifecycle(void) {
  cmp_ui_fab_t *fab = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_fab_create(NULL, "icon"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_fab_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_fab_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_fab_set_icon(NULL, "new_icon"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_fab_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_fab_create(&fab, "add"));
  ASSERT_NEQ(NULL, fab);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_fab_get_node(fab, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_fab_get_node(fab, &node));
  ASSERT_NEQ(NULL, node);

  /* Set icon tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_fab_set_icon(fab, "edit"));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_fab_set_icon(fab, NULL));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_fab_bind_a11y(fab, tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_fab_destroy(fab));

  PASS();
}

SUITE(cmp_ui_fab_suite) { RUN_TEST(test_ui_fab_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_fab_suite);
  GREATEST_MAIN_END();
}