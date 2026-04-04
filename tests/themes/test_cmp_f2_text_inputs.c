#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_text_inputs.h"
/* clang-format on */

TEST test_f2_text_input(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_text_input_t *data = NULL;
  int res;

  res = cmp_f2_text_input_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_text_input_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_VARIANT_OUTLINE, data->variant);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_SIZE_MEDIUM, data->size);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_REST, data->state);
  ASSERT_EQ(0, data->is_password);

  res = cmp_f2_text_input_set_variant(node, CMP_F2_TEXT_INPUT_VARIANT_FILLED);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_VARIANT_FILLED, data->variant);

  res = cmp_f2_text_input_set_size(node, CMP_F2_TEXT_INPUT_SIZE_LARGE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_SIZE_LARGE, data->size);

  res = cmp_f2_text_input_set_state(node, CMP_F2_TEXT_INPUT_STATE_FOCUS);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_FOCUS, data->state);

  res = cmp_f2_text_input_set_password_mode(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_password);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_textarea(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_textarea_t *data = NULL;
  int res;

  res = cmp_f2_textarea_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_textarea_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_SIZE_MEDIUM, data->size);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_REST, data->state);
  ASSERT_EQ(CMP_F2_TEXTAREA_RESIZE_NONE, data->resize_mode);
  ASSERT_EQ(0, data->auto_grow);

  res = cmp_f2_textarea_set_size(node, CMP_F2_TEXT_INPUT_SIZE_SMALL);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_SIZE_SMALL, data->size);

  res = cmp_f2_textarea_set_state(node, CMP_F2_TEXT_INPUT_STATE_DISABLED);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_DISABLED, data->state);

  res = cmp_f2_textarea_set_resize_mode(node, CMP_F2_TEXTAREA_RESIZE_VERTICAL);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXTAREA_RESIZE_VERTICAL, data->resize_mode);

  res = cmp_f2_textarea_set_auto_grow(node, 1, 200.0f);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->auto_grow);
  ASSERT_EQ(200.0f, data->max_height);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_field(void) {
  cmp_ui_node_t *node = NULL;
  cmp_ui_node_t *input_node = NULL;
  cmp_f2_field_t *data = NULL;
  int res;

  res = cmp_ui_text_input_create(&input_node);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_f2_field_create(&node, input_node, "First Name");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_field_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT(data->label_node != NULL);
  ASSERT_EQ(CMP_F2_FIELD_LABEL_TOP, data->label_position);
  ASSERT_EQ(0, data->is_required);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_REST, data->validation_state);

  res = cmp_f2_field_set_label_position(node, CMP_F2_FIELD_LABEL_LEFT);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_FIELD_LABEL_LEFT, data->label_position);

  res = cmp_f2_field_set_required(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_required);

  res = cmp_f2_field_set_help_text(node, "Enter your legal first name.");
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(data->help_text_node != NULL);

  res = cmp_f2_field_set_validation_message(node, "This field is required.",
                                            CMP_F2_TEXT_INPUT_STATE_INVALID);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_INVALID, data->validation_state);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_searchbox(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_searchbox_t *data = NULL;
  int res;

  res = cmp_f2_searchbox_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_searchbox_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_SIZE_MEDIUM, data->size);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_REST, data->state);
  ASSERT_EQ(0, data->is_collapsed);

  res = cmp_f2_searchbox_set_size(node, CMP_F2_TEXT_INPUT_SIZE_LARGE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_SIZE_LARGE, data->size);

  res = cmp_f2_searchbox_set_state(node, CMP_F2_TEXT_INPUT_STATE_FOCUS);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_STATE_FOCUS, data->state);

  res = cmp_f2_searchbox_set_collapsed_mode(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_collapsed);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_text_inputs_invalid_args(void) {
  int res;
  cmp_ui_node_t *input_node = NULL;

  res = cmp_f2_text_input_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_textarea_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_ui_text_input_create(&input_node);
  ASSERT_EQ(CMP_SUCCESS, res);
  res = cmp_f2_field_create(NULL, input_node, "Label");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);
  cmp_ui_node_destroy(input_node);

  res = cmp_f2_searchbox_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(cmp_f2_text_inputs_suite) {
  RUN_TEST(test_f2_text_input);
  RUN_TEST(test_f2_textarea);
  RUN_TEST(test_f2_field);
  RUN_TEST(test_f2_searchbox);
  RUN_TEST(test_f2_text_inputs_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_text_inputs_suite);
  GREATEST_MAIN_END();
}
