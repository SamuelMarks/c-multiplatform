/* clang-format off */
#include "cmp_ui_terminal.h"
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
  cmp_ui_terminal_t *terminal = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  /* Test creation with null pointer */
  rc = cmp_ui_terminal_create(NULL, 0xFF000000, 0xFFFFFFFF);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  /* Test normal creation */
  rc = cmp_ui_terminal_create(&terminal, 0xFF000000, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, terminal);

  /* Test get node */
  rc = cmp_ui_terminal_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_terminal_get_node(terminal, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_terminal_get_node(terminal, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT_NEQ_PTR(NULL, node);

  /* Test append output */
  rc = cmp_ui_terminal_append_output(NULL, "test");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_terminal_append_output(terminal, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_terminal_append_output(terminal, "Hello Terminal\n");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_terminal_append_output(terminal, "Line 2\n");
  ASSERT_EQ(CMP_SUCCESS, rc);

  /* Test destroy */
  rc = cmp_ui_terminal_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_terminal_destroy(terminal);
  ASSERT_EQ(CMP_SUCCESS, rc);

  printf("test_cmp_ui_terminal passed.\n");
  return 0;
}
