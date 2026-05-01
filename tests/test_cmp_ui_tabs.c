/* clang-format off */
#include "cmp.h"
#include "cmp_ui_tabs.h"
#include "greatest.h"
/* clang-format on */

TEST test_ui_tabs_lifecycle(void) {
  cmp_ui_tabs_t *tabs = NULL;
  int res;

  res = cmp_ui_tabs_create(&tabs);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, tabs);

  res = cmp_ui_tabs_destroy(tabs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_tabs_null_args(void) {
  cmp_ui_tabs_t *tabs = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int index;
  int res;

  res = cmp_ui_tabs_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_tabs_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_tabs_create(&tabs);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_tabs_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_tabs_get_node(tabs, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_tabs_add_tab(NULL, "Tab1", &index);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_tabs_add_tab(tabs, NULL, &index);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_tabs_set_selected(NULL, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_tabs_bind_a11y(NULL, tree);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_tabs_bind_a11y(tabs, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_tabs_destroy(tabs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_ui_tabs_operations(void) {
  cmp_ui_tabs_t *tabs = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int index1, index2;
  int res;

  res = cmp_ui_tabs_create(&tabs);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_tabs_get_node(tabs, &node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, node);

  res = cmp_ui_tabs_add_tab(tabs, "Settings", &index1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, index1);

  res = cmp_ui_tabs_add_tab(tabs, "Profile", &index2);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, index2);

  res = cmp_ui_tabs_set_selected(tabs, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Out of bounds set */
  res = cmp_ui_tabs_set_selected(tabs, 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_tabs_bind_a11y(tabs, tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_tabs_destroy(tabs);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(ui_tabs_suite) {
  RUN_TEST(test_ui_tabs_lifecycle);
  RUN_TEST(test_ui_tabs_null_args);
  RUN_TEST(test_ui_tabs_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_tabs_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
