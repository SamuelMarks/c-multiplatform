/**
 * @file test_material2_selection_controls.c
 * @brief Unit tests for Material Design 2 Selection Controls.
 */

/* clang-format off */
#include "material2/md2_selection_controls.h"
#include "ui_error.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_checkbox_lifecycle(void) {
  struct md2_checkbox *checkbox = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = md2_checkbox_create(&checkbox);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, checkbox);

  rc = md2_checkbox_get_component(checkbox, &comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, comp);

  rc = md2_checkbox_destroy(checkbox);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_checkbox_invalid_args(void) {
  struct md2_checkbox *checkbox = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = md2_checkbox_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_checkbox_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_checkbox_get_component(NULL, &comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_checkbox_create(&checkbox);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_checkbox_get_component(checkbox, NULL);
  /* Component fetch with NULL out_comp is handled by base or directly invalid
   */
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_checkbox_destroy(checkbox);
  PASS();
}

TEST test_md2_radio_button_lifecycle(void) {
  struct md2_radio_button *radio = NULL;
  struct ui_radio_group_base *base = NULL;
  ui_error_t rc;

  rc = md2_radio_button_create(&radio);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, radio);

  rc = md2_radio_button_get_base(radio, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_radio_button_destroy(radio);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_radio_button_invalid_args(void) {
  struct md2_radio_button *radio = NULL;
  struct ui_radio_group_base *base = NULL;
  ui_error_t rc;

  rc = md2_radio_button_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_radio_button_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_radio_button_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_radio_button_create(&radio);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_radio_button_get_base(radio, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_radio_button_destroy(radio);
  PASS();
}

TEST test_md2_switch_lifecycle(void) {
  struct md2_switch *sw = NULL;
  struct ui_slide_toggle_base *base = NULL;
  ui_error_t rc;

  rc = md2_switch_create(&sw);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, sw);

  rc = md2_switch_get_base(sw, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_switch_destroy(sw);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_switch_invalid_args(void) {
  struct md2_switch *sw = NULL;
  struct ui_slide_toggle_base *base = NULL;
  ui_error_t rc;

  rc = md2_switch_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_switch_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_switch_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_switch_create(&sw);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_switch_get_base(sw, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_switch_destroy(sw);
  PASS();
}

TEST test_md2_slider_lifecycle(void) {
  struct md2_slider *slider = NULL;
  struct ui_slider_base *base = NULL;
  ui_error_t rc;

  rc = md2_slider_create(&slider);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, slider);

  rc = md2_slider_get_base(slider, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_slider_destroy(slider);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_slider_invalid_args(void) {
  struct md2_slider *slider = NULL;
  struct ui_slider_base *base = NULL;
  ui_error_t rc;

  rc = md2_slider_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_slider_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_slider_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_slider_create(&slider);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_slider_get_base(slider, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_slider_destroy(slider);
  PASS();
}

SUITE(material2_selection_controls_suite) {
  RUN_TEST(test_md2_checkbox_lifecycle);
  RUN_TEST(test_md2_checkbox_invalid_args);
  RUN_TEST(test_md2_radio_button_lifecycle);
  RUN_TEST(test_md2_radio_button_invalid_args);
  RUN_TEST(test_md2_switch_lifecycle);
  RUN_TEST(test_md2_switch_invalid_args);
  RUN_TEST(test_md2_slider_lifecycle);
  RUN_TEST(test_md2_slider_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_selection_controls_suite);
  GREATEST_MAIN_END();
}
