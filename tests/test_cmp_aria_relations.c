/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_aria_relations_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_relations_t *rels = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_aria_relations_create(tree, &rels);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, rels);

  res = cmp_aria_relations_destroy(rels);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_aria_relations_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_relations_t *rels = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_aria_relations_create(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_relations_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_relations_create(NULL, &rels);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_relations_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_aria_relations_create(tree, &rels);

  res = cmp_aria_relations_add(NULL, 1, 2, CMP_ARIA_RELATION_OWNS);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_aria_relations_sync(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_aria_relations_destroy(rels);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_aria_relations_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_relations_t *rels = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  cmp_aria_relations_create(tree, &rels);

  res = cmp_aria_relations_add(rels, 10, 20, CMP_ARIA_RELATION_OWNS);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_relations_add(rels, 10, 30, CMP_ARIA_RELATION_CONTROLS);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_relations_add(rels, 20, 40, CMP_ARIA_RELATION_DESCRIBEDBY);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_aria_relations_sync(rels);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_aria_relations_destroy(rels);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(aria_relations_suite) {
  RUN_TEST(test_aria_relations_lifecycle);
  RUN_TEST(test_aria_relations_null_args);
  RUN_TEST(test_aria_relations_operations);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(aria_relations_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
