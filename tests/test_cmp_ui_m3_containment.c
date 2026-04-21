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

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_create(&sheet), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_get_node(sheet, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_set_visible(sheet, 1), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_set_visible(sheet, 0), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_bottom_sheet_destroy(sheet), "%d");
  PASS();
}

TEST test_card(void) {
  cmp_ui_card_t *card;
  cmp_ui_node_t *node;
  cmp_ui_node_t *child;

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_ELEVATED), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_get_node(card, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_box_create(&child), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_add_child(card, child), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_card_destroy(card), "%d");
  PASS();
}

TEST test_dialog(void) {
  cmp_ui_dialog_t *dialog;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS,
                cmp_ui_dialog_create(&dialog, "Warning", "Are you sure?"),
                "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_get_node(dialog, &node), "%d");
  ASSERT(node != NULL);

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_set_visible(dialog, 1), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_set_visible(dialog, 0), "%d");

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_dialog_destroy(dialog), "%d");
  PASS();
}

TEST test_divider(void) {
  cmp_ui_divider_t *divider;
  cmp_ui_node_t *node;

  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_divider_create(&divider), "%d");
  ASSERT_EQ_FMT(CMP_SUCCESS, cmp_ui_divider_get_node(divider, &node), "%d");
  ASSERT(node != NULL);

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