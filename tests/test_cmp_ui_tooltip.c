/* clang-format off */
#include "cmp.h"
#include "cmp_ui_tooltip.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_tooltip_suite);

TEST test_ui_tooltip_lifecycle(void) {
  cmp_ui_tooltip_t *tooltip = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_tooltip_create(NULL, "Tip", 0, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_tooltip_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_tooltip_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_tooltip_set_text(NULL, "new_text"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_tooltip_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_create(&tooltip, "Tip", 0, 0));
  ASSERT_NEQ(NULL, tooltip);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_tooltip_get_node(tooltip, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_get_node(tooltip, &node));
  ASSERT_NEQ(NULL, node);

  /* Set text tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_set_text(tooltip, "New Tip"));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_set_text(tooltip, NULL));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_bind_a11y(tooltip, tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_destroy(tooltip));

  PASS();
}

TEST test_ui_tooltip_null_text_init(void) {
  cmp_ui_tooltip_t *tooltip = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_create(&tooltip, NULL, 0, 0));
  ASSERT_NEQ(NULL, tooltip);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_tooltip_destroy(tooltip));

  PASS();
}

SUITE(cmp_ui_tooltip_suite) {
  RUN_TEST(test_ui_tooltip_lifecycle);
  RUN_TEST(test_ui_tooltip_null_text_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_tooltip_suite);
  GREATEST_MAIN_END();
}
