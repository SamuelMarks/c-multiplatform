/**
 * @file test_material3_cva.c
 * @brief Invariant tests verifying 100% CVA synchronization for Material 3 form
 * controls.
 */

/* clang-format off */
#include "greatest.h"
#include "material3/md3_checkbox.h"
#include "material3/md3_radio_button.h"
#include "material3/md3_slider.h"
#include "material3/md3_switch.h"
#include "material3/md3_text_field.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include <string.h>
/* clang-format on */

TEST test_md3_cva_checkbox(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_checkbox *cb = NULL;
  struct ui_control_value_accessor *cva = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  rc = md3_checkbox_create(dummy_engine, &cb, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cb != NULL);

  if (cva && cva->write_value) {
    memset(&payload, 0, sizeof(payload));
    payload.int_val = 1;
    rc = cva->write_value(cb, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_checkbox_destroy(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_cva_switch(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_switch *sw = NULL;
  struct ui_control_value_accessor *cva = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  rc = md3_switch_create(dummy_engine, &sw, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sw != NULL);

  if (cva && cva->write_value) {
    memset(&payload, 0, sizeof(payload));
    payload.int_val = 1;
    rc = cva->write_value(sw, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_switch_destroy(sw);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_cva_text_field(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_text_field *tf = NULL;
  struct ui_control_value_accessor *cva = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  rc = md3_text_field_create(dummy_engine, MD3_TEXT_FIELD_OUTLINED, &tf, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tf != NULL);

  if (cva && cva->write_value) {
    memset(&payload, 0, sizeof(payload));
    payload.ptr_val = (void *)"test_input";
    rc = cva->write_value(tf, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_text_field_destroy(tf);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_cva_slider(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_slider *slider = NULL;
  struct ui_control_value_accessor *cva = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  rc = md3_slider_create(dummy_engine, MD3_SLIDER_CONTINUOUS, 0.0f, 100.0f,
                         &slider, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(slider != NULL);

  if (cva && cva->write_value) {
    memset(&payload, 0, sizeof(payload));
    payload.float_val = 50.0f;
    rc = cva->write_value(slider->base, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_slider_destroy(slider);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_cva_radio(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  struct md3_radio_group *rg = NULL;
  struct ui_control_value_accessor *cva = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  rc = md3_radio_group_create(dummy_engine, &rg, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(rg != NULL);

  if (cva && cva->write_value) {
    memset(&payload, 0, sizeof(payload));
    payload.int_val = 0;
    rc = cva->write_value(rg, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = md3_radio_group_destroy(rg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

SUITE(md3_cva_suite) {
  RUN_TEST(test_md3_cva_checkbox);
  RUN_TEST(test_md3_cva_switch);
  RUN_TEST(test_md3_cva_text_field);
  RUN_TEST(test_md3_cva_slider);
  RUN_TEST(test_md3_cva_radio);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_cva_suite);
  GREATEST_MAIN_END();
}
