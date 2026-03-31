/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_a11y_rotor_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_a11y_rotor_t *rotor = NULL;
  int res;

  cmp_a11y_tree_create(&tree);

  res = cmp_a11y_rotor_create(tree, &rotor);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, rotor);

  res = cmp_a11y_rotor_destroy(rotor);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_rotor_null_args(void) {
  int res = cmp_a11y_rotor_create(NULL, NULL);
  cmp_a11y_rotor_t *rotor = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int count;
  int nodes[10];
  cmp_a11y_tree_create(&tree);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_rotor_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_rotor_create(NULL, &rotor);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_rotor_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_rotor_create(tree, &rotor);

  res = cmp_a11y_rotor_register_node(NULL, 1, CMP_A11Y_ROTOR_HEADING);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res =
      cmp_a11y_rotor_get_nodes(NULL, CMP_A11Y_ROTOR_HEADING, nodes, 10, &count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res =
      cmp_a11y_rotor_get_nodes(rotor, CMP_A11Y_ROTOR_HEADING, NULL, 10, &count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res =
      cmp_a11y_rotor_get_nodes(rotor, CMP_A11Y_ROTOR_HEADING, nodes, 10, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_a11y_rotor_get_nodes(rotor, CMP_A11Y_ROTOR_HEADING, nodes, -1,
                                 &count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_a11y_rotor_destroy(rotor);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_a11y_rotor_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_a11y_rotor_t *rotor = NULL;
  int res;
  int nodes[10];
  int count = 0;
  cmp_a11y_tree_create(&tree);

  cmp_a11y_rotor_create(tree, &rotor);

  res = cmp_a11y_rotor_register_node(rotor, 10, CMP_A11Y_ROTOR_HEADING);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_rotor_register_node(rotor, 20, CMP_A11Y_ROTOR_LINK);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_rotor_register_node(rotor, 30, CMP_A11Y_ROTOR_HEADING);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_a11y_rotor_get_nodes(rotor, CMP_A11Y_ROTOR_HEADING, nodes, 10,
                                 &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2, count);
  ASSERT_EQ(10, nodes[0]);
  ASSERT_EQ(30, nodes[1]);

  res = cmp_a11y_rotor_get_nodes(rotor, CMP_A11Y_ROTOR_LINK, nodes, 10, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, count);
  ASSERT_EQ(20, nodes[0]);

  res = cmp_a11y_rotor_get_nodes(rotor, CMP_A11Y_ROTOR_LANDMARK, nodes, 10,
                                 &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, count);

  /* Test truncation */
  res =
      cmp_a11y_rotor_get_nodes(rotor, CMP_A11Y_ROTOR_HEADING, nodes, 1, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2, count); /* total count */
  ASSERT_EQ(10, nodes[0]);

  cmp_a11y_rotor_destroy(rotor);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(a11y_rotor_suite) {
  RUN_TEST(test_a11y_rotor_lifecycle);
  RUN_TEST(test_a11y_rotor_null_args);
  RUN_TEST(test_a11y_rotor_operations);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(a11y_rotor_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
