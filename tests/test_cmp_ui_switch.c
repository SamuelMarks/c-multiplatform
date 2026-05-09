/* clang-format off */
#include "cmp.h"
#include "cmp_ui_switch.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_switch_suite);

TEST test_ui_switch_lifecycle(void) {
  cmp_ui_switch_t *sw = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_switch_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_switch_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_switch_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_switch_set_on(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_switch_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_switch_create(&sw));
  ASSERT_NEQ(NULL, sw);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_switch_get_node(sw, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_switch_get_node(sw, &node));
  ASSERT_NEQ(NULL, node);

  /* Set state */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_switch_set_on(sw, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_switch_set_on(sw, 0));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_switch_bind_a11y(sw, tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_switch_destroy(sw));

  PASS();
}

SUITE(cmp_ui_switch_suite) { RUN_TEST(test_ui_switch_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_switch_suite);
  GREATEST_MAIN_END();
}
