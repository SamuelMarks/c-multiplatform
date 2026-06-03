/* clang-format off */
#include "greatest.h"
#include "cmp_ui_bottom_sheet.h"
#include "cmp_ui_card.h"
#include "cmp_ui_dialog.h"
#include "cmp_ui_divider.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_bottom_sheet(void) {
  cmp_ui_bottom_sheet_t *sheet;
  cmp_ui_node_t *node;
  cmp_a11y_tree_t *tree;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_create(NULL), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_create(&sheet), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_bottom_sheet_get_node(NULL, &node), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_bottom_sheet_get_node(sheet, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_get_node(sheet, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_set_visible(NULL, 1),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_set_visible(sheet, 1), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_set_visible(sheet, 0), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_create(&tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_bottom_sheet_bind_a11y(NULL, tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_bottom_sheet_bind_a11y(sheet, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_bind_a11y(sheet, tree), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_destroy(tree), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_bottom_sheet_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_destroy(sheet), "%d");
  PASS();
}

TEST test_card(void) {
  cmp_ui_card_t *card;
  cmp_ui_node_t *node;
  cmp_ui_node_t *child;
  cmp_a11y_tree_t *tree;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_card_create(NULL, CMP_UI_CARD_STYLE_ELEVATED), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_ELEVATED), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_card_get_node(NULL, &node), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_card_get_node(card, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_get_node(card, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_box_create(&child), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_card_add_child(NULL, child),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_card_add_child(card, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_add_child(card, child), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_create(&tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_card_bind_a11y(NULL, tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_card_bind_a11y(card, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_bind_a11y(card, tree), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_destroy(tree), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_card_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_destroy(card), "%d");

  /* Test FILLED */
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_FILLED), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_destroy(card), "%d");

  /* Test OUTLINED */
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_OUTLINED), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_destroy(card), "%d");

  PASS();
}

TEST test_dialog(void) {
  cmp_ui_dialog_t *dialog;
  cmp_ui_node_t *node;
  cmp_a11y_tree_t *tree;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG,
                cmp_ui_dialog_create(NULL, "Warning", "Are you sure?"), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_dialog_create(&dialog, "Warning", "Are you sure?"),
                "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_dialog_get_node(NULL, &node),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_dialog_get_node(dialog, NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_get_node(dialog, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_dialog_set_visible(NULL, 1),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_set_visible(dialog, 1), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_set_visible(dialog, 0), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_create(&tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_dialog_bind_a11y(NULL, tree),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_dialog_bind_a11y(dialog, NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_bind_a11y(dialog, tree), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_destroy(tree), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_dialog_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_destroy(dialog), "%d");

  /* Test without title/content */
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_create(&dialog, NULL, NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_destroy(dialog), "%d");

  PASS();
}

TEST test_divider(void) {
  cmp_ui_divider_t *divider;
  cmp_ui_node_t *node;
  cmp_a11y_tree_t *tree;

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_divider_create(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_divider_create(&divider), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_divider_get_node(NULL, &node),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_divider_get_node(divider, NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_divider_get_node(divider, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_create(&tree), "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_divider_bind_a11y(NULL, tree),
                "%d");
  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_divider_bind_a11y(divider, NULL),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_divider_bind_a11y(divider, tree), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_a11y_tree_destroy(tree), "%d");

  ASSERT_EQ_FMT(CMP_ERROR_INVALID_ARG, cmp_ui_divider_destroy(NULL), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_divider_destroy(divider), "%d");
  PASS();
}

SUITE(m3_containment_suite) {
  RUN_TEST(test_bottom_sheet);
  RUN_TEST(test_card);
  RUN_TEST(test_dialog);
  RUN_TEST(test_divider);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(m3_containment_suite);
  GREATEST_MAIN_END();
}
