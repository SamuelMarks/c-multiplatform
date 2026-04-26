/* clang-format off */
#include "greatest.h"
#include "cmp_ui_action_button.h"
#include "cmp_ui_fab.h"
#include "cmp_ui_icon_button.h"
#include "cmp_ui_segmented_button.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_action_button(void) {
  cmp_ui_action_button_t *btn;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_action_button_create(
                    &btn, "Click", CMP_UI_ACTION_BUTTON_STYLE_ELEVATED),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_action_button_get_node(btn, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_action_button_set_label(btn, "New Label"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_action_button_set_label(btn, NULL), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_action_button_destroy(btn), "%d");

  PASS();
}

TEST test_fab(void) {
  cmp_ui_fab_t *fab;
  cmp_ui_node_t *node;
  cmp_a11y_tree_t *tree;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_fab_create(NULL, "add_icon"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_create(&fab, "add_icon"), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_fab_get_node(NULL, &node), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_fab_get_node(fab, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_get_node(fab, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_fab_set_icon(NULL, "edit_icon"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_set_icon(fab, "edit_icon"), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_set_icon(fab, NULL), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_create(&tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_fab_bind_a11y(NULL, tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_fab_bind_a11y(fab, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_bind_a11y(fab, tree), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_destroy(tree), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_fab_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_destroy(fab), "%d");

  /* Test NULL icon name */
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_create(&fab, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_fab_destroy(fab), "%d");

  PASS();
}

TEST test_icon_button(void) {
  cmp_ui_icon_button_t *btn;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_icon_button_create(&btn, "settings",
                                          CMP_UI_ICON_BUTTON_STYLE_STANDARD),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_icon_button_get_node(btn, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_icon_button_set_icon(btn, "close"), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_icon_button_set_icon(btn, NULL), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_icon_button_destroy(btn), "%d");

  PASS();
}

TEST test_segmented_button(void) {
  cmp_ui_segmented_button_t *btn;
  cmp_ui_node_t *node;
  int idx1, idx2;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_segmented_button_create(&btn, 0), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_segmented_button_get_node(btn, &node),
                "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_segmented_button_add_segment(btn, "Seg 1", NULL, &idx1),
                "%d");
  ASSERT_EQ_FMT(
      CMP_SUCCESS,
      cmp_ui_segmented_button_add_segment(btn, "Seg 2", "icon", &idx2), "%d");

  ASSERT_EQ(0, idx1);
  ASSERT_EQ(1, idx2);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_segmented_button_set_selected(btn, idx1),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_segmented_button_set_deselected(btn, idx1),
                "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_segmented_button_destroy(btn), "%d");

  PASS();
}

SUITE(m3_buttons_suite) {
  RUN_TEST(test_action_button);
  RUN_TEST(test_fab);
  RUN_TEST(test_icon_button);
  RUN_TEST(test_segmented_button);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(m3_buttons_suite);
  GREATEST_MAIN_END();
}