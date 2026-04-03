#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_button.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

TEST test_f2_uia_tree_validation(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_ui_node_t *btn = NULL;
  int res;

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_create(&btn, "UIA Button", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Serialize layout to a11y tree simulating Windows Inspect.exe UIA population
   */
  res = cmp_a11y_tree_serialize(tree, btn, NULL, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Assert the button received the correct a11y traits from Fluent 2 mapping */
  /* This validates the a11y DOM tree translation */
  {
    uint32_t traits = 0;
    cmp_a11y_tree_get_node_traits(tree, btn->layout->id, &traits);
    ASSERT(traits & CMP_A11Y_TRAIT_BUTTON);
  }

  cmp_a11y_tree_destroy(tree);
  cmp_ui_node_destroy(btn);
  PASS();
}

TEST test_f2_dom_aria_validation(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_aria_t *aria = NULL;
  cmp_ui_node_t *btn = NULL;
  int res;

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Map the active a11y tree to an ARIA structure mimicking Emscripten Web
   * output */
  res = cmp_aria_create(tree, &aria);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_create(&btn, "ARIA Button", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_a11y_tree_serialize(tree, btn, NULL, 0);

  /* Check ARIA roles sync logic */
  res = cmp_aria_sync(aria);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_aria_destroy(aria);
  cmp_a11y_tree_destroy(tree);
  cmp_ui_node_destroy(btn);
  PASS();
}

TEST test_f2_keyboard_flow(void) {
  cmp_a11y_tree_t *tree = NULL;
  cmp_focus_manager_t *focus = NULL;
  cmp_ui_node_t *box = NULL;
  cmp_ui_node_t *btn1 = NULL;
  cmp_ui_node_t *btn2 = NULL;
  int res;

  res = cmp_a11y_tree_create(&tree);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_focus_manager_create(tree, &focus);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_ui_box_create(&box);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_create(&btn1, "Btn1", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  cmp_ui_node_add_child(box, btn1);

  res = cmp_f2_button_create(&btn2, "Btn2", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  cmp_ui_node_add_child(box, btn2);

  cmp_a11y_tree_serialize(tree, box, NULL, 0);

  /* Simulate TAB key pressing to move focus down the DOM tree */
  {
    int next_node = -1;
    res = cmp_focus_manager_navigate(focus, btn1->layout->id, 1, &next_node);
    ASSERT_EQ(CMP_SUCCESS, res);
  }

  /* Clean up */
  cmp_focus_manager_destroy(focus);
  cmp_a11y_tree_destroy(tree);
  cmp_ui_node_destroy(box);
  PASS();
}

SUITE(cmp_f2_a11y_suite) {
  RUN_TEST(test_f2_uia_tree_validation);
  RUN_TEST(test_f2_dom_aria_validation);
  RUN_TEST(test_f2_keyboard_flow);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_a11y_suite);
  GREATEST_MAIN_END();
}
