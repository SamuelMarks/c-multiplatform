/* clang-format off */
#include "cmp.h"
#include "cmp_ui_app_bar.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_app_bar_suite);

TEST test_ui_app_bar_lifecycle(void) {
  cmp_ui_app_bar_t *bar = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  cmp_ui_node_t *action = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_app_bar_create(NULL, CMP_UI_APP_BAR_PLACEMENT_TOP));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_set_title(NULL, "new title"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_bind_a11y(NULL, tree));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_add_action(NULL, action));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_app_bar_create(&bar, CMP_UI_APP_BAR_PLACEMENT_TOP));
  ASSERT_NEQ(NULL, bar);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_get_node(bar, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_app_bar_get_node(bar, &node));
  ASSERT_NEQ(NULL, node);

  /* Set title tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_app_bar_set_title(bar, "Test Title"));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_app_bar_set_title(bar, NULL));

  /* Add action test */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_button_create(&action, "Click", 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_app_bar_add_action(bar, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_app_bar_add_action(bar, action));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_app_bar_bind_a11y(bar, tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_app_bar_destroy(bar));

  PASS();
}

SUITE(cmp_ui_app_bar_suite) { RUN_TEST(test_ui_app_bar_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_app_bar_suite);
  GREATEST_MAIN_END();
}