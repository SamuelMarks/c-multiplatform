/**
 * @file test_cupertino_stepper.c
 * @brief Unit tests for Cupertino Stepper component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_stepper.h"
#include "ui_engine.h"
#include <math.h>
#include <string.h>
/* clang-format on */

SUITE(cupertino_stepper_suite);

TEST test_stepper_lifecycle_and_cva(void) {
  struct cupertino_stepper *stepper = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_spin_button_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  double val = 0.0;
  union ui_signal_payload payload;
  ui_error_t rc;

  memset(&payload, 0, sizeof(payload));

  /* Invalid arguments on create */
  rc = cupertino_stepper_create(NULL, 0.0, 100.0, 1.0, &stepper, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_create(dummy_engine, 100.0, 0.0, 1.0, &stepper, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 0.0, &stepper, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, -1.0, &stepper, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation without out_cva */
  rc = cupertino_stepper_create(dummy_engine, 0.0, 10.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(stepper != NULL);
  rc = cupertino_stepper_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  stepper = NULL;

  /* Valid creation with out_cva */
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 5.0, &stepper, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(stepper != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(0, stepper->is_disabled);
  ASSERT_EQ(1, stepper->auto_repeat);
  ASSERT(fabs(stepper->value - 0.0) < 1e-5);
  ASSERT(fabs(stepper->step - 5.0) < 1e-5);

  /* Base query */
  rc = cupertino_stepper_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_get_base(stepper, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_get_base(stepper, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Value get/set */
  rc = cupertino_stepper_get_value(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_get_value(stepper, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 0.0) < 1e-5);

  rc = cupertino_stepper_set_value(NULL, 50.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_value(stepper, 50.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 50.0) < 1e-5);

  /* Clamping min / max */
  rc = cupertino_stepper_set_value(stepper, -20.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 0.0) < 1e-5);

  rc = cupertino_stepper_set_value(stepper, 200.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 100.0) < 1e-5);

  /* Increment / Decrement */
  rc = cupertino_stepper_set_value(stepper, 10.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_stepper_increment(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_increment(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 15.0) < 1e-5);

  rc = cupertino_stepper_decrement(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_decrement(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 10.0) < 1e-5);

  /* Step manipulation */
  rc = cupertino_stepper_set_step(NULL, 2.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_step(stepper, 0.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_step(stepper, -1.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_step(stepper, 2.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(stepper->step - 2.0) < 1e-5);

  /* Range manipulation */
  rc = cupertino_stepper_set_range(NULL, 10.0, 50.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_range(stepper, 50.0, 10.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_range(stepper, 20.0, 40.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 20.0) < 1e-5);

  rc = cupertino_stepper_set_value(stepper, 35.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_set_range(stepper, 10.0, 30.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 30.0) < 1e-5);

  /* Range manipulation where current value is inside new range */
  rc = cupertino_stepper_set_range(stepper, 10.0, 50.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Auto repeat toggle */
  rc = cupertino_stepper_set_auto_repeat(NULL, 0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_auto_repeat(stepper, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, stepper->auto_repeat);
  rc = cupertino_stepper_set_auto_repeat(stepper, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, stepper->auto_repeat);

  /* Disabled state */
  rc = cupertino_stepper_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_set_disabled(stepper, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, stepper->is_disabled);

  /* Increment/decrement while disabled should be no-op */
  rc = cupertino_stepper_increment(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_decrement(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Re-enable */
  rc = cupertino_stepper_set_disabled(stepper, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, stepper->is_disabled);

  /* Timer on_tick */
  rc = cupertino_stepper_on_tick(NULL, 100.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_on_tick(stepper, -5.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_on_tick(stepper, 100.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Disable and tick should be no-op */
  rc = cupertino_stepper_set_disabled(stepper, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_on_tick(stepper, 100.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_set_disabled(stepper, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Disable auto_repeat and tick should be no-op */
  rc = cupertino_stepper_set_auto_repeat(stepper, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_on_tick(stepper, 100.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA write value and set disabled */
  ASSERT(cva->write_value != NULL);
  ASSERT(cva->set_disabled_state != NULL);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  payload.float_val = 25.0f;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_stepper_get_value(stepper, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(fabs(val - 25.0) < 1e-5);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, stepper->is_disabled);

  rc = cva->set_disabled_state(cva->component, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, stepper->is_disabled);

  /* Destruction */
  rc = cupertino_stepper_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_stepper_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_stepper_oom_mock(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_stepper *stepper = NULL;
  ui_error_t rc;
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;
  extern int g_cupertino_stepper_mock_set_min_fail;
  extern int g_cupertino_stepper_mock_set_max_fail;
  extern int g_cupertino_stepper_mock_set_step_fail;
  extern int g_cupertino_stepper_mock_set_value_fail;
  extern int g_cupertino_stepper_mock_destroy_fail;
  extern int g_cupertino_stepper_mock_increment_fail;
  extern int g_cupertino_stepper_mock_decrement_fail;
  extern int g_cupertino_stepper_mock_set_disabled_fail;
  extern int g_cupertino_stepper_mock_on_tick_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, stepper);

  g_malloc_fail_countdown = 1;
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, stepper);
  g_malloc_fail_countdown = -1;

  /* Creation step failures */
  g_cupertino_stepper_mock_set_min_fail = 1;
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, stepper);
  g_cupertino_stepper_mock_set_min_fail = 0;

  g_cupertino_stepper_mock_set_max_fail = 1;
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, stepper);
  g_cupertino_stepper_mock_set_max_fail = 0;

  g_cupertino_stepper_mock_set_step_fail = 1;
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, stepper);
  g_cupertino_stepper_mock_set_step_fail = 0;

  g_cupertino_stepper_mock_set_value_fail = 1;
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT_EQ(NULL, stepper);
  g_cupertino_stepper_mock_set_value_fail = 0;

  /* Method failure paths on valid stepper */
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_NEQ(NULL, stepper);

  g_cupertino_stepper_mock_set_value_fail = 1;
  rc = cupertino_stepper_set_value(stepper, 5.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_set_value_fail = 0;

  g_cupertino_stepper_mock_increment_fail = 1;
  rc = cupertino_stepper_increment(stepper);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_increment_fail = 0;

  g_cupertino_stepper_mock_decrement_fail = 1;
  rc = cupertino_stepper_decrement(stepper);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_decrement_fail = 0;

  g_cupertino_stepper_mock_set_step_fail = 1;
  rc = cupertino_stepper_set_step(stepper, 2.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_set_step_fail = 0;

  g_cupertino_stepper_mock_set_min_fail = 1;
  rc = cupertino_stepper_set_range(stepper, 10.0, 50.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_set_min_fail = 0;

  g_cupertino_stepper_mock_set_max_fail = 1;
  rc = cupertino_stepper_set_range(stepper, 10.0, 50.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_set_max_fail = 0;

  /* Failures when set_range clamps value below min or above max */
  stepper->value = 5.0;
  g_cupertino_stepper_mock_set_value_fail = 1;
  rc = cupertino_stepper_set_range(stepper, 10.0, 50.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  stepper->value = 60.0;
  rc = cupertino_stepper_set_range(stepper, 10.0, 50.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_set_value_fail = 0;

  g_cupertino_stepper_mock_set_disabled_fail = 1;
  rc = cupertino_stepper_set_disabled(stepper, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_set_disabled_fail = 0;
  stepper->is_disabled = 0;
  stepper->auto_repeat = 1;

  g_cupertino_stepper_mock_on_tick_fail = 1;
  rc = cupertino_stepper_on_tick(stepper, 16.0);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_stepper_mock_on_tick_fail = 0;

  g_cupertino_stepper_mock_destroy_fail = 1;
  rc = cupertino_stepper_destroy(stepper);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_stepper_mock_destroy_fail = 0;
  stepper = NULL;

  /* Test destroy with stepper->base == NULL */
  rc = cupertino_stepper_create(dummy_engine, 0.0, 100.0, 1.0, &stepper, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_spin_button_base_destroy(stepper->base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  stepper->base = NULL;
  rc = cupertino_stepper_destroy(stepper);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  stepper = NULL;
#endif

  PASS();
}

SUITE(cupertino_stepper_suite) {
  RUN_TEST(test_stepper_lifecycle_and_cva);
  RUN_TEST(test_stepper_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_stepper_suite);
  GREATEST_MAIN_END();
}
