/* clang-format off */
#include "cmp.h"
#include "cmp_ui_navigation_rail.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_navigation_rail_suite);

TEST test_ui_navigation_rail_lifecycle(void) {
  cmp_ui_navigation_rail_t *rail = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int index;
  int i;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_navigation_rail_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_navigation_rail_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_navigation_rail_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_navigation_rail_add_destination(
                                       NULL, "icon", "label", &index));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_navigation_rail_set_selected(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_navigation_rail_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_create(&rail));
  ASSERT_NEQ(NULL, rail);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_navigation_rail_get_node(rail, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_get_node(rail, &node));
  ASSERT_NEQ(NULL, node);

  /* Add destinations */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_add_destination(
                             rail, "home", "Home", &index));
  ASSERT_EQ(0, index);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_add_destination(
                             rail, "settings", NULL, &index));
  ASSERT_EQ(1, index);

  /* Add more to trigger realloc */
  for (i = 0; i < 5; i++) {
    ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_add_destination(
                               rail, "icon", "label", &index));
  }

  /* Set selection (invalid) */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_navigation_rail_set_selected(rail, -1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_navigation_rail_set_selected(rail, 100));

  /* Set selection (valid) */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_set_selected(rail, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_set_selected(rail, 2));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_bind_a11y(rail, tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_navigation_rail_destroy(rail));

  PASS();
}

SUITE(cmp_ui_navigation_rail_suite) {
  RUN_TEST(test_ui_navigation_rail_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_navigation_rail_suite);
  GREATEST_MAIN_END();
}
