/**
 * @file test_material2_selection_controls.c
 * @brief Unit tests for Material Design 2 Selection Controls.
 */

/* clang-format off */
#include "material2/md2_selection_controls.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_test_mock_mem.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_checkbox(void) {
  struct md2_checkbox *checkbox = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = md2_checkbox_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_checkbox_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_checkbox_get_component(checkbox, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_checkbox_get_component(NULL, &comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_checkbox_create(&checkbox);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, checkbox);

  rc = md2_checkbox_get_component(checkbox, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_checkbox_get_component(checkbox, &comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, comp);

  rc = md2_checkbox_destroy(checkbox);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_radio_button(void) {
  struct md2_radio_button *radio = NULL;
  struct ui_radio_group_base *base = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = md2_radio_button_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_radio_button_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_radio_button_get_base(radio, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_radio_button_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_radio_button_create(&radio);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, radio);

  rc = md2_radio_button_get_base(radio, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_radio_button_get_base(radio, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_radio_button_destroy(radio);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_switch(void) {
  struct md2_switch *sw = NULL;
  struct ui_slide_toggle_base *base = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = md2_switch_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_switch_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_switch_get_base(sw, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_switch_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_switch_create(&sw);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, sw);

  rc = md2_switch_get_base(sw, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_switch_get_base(sw, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_switch_destroy(sw);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_slider(void) {
  struct md2_slider *slider = NULL;
  struct ui_slider_base *base = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = md2_slider_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_slider_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_slider_get_base(slider, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_slider_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_slider_create(&slider);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, slider);

  rc = md2_slider_get_base(slider, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_slider_get_base(slider, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_slider_destroy(slider);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_selection_controls_oom(void) {
  struct md2_checkbox *checkbox = NULL;
  struct md2_radio_button *radio = NULL;
  struct md2_switch *sw = NULL;
  struct md2_slider *slider = NULL;
  ui_error_t rc;
  int i;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_checkbox_create(&checkbox);
    if (rc == UI_ERROR_NONE) {
      md2_checkbox_destroy(checkbox);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_radio_button_create(&radio);
    if (rc == UI_ERROR_NONE) {
      md2_radio_button_destroy(radio);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_switch_create(&sw);
    if (rc == UI_ERROR_NONE) {
      md2_switch_destroy(sw);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_slider_create(&slider);
    if (rc == UI_ERROR_NONE) {
      md2_slider_destroy(slider);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  PASS();
}

SUITE(material2_selection_controls_suite) {
  RUN_TEST(test_md2_checkbox);
  RUN_TEST(test_md2_radio_button);
  RUN_TEST(test_md2_switch);
  RUN_TEST(test_md2_slider);
  RUN_TEST(test_md2_selection_controls_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_selection_controls_suite);
  GREATEST_MAIN_END();
}
