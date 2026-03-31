/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_aria_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_t *aria = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_aria_create(tree, &aria);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, aria);

  res = cmp_aria_destroy(aria);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_aria_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  int res;
  cmp_aria_t *aria = NULL;
  cmp_a11y_tree_create(&tree);

  res = cmp_aria_create(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_create(NULL, &aria);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_aria_create(tree, &aria);

  res = cmp_aria_set_role(NULL, 1, CMP_ARIA_ROLE_BUTTON);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_set_state_bool(NULL, 1, "aria-expanded", 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_set_state_bool(aria, 1, NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_sync(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_aria_destroy(aria);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_aria_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_t *aria = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  cmp_aria_create(tree, &aria);

  res = cmp_aria_set_role(aria, 10, CMP_ARIA_ROLE_DIALOG);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_set_state_bool(aria, 10, "aria-expanded", 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_set_state_bool(aria, 20, "aria-checked", 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_sync(aria);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_aria_destroy(aria);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(aria_suite) {
  RUN_TEST(test_aria_lifecycle);
  RUN_TEST(test_aria_null_args);
  RUN_TEST(test_aria_operations);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(aria_suite);
  GREATEST_MAIN_END();
}
