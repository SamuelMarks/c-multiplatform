#include <stdlib.h>
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include "themes/cmp_f2_dropdowns.h"
/* clang-format on */

TEST test_f2_dropdown(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_dropdown_t *data = NULL;
  int res;

  res = cmp_f2_dropdown_create(&node, CMP_F2_DROPDOWN_SELECTION_SINGLE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(node != NULL);

  data = (cmp_f2_dropdown_t *)node->properties;
  ASSERT(data != NULL);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_VARIANT_OUTLINE, data->variant);
  ASSERT_EQ(CMP_F2_DROPDOWN_SELECTION_SINGLE, data->selection_mode);
  ASSERT_EQ(0, data->is_open);
  ASSERT_EQ(0, data->is_disabled);
  ASSERT(data->trigger_node != NULL);
  ASSERT(data->chevron_node != NULL);

  res = cmp_f2_dropdown_set_variant(node, CMP_F2_TEXT_INPUT_VARIANT_UNDERLINE);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_TEXT_INPUT_VARIANT_UNDERLINE, data->variant);

  res = cmp_f2_dropdown_set_open(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_open);

  res = cmp_f2_dropdown_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_combobox(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_combobox_t *data = NULL;
  int res;

  res = cmp_f2_combobox_create(&node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_combobox_t *)node->properties;
  ASSERT_EQ(1, data->is_freeform);
  ASSERT(data->input_node != NULL);

  res = cmp_f2_combobox_set_open(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_open);

  res = cmp_f2_combobox_set_disabled(node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, data->is_disabled);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_listbox(void) {
  cmp_ui_node_t *listbox = NULL;
  cmp_ui_node_t *option = NULL;
  cmp_ui_node_t *group = NULL;
  cmp_f2_listbox_option_t *opt_data = NULL;
  int res;

  res = cmp_f2_listbox_create(&listbox);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_FLEX_COLUMN, listbox->layout->direction);

  res = cmp_f2_listbox_option_create(&option, "Option 1");
  ASSERT_EQ(CMP_SUCCESS, res);
  opt_data = (cmp_f2_listbox_option_t *)option->properties;
  ASSERT_EQ(0, opt_data->is_selected);
  ASSERT(opt_data->content_node != NULL);

  res = cmp_f2_listbox_option_set_selected(option, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, opt_data->is_selected);

  res = cmp_f2_listbox_option_set_disabled(option, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, opt_data->is_disabled);

  res = cmp_f2_listbox_group_create(&group, "Header");
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_ui_node_add_child(listbox, option);
  cmp_ui_node_add_child(listbox, group);

  cmp_ui_node_destroy(listbox);
  PASS();
}

TEST test_f2_color_picker(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_color_picker_t *data = NULL;
  int res;

  res = cmp_f2_color_picker_create(&node, 0xFF00FF00);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_color_picker_t *)node->properties;
  ASSERT_EQ(0xFF00FF00, data->current_color);

  res = cmp_f2_color_picker_set_color(node, 0xFFFF0000);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0xFFFF0000, data->current_color);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_calendar_and_datepicker(void) {
  cmp_ui_node_t *cal_node = NULL;
  cmp_ui_node_t *dp_node = NULL;
  cmp_f2_calendar_t *cal_data = NULL;
  cmp_f2_date_picker_t *dp_data = NULL;
  int res;

  res = cmp_f2_calendar_create(&cal_node);
  ASSERT_EQ(CMP_SUCCESS, res);
  cal_data = (cmp_f2_calendar_t *)cal_node->properties;
  ASSERT_EQ(CMP_F2_CALENDAR_VIEW_MONTH, cal_data->view_mode);

  res = cmp_f2_calendar_set_view(cal_node, CMP_F2_CALENDAR_VIEW_YEAR);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(CMP_F2_CALENDAR_VIEW_YEAR, cal_data->view_mode);

  res = cmp_f2_calendar_set_date(cal_node, 2026, 4, 2);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2026, cal_data->selected_year);
  ASSERT_EQ(4, cal_data->selected_month);
  ASSERT_EQ(2, cal_data->selected_day);
  cmp_ui_node_destroy(cal_node);

  res = cmp_f2_date_picker_create(&dp_node);
  ASSERT_EQ(CMP_SUCCESS, res);
  dp_data = (cmp_f2_date_picker_t *)dp_node->properties;
  ASSERT_EQ(0, dp_data->is_open);
  ASSERT(dp_data->input_node != NULL);

  res = cmp_f2_date_picker_set_open(dp_node, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, dp_data->is_open);
  cmp_ui_node_destroy(dp_node);

  PASS();
}

TEST test_f2_time_picker(void) {
  cmp_ui_node_t *node = NULL;
  cmp_f2_time_picker_t *data = NULL;
  int res;

  res = cmp_f2_time_picker_create(&node);
  ASSERT_EQ(CMP_SUCCESS, res);

  data = (cmp_f2_time_picker_t *)node->properties;
  ASSERT_EQ(12, data->selected_hour);
  ASSERT_EQ(0, data->selected_minute);
  ASSERT_EQ(0, data->is_pm);

  res = cmp_f2_time_picker_set_time(node, 4, 30, 1);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(4, data->selected_hour);
  ASSERT_EQ(30, data->selected_minute);
  ASSERT_EQ(1, data->is_pm);

  cmp_ui_node_destroy(node);
  PASS();
}

TEST test_f2_dropdowns_invalid_args(void) {
  int res;

  res = cmp_f2_dropdown_create(NULL, CMP_F2_DROPDOWN_SELECTION_SINGLE);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_combobox_create(NULL, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_listbox_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_color_picker_create(NULL, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_calendar_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_date_picker_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_f2_time_picker_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  PASS();
}

SUITE(cmp_f2_dropdowns_suite) {
  RUN_TEST(test_f2_dropdown);
  RUN_TEST(test_f2_combobox);
  RUN_TEST(test_f2_listbox);
  RUN_TEST(test_f2_color_picker);
  RUN_TEST(test_f2_calendar_and_datepicker);
  RUN_TEST(test_f2_time_picker);
  RUN_TEST(test_f2_dropdowns_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_f2_dropdowns_suite);
  GREATEST_MAIN_END();
}
