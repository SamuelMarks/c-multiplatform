/* clang-format off */
#include "cmp.h"
#include "cmp_ui_icon_button.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_icon_button_suite);

TEST test_ui_icon_button_lifecycle(void) {
  cmp_ui_icon_button_t *btn = NULL;
  cmp_ui_node_t *node = NULL;
  cmp_a11y_tree_t *tree = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_icon_button_create(NULL, "icon",
                                      CMP_UI_ICON_BUTTON_STYLE_STANDARD));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_icon_button_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_icon_button_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_icon_button_set_icon(NULL, "new_icon"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_icon_button_bind_a11y(NULL, tree));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_icon_button_create(&btn, "add",
                                      CMP_UI_ICON_BUTTON_STYLE_FILLED_TONAL));
  ASSERT_NEQ(NULL, btn);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_icon_button_get_node(btn, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_icon_button_get_node(btn, &node));
  ASSERT_NEQ(NULL, node);

  /* Set icon tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_icon_button_set_icon(btn, "edit"));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_icon_button_set_icon(btn, NULL));

  /* A11y tests */
  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_create(&tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_icon_button_bind_a11y(btn, tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_destroy(tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_icon_button_destroy(btn));

  PASS();
}

TEST test_ui_icon_button_null_icon_init(void) {
  cmp_ui_icon_button_t *btn = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_icon_button_create(
                             &btn, NULL, CMP_UI_ICON_BUTTON_STYLE_STANDARD));
  ASSERT_NEQ(NULL, btn);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_icon_button_destroy(btn));

  PASS();
}

SUITE(cmp_ui_icon_button_suite) {
  RUN_TEST(test_ui_icon_button_lifecycle);
  RUN_TEST(test_ui_icon_button_null_icon_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_icon_button_suite);
  GREATEST_MAIN_END();
}
