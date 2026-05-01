/* clang-format off */
#include "cmp_ui_tree_view.h"
#include "cmp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define ASSERT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected %d, got %d\n", __FILE__,       \
             __LINE__, (int)(expected), (int)(actual));                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_NEQ_PTR(expected, actual)                                       \
  do {                                                                         \
    if ((expected) == (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected not %p, got %p\n", __FILE__,   \
             __LINE__, (void *)(expected), (void *)(actual));                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  cmp_ui_tree_view_t *tree = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  /* Test creation with null pointer */
  rc = cmp_ui_tree_view_create(NULL, 0xFFFFFFFF);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal creation */
  rc = cmp_ui_tree_view_create(&tree, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, tree);

  /* Test get node */
  rc = cmp_ui_tree_view_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_tree_view_get_node(tree, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_tree_view_get_node(tree, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, node);

  /* Test add item */
  rc = cmp_ui_tree_view_add_item(NULL, "Test", 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_tree_view_add_item(tree, NULL, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_tree_view_add_item(tree, "Test", -1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_tree_view_add_item(tree, "Root Node", 0);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_tree_view_add_item(tree, "Child Node", 1);
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test destroy */
  rc = cmp_ui_tree_view_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_tree_view_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  printf("test_cmp_ui_tree_view passed.\n");
  return 0;
}
