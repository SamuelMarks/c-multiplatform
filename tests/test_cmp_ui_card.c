/* clang-format off */
#include "cmp.h"
#include "cmp_ui_card.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_card_suite);

TEST test_ui_card_lifecycle(void) {
  cmp_ui_card_t *card = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *child = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_card_create(NULL, CMP_UI_CARD_STYLE_ELEVATED));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_card_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_card_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_card_add_child(NULL, child));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_card_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_FILLED));
  ASSERT_NEQ(NULL, card);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_card_get_node(card, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_get_node(card, &node));
  ASSERT_NEQ(NULL, node);

  /* Add child */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_box_create(&child));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_card_add_child(card, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_add_child(card, child));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_bind_a11y(card, tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_destroy(card));

  PASS();
}

TEST test_ui_card_styles(void) {
  cmp_ui_card_t *card = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_ELEVATED));
  ASSERT_NEQ(NULL, card);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_destroy(card));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_create(&card, CMP_UI_CARD_STYLE_OUTLINED));
  ASSERT_NEQ(NULL, card);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_card_destroy(card));

  PASS();
}

SUITE(cmp_ui_card_suite) {
  RUN_TEST(test_ui_card_lifecycle);
  RUN_TEST(test_ui_card_styles);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_card_suite);
  GREATEST_MAIN_END();
}