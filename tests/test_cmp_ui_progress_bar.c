/* clang-format off */
#include "cmp.h"
#include "cmp_ui_progress_bar.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_progress_bar_suite);

TEST test_ui_progress_bar_lifecycle(void) {
  cmp_ui_progress_bar_t *bar = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_progress_bar_create(NULL, 0, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_progress_bar_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_progress_bar_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_progress_bar_set_progress(NULL, 0.5f));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_progress_bar_create(&bar, 0xFF000000, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, bar);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_progress_bar_get_node(bar, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_bar_get_node(bar, &node));
  ASSERT_NEQ(NULL, node);

  /* Set progress bounds */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_progress_bar_set_progress(bar, -1.0f)); /* clamps to 0.0f */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_progress_bar_set_progress(bar, 2.0f)); /* clamps to 1.0f */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_bar_set_progress(bar, 0.5f));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_bar_destroy(bar));

  PASS();
}

SUITE(cmp_ui_progress_bar_suite) { RUN_TEST(test_ui_progress_bar_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_progress_bar_suite);
  GREATEST_MAIN_END();
}