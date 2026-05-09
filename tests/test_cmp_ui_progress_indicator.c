/* clang-format off */
#include "cmp.h"
#include "cmp_ui_progress_indicator.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_progress_indicator_suite);

TEST test_ui_progress_indicator_lifecycle(void) {
  cmp_ui_progress_indicator_t *indicator = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_progress_indicator_create(
                                       NULL, CMP_UI_PROGRESS_INDICATOR_LINEAR));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_progress_indicator_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_progress_indicator_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_progress_indicator_set_progress(NULL, 0.5f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_progress_indicator_set_type(
                NULL, CMP_UI_PROGRESS_INDICATOR_CIRCULAR));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_progress_indicator_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_indicator_create(
                             &indicator, CMP_UI_PROGRESS_INDICATOR_LINEAR));
  ASSERT_NEQ(NULL, indicator);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_progress_indicator_get_node(indicator, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_indicator_get_node(indicator, &node));
  ASSERT_NEQ(NULL, node);

  /* Set progress bounds */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_indicator_set_progress(
                             indicator, -1.0f)); /* clamps to 0.0f */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_indicator_set_progress(
                             indicator, 2.0f)); /* clamps to 1.0f */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_progress_indicator_set_progress(indicator, 0.5f));

  /* Set type */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_indicator_set_type(
                             indicator, CMP_UI_PROGRESS_INDICATOR_CIRCULAR));
  ASSERT_EQ(
      CMP_SUCCESS,
      cmp_ui_progress_indicator_set_type(
          indicator, CMP_UI_PROGRESS_INDICATOR_CIRCULAR)); /* No-op path */

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_indicator_bind_a11y(indicator, tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_progress_indicator_destroy(indicator));

  PASS();
}

SUITE(cmp_ui_progress_indicator_suite) {
  RUN_TEST(test_ui_progress_indicator_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_progress_indicator_suite);
  GREATEST_MAIN_END();
}
