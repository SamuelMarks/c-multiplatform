/* clang-format off */
#include "cmp.h"
#include "cmp_ui_virtual_list.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_virtual_list_suite);

TEST test_ui_virtual_list_lifecycle(void) {
  cmp_ui_virtual_list_t *list = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_virtual_list_create(NULL, 10, 48.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_virtual_list_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_virtual_list_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_virtual_list_set_item_count(NULL, 20));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_virtual_list_create(&list, 10, 48.0f));
  ASSERT_NEQ(NULL, list);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_virtual_list_get_node(list, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_virtual_list_get_node(list, &node));
  ASSERT_NEQ(NULL, node);

  /* Set item count */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_virtual_list_set_item_count(list, 20));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_virtual_list_destroy(list));

  PASS();
}

SUITE(cmp_ui_virtual_list_suite) { RUN_TEST(test_ui_virtual_list_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_virtual_list_suite);
  GREATEST_MAIN_END();
}