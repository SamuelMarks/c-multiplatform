#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_button.h"
/* clang-format on */

TEST test_f2_button_create(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_button_t *btn_data = NULL;
  int res;

  res = cmp_f2_button_create(&node, "Click Me", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);
  ASSERT(node->properties != NULL);

  btn_data = (cmp_f2_button_t *)node->properties;
  ASSERT_EQ(CMP_F2_BUTTON_TYPE_STANDARD, btn_data->type);
  ASSERT_EQ(CMP_F2_BUTTON_VARIANT_SECONDARY, btn_data->variant);
  ASSERT_EQ(CMP_F2_BUTTON_SIZE_MEDIUM, btn_data->size);
  ASSERT_EQ(CMP_F2_BUTTON_SHAPE_ROUNDED, btn_data->shape);
  ASSERT_EQ(CMP_F2_BUTTON_LAYOUT_TEXT_ONLY, btn_data->layout_mode);

  ASSERT_EQ(32.0f, node->layout->height);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_button_variants_and_sizes(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_button_t *btn_data = NULL;
  int res;

  res = cmp_f2_button_create(&node, "Variants", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_set_variant(node, CMP_F2_BUTTON_VARIANT_PRIMARY);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_set_size(node, CMP_F2_BUTTON_SIZE_SMALL);
  ASSERT_EQ(CMP_SUCCESS, res);

  btn_data = (cmp_f2_button_t *)node->properties;
  ASSERT_EQ(CMP_F2_BUTTON_VARIANT_PRIMARY, btn_data->variant);
  ASSERT_EQ(CMP_F2_BUTTON_SIZE_SMALL, btn_data->size);
  ASSERT_EQ(24.0f, node->layout->height);

  res = cmp_f2_button_set_size(node, CMP_F2_BUTTON_SIZE_LARGE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(40.0f, node->layout->height);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_button_states_and_shapes(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_button_t *btn_data = NULL;
  int res;

  res = cmp_f2_button_create(&node, "Shapes", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_set_shape(node, CMP_F2_BUTTON_SHAPE_CIRCULAR);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_set_state(node, CMP_F2_BUTTON_STATE_HOVER);
  ASSERT_EQ(CMP_SUCCESS, res);

  btn_data = (cmp_f2_button_t *)node->properties;
  ASSERT_EQ(CMP_F2_BUTTON_SHAPE_CIRCULAR, btn_data->shape);
  ASSERT_EQ(CMP_F2_BUTTON_STATE_HOVER, btn_data->state);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_compound_button(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_button_t *btn_data = NULL;
  int res;

  res = cmp_f2_compound_button_create(&node, "Primary", "Secondary description",
                                      NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  btn_data = (cmp_f2_button_t *)node->properties;
  ASSERT_EQ(CMP_F2_BUTTON_TYPE_COMPOUND, btn_data->type);
  ASSERT(btn_data->secondary_text_node != NULL);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_menu_split_button(void) {
  cmp_ui_node_t *menu_node = NULL;
  cmp_ui_node_t *split_node = NULL;
  cmp_f2_button_t *btn_data = NULL;
  int res;

  res = cmp_f2_menu_button_create(&menu_node, "Menu", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  btn_data = (cmp_f2_button_t *)menu_node->properties;
  ASSERT_EQ(CMP_F2_BUTTON_TYPE_MENU, btn_data->type);
  ASSERT(btn_data->chevron_node != NULL);

  res = cmp_f2_button_set_menu_open(menu_node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, btn_data->is_menu_open);
  cmp_ui_node_destroy(menu_node);

  res = cmp_f2_split_button_create(&split_node, "Split", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  btn_data = (cmp_f2_button_t *)split_node->properties;
  ASSERT_EQ(CMP_F2_BUTTON_TYPE_SPLIT, btn_data->type);
  ASSERT(btn_data->divider_node != NULL);
  ASSERT(btn_data->chevron_node != NULL);
  cmp_ui_node_destroy(split_node);

  PASS();
}

TEST test_f2_toggle_button(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_button_t *btn_data = NULL;
  int res;

  res = cmp_f2_toggle_button_create(&node, "Toggle", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  btn_data = (cmp_f2_button_t *)node->properties;
  ASSERT_EQ(CMP_F2_BUTTON_TYPE_TOGGLE, btn_data->type);

  res = cmp_f2_button_set_toggled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, btn_data->is_toggled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_button_invalid_args(void) {
  int res;
  cmp_ui_node_t *node = NULL;

  res = cmp_f2_button_create(NULL, "Fail", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_box_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_button_set_variant(node, CMP_F2_BUTTON_VARIANT_PRIMARY);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_button_set_toggled(node, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_ui_node_destroy(node);

  /* Test setting toggled on a standard button */
  res = cmp_f2_button_create(&node, "Standard", NULL);
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_f2_button_set_toggled(node, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res);
  cmp_ui_node_destroy(node);

  PASS();
}

SUITE(cmp_f2_button_suite) {
  RUN_TEST(test_f2_button_create);
  RUN_TEST(test_f2_button_variants_and_sizes);
  RUN_TEST(test_f2_button_states_and_shapes);
  RUN_TEST(test_f2_compound_button);
  RUN_TEST(test_f2_menu_split_button);
  RUN_TEST(test_f2_toggle_button);
  RUN_TEST(test_f2_button_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_button_suite);
  GREATEST_MAIN_END();
}
