/* clang-format off */
#include "cmp.h"
#include "cmp_ui_splitter.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_splitter_suite);

TEST test_ui_splitter_lifecycle(void) {
  cmp_ui_splitter_t *splitter = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_splitter_create(NULL, 1, 0xFF000000));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_splitter_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_splitter_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_splitter_set_position(NULL, 0.5f));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_splitter_create(&splitter, 1, 0xFF000000));
  ASSERT_NEQ(NULL, splitter);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_splitter_get_node(splitter, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_splitter_get_node(splitter, &node));
  ASSERT_NEQ(NULL, node);

  /* Set position bounds */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_splitter_set_position(splitter, -1.0f)); /* clamps to 0.0f */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_splitter_set_position(splitter, 2.0f)); /* clamps to 1.0f */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_splitter_set_position(splitter, 0.5f));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_splitter_destroy(splitter));

  PASS();
}

SUITE(cmp_ui_splitter_suite) { RUN_TEST(test_ui_splitter_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_splitter_suite);
  GREATEST_MAIN_END();
}