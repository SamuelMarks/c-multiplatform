/**
 * @file test_cupertino_switch.c
 * @brief Unit tests for Cupertino Switch component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_switch.h"
#include "ui_engine.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_switch_suite);

TEST test_switch_lifecycle_and_cva(void) {
  struct cupertino_switch *sw = NULL;
  struct cupertino_switch *sw2 = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_slide_toggle_base *base = NULL;
  struct ui_slide_toggle_base *saved_base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  int checked = 0;
  union ui_signal_payload payload;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_haptics_mock_return;
#endif

  memset(&payload, 0, sizeof(payload));

  /* Invalid arguments */
  rc = cupertino_switch_create(NULL, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with CVA */
  rc = cupertino_switch_create(dummy_engine, &sw, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sw != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(0, sw->show_accessibility_labels);
  ASSERT_EQ(0, sw->is_dragging);
  ASSERT(fabs(sw->thumb_width - CUPERTINO_SWITCH_THUMB_DIAMETER) < 1e-5f);

  /* Creation without CVA */
  rc = cupertino_switch_create(dummy_engine, &sw2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(sw2 != NULL);

  /* Get base */
  rc = cupertino_switch_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_get_base(sw, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_get_base(sw, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Checked state manipulation */
  rc = cupertino_switch_set_checked(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_get_checked(NULL, &checked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_get_checked(sw, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_switch_get_checked(sw, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, checked);

  /* Set checked to 1 */
  rc = cupertino_switch_set_checked(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_switch_get_checked(sw, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, checked);

  /* Setting same checked value should be no-op for haptics */
  rc = cupertino_switch_set_checked(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test haptic failure percolation */
  g_cupertino_haptics_mock_return = (int)UI_ERROR_UNKNOWN;
  rc = cupertino_switch_set_checked(sw, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_haptics_mock_return = -1;
#endif

  /* Disabled state */
  rc = cupertino_switch_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_set_disabled(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_switch_set_disabled(sw, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Accessibility labels */
  rc = cupertino_switch_set_show_accessibility_labels(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_set_show_accessibility_labels(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, sw->show_accessibility_labels);

  /* Drag state and thumb stretch */
  rc = cupertino_switch_set_dragging(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_set_dragging(sw, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, sw->is_dragging);
  ASSERT(fabs(sw->thumb_width - CUPERTINO_SWITCH_THUMB_DRAG_WIDTH) < 1e-5f);

  rc = cupertino_switch_set_dragging(sw, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, sw->is_dragging);
  ASSERT(fabs(sw->thumb_width - CUPERTINO_SWITCH_THUMB_DIAMETER) < 1e-5f);

  /* CVA methods */
  ASSERT(cva->write_value != NULL);
  ASSERT(cva->set_disabled_state != NULL);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  payload.bool_val = 0;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_switch_get_checked(sw, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, checked);

  payload.bool_val = 1;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_switch_get_checked(sw, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, checked);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test base failure percolation on set_checked */
  saved_base = sw2->base;
  sw2->base = NULL;
  rc = cupertino_switch_set_checked(sw2, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy with sw2->base NULL */
  rc = cupertino_switch_destroy(sw2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_slide_toggle_base_destroy(saved_base);

  /* Destroy */
  rc = cupertino_switch_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_switch_destroy(sw);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_switch_oom_mock(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_switch *sw = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;

  g_malloc_fail_countdown = 0;
  rc = cupertino_switch_create(dummy_engine, &sw, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, sw);

  g_malloc_fail_countdown = 1;
  rc = cupertino_switch_create(dummy_engine, &sw, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, sw);

  g_malloc_fail_countdown = -1;
#endif

  PASS();
}

SUITE(cupertino_switch_suite) {
  RUN_TEST(test_switch_lifecycle_and_cva);
  RUN_TEST(test_switch_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_switch_suite);
  GREATEST_MAIN_END();
}
