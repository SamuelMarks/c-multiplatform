/**
 * @file test_cupertino_slider.c
 * @brief Unit tests for Cupertino Slider component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_slider.h"
#include "ui_engine.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_slider_suite);

TEST test_slider_lifecycle_and_cva(void) {
  struct cupertino_slider *slider = NULL;
  struct cupertino_slider *slider2 = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_slider_base *base = NULL;
  struct ui_slider_base *saved_base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  float val = 0.0f;
  union ui_signal_payload payload;
  ui_error_t rc;

  memset(&payload, 0, sizeof(payload));

  /* Invalid arguments */
  rc = cupertino_slider_create(NULL, 10.0f, 0.0f, &slider, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_create(dummy_engine, 10.0f, 10.0f, &slider, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with CVA */
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(slider != NULL);
  ASSERT(cva != NULL);
  ASSERT(fabs(slider->min - 0.0f) < 1e-5f);
  ASSERT(fabs(slider->max - 100.0f) < 1e-5f);

  /* Valid creation without CVA */
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(slider2 != NULL);

  /* Get base */
  rc = cupertino_slider_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_get_base(slider, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_get_base(slider, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Set / get value */
  rc = cupertino_slider_set_value(NULL, 50.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_get_value(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_get_value(slider, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_slider_set_value(slider, 50.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_slider_get_value(slider, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 50.0f) < 1e-5f);

  /* Clamping beyond bounds */
  rc = cupertino_slider_set_value(slider, -20.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_slider_get_value(slider, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 0.0f) < 1e-5f);

  rc = cupertino_slider_set_value(slider, 150.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_slider_get_value(slider, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 100.0f) < 1e-5f);

  /* Step increment */
  rc = cupertino_slider_set_step(NULL, 5.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_set_step(slider, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_set_step(slider, 5.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(slider->step - 5.0f) < 1e-5f);

  /* Disabled state */
  rc = cupertino_slider_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_set_disabled(slider, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_slider_set_disabled(slider, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA methods */
  ASSERT(cva->write_value != NULL);
  ASSERT(cva->set_disabled_state != NULL);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  payload.float_val = 75.0f;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_slider_get_value(slider, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 75.0f) < 1e-5f);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy slider2 with base == NULL */
  saved_base = slider2->base;
  slider2->base = NULL;
  rc = cupertino_slider_destroy(slider2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_slider_base_destroy(saved_base);

  /* Destroy */
  rc = cupertino_slider_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_slider_destroy(slider);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_slider_oom_mock(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_slider *slider = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;
  extern int g_cupertino_slider_mock_set_min_fail;
  extern int g_cupertino_slider_mock_set_max_fail;
  extern int g_cupertino_slider_mock_destroy_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, slider);

  g_malloc_fail_countdown = 1;
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, slider);

  g_malloc_fail_countdown = -1;

  /* Mock set_min failure */
  g_cupertino_slider_mock_set_min_fail = 1;
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, slider);

  /* Mock set_min failure + destroy failure */
  g_cupertino_slider_mock_destroy_fail = 1;
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, NULL);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, slider);
  g_cupertino_slider_mock_set_min_fail = 0;
  g_cupertino_slider_mock_destroy_fail = 0;

  /* Mock set_max failure */
  g_cupertino_slider_mock_set_max_fail = 1;
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, slider);

  /* Mock set_max failure + destroy failure */
  g_cupertino_slider_mock_destroy_fail = 1;
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, NULL);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, slider);
  g_cupertino_slider_mock_set_max_fail = 0;
  g_cupertino_slider_mock_destroy_fail = 0;

  /* Mock destroy failure */
  rc = cupertino_slider_create(dummy_engine, 0.0f, 100.0f, &slider, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_slider_mock_destroy_fail = 1;
  rc = cupertino_slider_destroy(slider);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_slider_mock_destroy_fail = 0;
#endif

  PASS();
}

SUITE(cupertino_slider_suite) {
  RUN_TEST(test_slider_lifecycle_and_cva);
  RUN_TEST(test_slider_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_slider_suite);
  GREATEST_MAIN_END();
}
