/* clang-format off */
#include "cmp.h"
#include "cmp_ui_accordion.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_accordion_suite);

TEST test_ui_accordion_lifecycle(void) {
  cmp_ui_accordion_t *accordion = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_accordion_create(NULL, "Title", 0xFFFFFFFF));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_accordion_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_accordion_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_accordion_get_node(accordion, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_accordion_set_expanded(NULL, 1));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_accordion_create(&accordion, "Test Accordion", 0xFFFFFFFF));
  ASSERT_NEQ(NULL, accordion);

  /* Get node */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_accordion_get_node(accordion, &node));
  ASSERT_NEQ(NULL, node);

  /* Set expanded */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_accordion_set_expanded(accordion, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_accordion_set_expanded(accordion, 0));

  /* Successful destruction */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_accordion_destroy(accordion));

  PASS();
}

TEST test_ui_accordion_null_title(void) {
  cmp_ui_accordion_t *accordion = NULL;

  /* Create with NULL title */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_accordion_create(&accordion, NULL, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, accordion);

  /* Successful destruction */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_accordion_destroy(accordion));

  PASS();
}

SUITE(cmp_ui_accordion_suite) {
  RUN_TEST(test_ui_accordion_lifecycle);
  RUN_TEST(test_ui_accordion_null_title);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_accordion_suite);
  GREATEST_MAIN_END();
}
