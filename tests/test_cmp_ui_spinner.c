/* clang-format off */
#include "cmp.h"
#include "cmp_ui_spinner.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_spinner_suite);

TEST test_ui_spinner_lifecycle(void) {
  cmp_ui_spinner_t *spinner = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_spinner_create(NULL, 24.0f, 0xFF000000));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_spinner_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_spinner_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_spinner_update(NULL, 16.0f));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_spinner_create(&spinner, 24.0f, 0xFF000000));
  ASSERT_NEQ(NULL, spinner);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_spinner_get_node(spinner, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_spinner_get_node(spinner, &node));
  ASSERT_NEQ(NULL, node);

  /* Update */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_spinner_update(spinner, 16.0f));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_spinner_update(spinner, 2000.0f)); /* Test wrap around */

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_spinner_destroy(spinner));

  PASS();
}

SUITE(cmp_ui_spinner_suite) { RUN_TEST(test_ui_spinner_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_spinner_suite);
  GREATEST_MAIN_END();
}