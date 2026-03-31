/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_focus_manager_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_manager_t *mgr = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_focus_manager_create(tree, &mgr);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, mgr);

  res = cmp_focus_manager_destroy(mgr);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_focus_manager_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_manager_t *mgr = NULL;
  int res;
  int next_id = -1;
  cmp_a11y_tree_create(&tree);

  res = cmp_focus_manager_create(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_manager_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_manager_create(NULL, &mgr);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_manager_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_focus_manager_create(tree, &mgr);

  res = cmp_focus_manager_set_focus(NULL, 1, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_manager_navigate(NULL, 1, 0, &next_id);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_manager_navigate(mgr, 1, 0, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_focus_manager_destroy(mgr);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_focus_manager_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_manager_t *mgr = NULL;
  int next_id = -1;
  int res;
  cmp_a11y_tree_create(&tree);

  cmp_focus_manager_create(tree, &mgr);

  res = cmp_focus_manager_navigate(mgr, 10, 0, &next_id);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(-1, next_id); /* No nodes yet */

  res = cmp_focus_manager_set_focus(mgr, 10, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_focus_manager_set_focus(mgr, 20, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Navigate with one unknown current node, should pick first */
  res = cmp_focus_manager_navigate(mgr, 99, 0, &next_id);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(10, next_id);

  /* Navigate with known node but no matches in direction, should return current
   */
  res = cmp_focus_manager_navigate(mgr, 10, 1, &next_id);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(10, next_id);

  /* Remove focus */
  res = cmp_focus_manager_set_focus(mgr, 10, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_focus_manager_destroy(mgr);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(focus_manager_suite) {
  RUN_TEST(test_focus_manager_lifecycle);
  RUN_TEST(test_focus_manager_null_args);
  RUN_TEST(test_focus_manager_operations);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(focus_manager_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
