/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_focus_ring_lifecycle(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_ring_t *ring = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_focus_ring_create(tree, &ring);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, ring);

  res = cmp_focus_ring_destroy(ring);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_focus_ring_null_args(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_ring_t *ring = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  res = cmp_focus_ring_create(NULL, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_ring_create(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_ring_create(NULL, &ring);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_ring_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_focus_ring_create(tree, &ring);

  res = cmp_focus_ring_set_keyboard_mode(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_focus_ring_node_focused(NULL, 10);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_focus_ring_destroy(ring);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

TEST test_focus_ring_operations(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_ring_t *ring = NULL;
  int res;
  cmp_a11y_tree_create(&tree);

  cmp_focus_ring_create(tree, &ring);

  /* Start in pointer mode by default */
  res = cmp_focus_ring_node_focused(ring, 10);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Switch to keyboard mode */
  res = cmp_focus_ring_set_keyboard_mode(ring, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Focus a new node via keyboard */
  res = cmp_focus_ring_node_focused(ring, 20);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Switch back to pointer */
  res = cmp_focus_ring_set_keyboard_mode(ring, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_focus_ring_destroy(ring);
  cmp_a11y_tree_destroy(tree);
  PASS();
}

SUITE(focus_ring_suite) {
  RUN_TEST(test_focus_ring_lifecycle);
  RUN_TEST(test_focus_ring_null_args);
  RUN_TEST(test_focus_ring_operations);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(focus_ring_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
