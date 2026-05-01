/* clang-format off */
#include "cmp_ui_text_field.h"
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
  cmp_ui_text_field_t *field = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int rc;

  /* Setup A11y Tree for testing */
  rc = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test creation with null pointer */
  rc = cmp_ui_text_field_create(NULL, "Test Label");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal creation */
  rc = cmp_ui_text_field_create(&field, "Test Label");
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, field);

  /* Test normal creation with no label */
  {
    cmp_ui_text_field_t *field2 = NULL;
    rc = cmp_ui_text_field_create(&field2, NULL);
    ASSERT_EQ(CMP_SUCCESS, rc);
    ASSERT_NEQ_PTR(NULL, field2);
    rc = cmp_ui_text_field_destroy(field2);
    ASSERT_EQ(CMP_SUCCESS, rc);
  }

  /* Test get node */
  rc = cmp_ui_text_field_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_text_field_get_node(field, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_text_field_get_node(field, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, node);

  /* Test set value */
  rc = cmp_ui_text_field_set_value(NULL, "New Value");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_text_field_set_value(field, "New Value");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_text_field_set_value(field, "Another Value");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_text_field_set_value(field, NULL);
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test a11y bind */
  rc = cmp_ui_text_field_bind_a11y(NULL, tree);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_text_field_bind_a11y(field, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_text_field_bind_a11y(field, tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test destroy */
  rc = cmp_ui_text_field_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_text_field_destroy(field);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_a11y_tree_destroy(tree);
  ASSERT_EQ(CMP_SUCCESS, rc);

  printf("test_cmp_ui_text_field passed.\n");
  return 0;
}
