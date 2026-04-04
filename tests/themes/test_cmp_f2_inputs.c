#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_inputs.h"
/* clang-format on */

TEST test_f2_checkbox(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_checkbox_t *data = NULL;
  int res;

  res = cmp_f2_checkbox_create(&node, "Check me");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_checkbox_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_CHECKBOX_SIZE_MEDIUM, data->size);
  ASSERT_EQ(CMP_F2_CHECKBOX_STATE_UNCHECKED, data->state);
  ASSERT_EQ(CMP_F2_LABEL_POSITION_AFTER, data->label_position);
  ASSERT_EQ(0, data->is_disabled);

  res = cmp_f2_checkbox_set_state(node, CMP_F2_CHECKBOX_STATE_INDETERMINATE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_CHECKBOX_STATE_INDETERMINATE, data->state);

  res = cmp_f2_checkbox_set_size(node, CMP_F2_CHECKBOX_SIZE_LARGE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_CHECKBOX_SIZE_LARGE, data->size);

  res = cmp_f2_checkbox_set_label_position(node, CMP_F2_LABEL_POSITION_BEFORE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_LABEL_POSITION_BEFORE, data->label_position);

  res = cmp_f2_checkbox_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_radio(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *group = NULL;
  cmp_f2_radio_t *data = NULL;
  int res;

  res = cmp_f2_radio_create(&node, "Radio");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_radio_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(0, data->is_selected);

  res = cmp_f2_radio_set_selected(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_selected);

  res = cmp_f2_radio_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);

  /* Test group creation */
  res = cmp_f2_radio_group_create(&group, 1); /* horizontal */
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_ROW, group->layout->direction);
  cmp_ui_node_destroy(group);

  PASS();
}

TEST test_f2_toggle(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_toggle_t *data = NULL;
  int res;

  res = cmp_f2_toggle_create(&node, "Toggle");
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_toggle_t *)node->properties;
  ASSERT_EQ(0, data->is_on);

  res = cmp_f2_toggle_set_on(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_on);

  res = cmp_f2_toggle_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_slider(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_slider_t *data = NULL;
  int res;

  res = cmp_f2_slider_create(&node, CMP_F2_SLIDER_HORIZONTAL);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_slider_t *)node->properties;
  ASSERT_EQ(CMP_F2_SLIDER_HORIZONTAL, data->orientation);
  ASSERT_EQ(0, data->is_range);
  ASSERT_EQ(100.0f, data->max_val);

  res = cmp_f2_slider_set_range_mode(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_range);

  res = cmp_f2_slider_set_bounds(node, 10.0f, 50.0f, 5.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(10.0f, data->min_val);
  ASSERT_EQ(50.0f, data->max_val);
  ASSERT_EQ(5.0f, data->step_val);
  ASSERT_EQ(1, data->is_stepped);

  res = cmp_f2_slider_set_value(node, 15.0f, 40.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(15.0f, data->value1);
  ASSERT_EQ(40.0f, data->value2);

  res = cmp_f2_slider_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_spin_button(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_spin_button_t *data = NULL;
  int res;

  res = cmp_f2_spin_button_create(&node, 50.0f);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_spin_button_t *)node->properties;
  ASSERT_EQ(50.0f, data->value);

  res = cmp_f2_spin_button_set_bounds(node, 0.0f, 10.0f, 0.5f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(10.0f, data->max_val);

  res = cmp_f2_spin_button_set_value(node, 2.5f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2.5f, data->value);

  res = cmp_f2_spin_button_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_inputs_invalid_args(void) {
  int res;

  res = cmp_f2_checkbox_create(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_checkbox_set_state(NULL, CMP_F2_CHECKBOX_STATE_CHECKED);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_radio_create(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_toggle_create(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_slider_create(NULL, CMP_F2_SLIDER_HORIZONTAL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_spin_button_create(NULL, 0.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(cmp_f2_inputs_suite) {
  RUN_TEST(test_f2_checkbox);
  RUN_TEST(test_f2_radio);
  RUN_TEST(test_f2_toggle);
  RUN_TEST(test_f2_slider);
  RUN_TEST(test_f2_spin_button);
  RUN_TEST(test_f2_inputs_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_inputs_suite);
  GREATEST_MAIN_END();
}
