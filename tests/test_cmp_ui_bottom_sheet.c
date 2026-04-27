/* clang-format off */
#include "cmp.h"
#include "cmp_ui_bottom_sheet.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_bottom_sheet_suite);

TEST test_ui_bottom_sheet_lifecycle(void) {
  cmp_ui_bottom_sheet_t *sheet = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_set_visible(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_bottom_sheet_create(&sheet));
  ASSERT_NEQ(NULL, sheet);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_get_node(sheet, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_bottom_sheet_get_node(sheet, &node));
  ASSERT_NEQ(NULL, node);

  /* Set visible tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_bottom_sheet_set_visible(sheet, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_bottom_sheet_set_visible(sheet, 0));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_bottom_sheet_bind_a11y(sheet, tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_bottom_sheet_destroy(sheet));

  PASS();
}

SUITE(cmp_ui_bottom_sheet_suite) { RUN_TEST(test_ui_bottom_sheet_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_bottom_sheet_suite);
  GREATEST_MAIN_END();
}