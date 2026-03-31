/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_a11y_tree_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, tree);

  res = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_a11y_tree_null_args(void) {
  int res = cmp_a11y_tree_create(NULL);
  char buf[64];
  cmp_a11y_tree_t *tree = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_add_node(NULL, 1, "button", "Submit");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_get_node_desc(NULL, 1, buf, 64);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_get_node_desc(tree, 1, NULL, 64);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_tree_get_node_desc(tree, 1, buf, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_tree_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;
  char buf[128];

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_tree_add_node(tree, 10, "button", "Submit");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_add_node(tree, 20, NULL, "Content");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_add_node(tree, 30, "dialog", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_tree_get_node_desc(tree, 10, buf, 128);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("button: Submit", buf);

  res = cmp_a11y_tree_get_node_desc(tree, 20, buf, 128);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("unknown: Content", buf);

  res = cmp_a11y_tree_get_node_desc(tree, 30, buf, 128);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("dialog: unnamed", buf);

  res = cmp_a11y_tree_get_node_desc(tree, 999, buf, 128);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  /* Test bounds error */
  res = cmp_a11y_tree_get_node_desc(tree, 10, buf, 5);
  ASSERT_EQ(CMP_ERROR_BOUNDS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(a11y_tree_suite) {
  RUN_TEST(test_a11y_tree_lifecycle);
  RUN_TEST(test_a11y_tree_null_args);
  RUN_TEST(test_a11y_tree_operations);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(a11y_tree_suite);
  GREATEST_MAIN_END();
}
