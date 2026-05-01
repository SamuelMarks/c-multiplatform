/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_focus_navigation_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_nav_t *nav = NULL;
  int res;

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_focus_nav_create(tree, &nav);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, nav);

  res = cmp_focus_nav_destroy(nav);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_focus_navigation_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_nav_t *nav = NULL;
  int res;

  res = cmp_focus_nav_create(NULL, &nav);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_focus_nav_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_nav_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_nav_create(tree, &nav);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_focus_nav_handle_tab(NULL, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_nav_destroy(nav);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_focus_navigation_handle_tab(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_nav_t *nav = NULL;
  int res;

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_focus_nav_create(tree, &nav);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Test normal tab */
  res = cmp_focus_nav_handle_tab(nav, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Test shift-tab */
  res = cmp_focus_nav_handle_tab(nav, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_focus_nav_destroy(nav);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(focus_navigation_suite) {
  RUN_TEST(test_focus_navigation_lifecycle);
  RUN_TEST(test_focus_navigation_null_args);
  RUN_TEST(test_focus_navigation_handle_tab);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(focus_navigation_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
