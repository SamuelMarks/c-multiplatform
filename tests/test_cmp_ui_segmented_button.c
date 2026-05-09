/* clang-format off */
#include "cmp.h"
#include "cmp_ui_segmented_button.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_segmented_button_suite);

TEST test_ui_segmented_button_lifecycle(void) {
  cmp_ui_segmented_button_t *btn = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;
  int seg_index;
  int i;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_segmented_button_create(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_segmented_button_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_add_segment(NULL, "a", "b", &seg_index));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_set_selected(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_set_deselected(NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_segmented_button_create(&btn, 0)); /* single select */
  ASSERT_NEQ(NULL, btn);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_segmented_button_get_node(btn, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_get_node(btn, &node));
  ASSERT_NEQ(NULL, node);

  /* Add segments */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_add_segment(btn, "A", "icon_a",
                                                             &seg_index));
  ASSERT_EQ(0, seg_index);
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_segmented_button_add_segment(btn, "B", NULL, &seg_index));
  ASSERT_EQ(1, seg_index);
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_segmented_button_add_segment(btn, NULL, "icon_c", NULL));

  /* Trigger reallocation capacity limit (defaults to 4) */
  for (i = 0; i < 5; i++) {
    ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_add_segment(
                               btn, "X", "icon_x", &seg_index));
  }

  /* Set selection (invalid) */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_set_selected(btn, -1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_set_selected(btn, 100));

  /* Set selection (valid) */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_set_selected(btn, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_set_selected(
                             btn, 2)); /* Should deselect 1 */

  /* Set deselection (invalid) */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_set_deselected(btn, -1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_segmented_button_set_deselected(btn, 100));

  /* Set deselection (valid) */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_set_deselected(btn, 2));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_bind_a11y(btn, tree));
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_destroy(btn));

  PASS();
}

TEST test_ui_segmented_button_multi_select(void) {
  cmp_ui_segmented_button_t *btn = NULL;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_segmented_button_create(&btn, 1)); /* multi select */
  ASSERT_NEQ(NULL, btn);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_segmented_button_add_segment(btn, "A", NULL, NULL));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_segmented_button_add_segment(btn, "B", NULL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_set_selected(btn, 0));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_set_selected(
                             btn, 1)); /* 0 should stay selected */

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_segmented_button_destroy(btn));

  PASS();
}

SUITE(cmp_ui_segmented_button_suite) {
  RUN_TEST(test_ui_segmented_button_lifecycle);
  RUN_TEST(test_ui_segmented_button_multi_select);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_segmented_button_suite);
  GREATEST_MAIN_END();
}
