/**
 * @file test_cupertino_indicator.c
 * @brief Unit tests for Cupertino Indicator component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_indicator.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_indicator_suite);

TEST test_indicator_invalid_args(void) {
  struct cupertino_indicator_descriptor desc;
  struct cupertino_indicator *indicator = NULL;
  struct ui_progress_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float val1 = 0.0f;
  float val2 = 0.0f;
  int animating = 0;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

  /* Create invalid */
  rc = cupertino_indicator_create(NULL, &desc, &indicator);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_create(dummy_engine, NULL, &indicator);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy invalid */
  rc = cupertino_indicator_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Start / stop / is_animating */
  rc = cupertino_indicator_start(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_stop(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_is_animating(NULL, &animating);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Tick */
  rc = cupertino_indicator_tick(NULL, 16.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Spoke opacity */
  rc = cupertino_indicator_get_spoke_opacity(NULL, 0, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Value */
  rc = cupertino_indicator_set_value(NULL, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_value(NULL, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Dimensions / base */
  rc = cupertino_indicator_get_dimensions(NULL, &val1, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid indicator to test null out pointers, out-of-range spokes, and
   * negative tick */
  rc = cupertino_indicator_create(dummy_engine, &desc, &indicator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_indicator_is_animating(indicator, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_tick(indicator, -1.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_spoke_opacity(indicator, -1, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_spoke_opacity(indicator, 99, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_spoke_opacity(indicator, 0, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_value(indicator, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_dimensions(indicator, NULL, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_dimensions(indicator, &val1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_indicator_get_base(indicator, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy with indicator->base == NULL */
  struct ui_progress_base *saved_base = indicator->base;
  indicator->base = NULL;
  rc = cupertino_indicator_destroy(indicator);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_progress_base_destroy(saved_base);

  PASS();
}

TEST test_indicator_activity_spinner(void) {
  struct cupertino_indicator_descriptor desc;
  struct cupertino_indicator *ind = NULL;
  struct ui_progress_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float op0 = 0.0f;
  float op1 = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  int animating = 0;
  ui_error_t rc;

  /* Small spinner (8 spokes, 20x20) */
  memset(&desc, 0, sizeof(desc));
  desc.type = CUPERTINO_INDICATOR_TYPE_ACTIVITY;
  desc.size = CUPERTINO_INDICATOR_SIZE_SMALL;
  desc.is_animating = 1;

  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ind != NULL);

  /* Base */
  rc = cupertino_indicator_get_base(ind, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Dimensions */
  rc = cupertino_indicator_get_dimensions(ind, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 19.9f && w < 20.1f);
  ASSERT(h > 19.9f && h < 20.1f);

  /* Check initial leading spoke opacity = 1.0 */
  rc = cupertino_indicator_get_spoke_opacity(ind, 0, &op0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(op0 > 0.999f);

  /* Check decaying opacity on other spoke */
  rc = cupertino_indicator_get_spoke_opacity(ind, 1, &op1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(op1 < op0);

  /* Tick 90ms (> 83.33ms step) -> rotates step */
  rc = cupertino_indicator_tick(ind, 90.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Spoke 1 is now the leading spoke (opacity ~ 1.0) */
  rc = cupertino_indicator_get_spoke_opacity(ind, 1, &op1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(op1 > 0.999f);

  /* Stop animation */
  rc = cupertino_indicator_stop(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_indicator_is_animating(ind, &animating);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, animating);

  /* Ticking while stopped does nothing */
  rc = cupertino_indicator_tick(ind, 100.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_indicator_get_spoke_opacity(ind, 1, &op0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(op0 > 0.999f);

  /* Start again */
  rc = cupertino_indicator_start(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_indicator_is_animating(ind, &animating);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, animating);

  /* Clean up */
  rc = cupertino_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Large spinner (12 spokes, 37x37) */
  desc.size = CUPERTINO_INDICATOR_SIZE_LARGE;
  desc.is_indeterminate = 1;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ind != NULL);
  rc = cupertino_indicator_get_dimensions(ind, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(w > 36.9f && w < 37.1f);
  ASSERT(h > 36.9f && h < 37.1f);

  rc = cupertino_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_indicator_progress_bar(void) {
  struct cupertino_indicator_descriptor desc;
  struct cupertino_indicator *ind = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  float val = 0.0f;
  float w = 0.0f;
  float h = 0.0f;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.type = CUPERTINO_INDICATOR_TYPE_BAR;
  desc.initial_value = 0.25f;

  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(ind != NULL);

  /* Height = 4.0pt */
  rc = cupertino_indicator_get_dimensions(ind, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(h > 3.99f && h < 4.01f);

  /* Initial value */
  rc = cupertino_indicator_get_value(ind, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(val > 0.249f && val < 0.251f);

  /* Set value */
  rc = cupertino_indicator_set_value(ind, 0.75f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_indicator_get_value(ind, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(val > 0.749f && val < 0.751f);

  /* Clamp bounds */
  rc = cupertino_indicator_set_value(ind, -0.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_indicator_get_value(ind, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(val < 0.001f);

  rc = cupertino_indicator_set_value(ind, 2.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_indicator_get_value(ind, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(val > 0.999f);

  rc = cupertino_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_indicator_oom_mock(void) {
  struct cupertino_indicator_descriptor desc;
  struct cupertino_indicator *ind = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_indicator_mock_create_fail;
  extern int g_cupertino_indicator_mock_destroy_fail;
  extern int g_cupertino_indicator_mock_set_indet_fail;
  extern int g_cupertino_indicator_mock_set_deter_fail;

  /* Fail malloc for struct cupertino_indicator */
  g_malloc_fail_countdown = 0;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(ind == NULL);

  /* Fail malloc for ui_progress_base */
  g_malloc_fail_countdown = 1;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(ind == NULL);
  g_malloc_fail_countdown = -1;

  /* Fail mock progress base create */
  g_cupertino_indicator_mock_create_fail = 1;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(ind == NULL);
  g_cupertino_indicator_mock_create_fail = 0;

  /* Fail mock set_indeterminate */
  desc.is_indeterminate = 1;
  g_cupertino_indicator_mock_set_indet_fail = 1;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(ind == NULL);
  g_cupertino_indicator_mock_set_indet_fail = 0;

  /* Fail mock set_indeterminate with mock destroy failure */
  g_cupertino_indicator_mock_set_indet_fail = 1;
  g_cupertino_indicator_mock_destroy_fail = 1;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(ind == NULL);
  g_cupertino_indicator_mock_set_indet_fail = 0;
  g_cupertino_indicator_mock_destroy_fail = 0;

  /* Fail mock set_determinate */
  desc.is_indeterminate = 0;
  g_cupertino_indicator_mock_set_deter_fail = 1;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(ind == NULL);

  /* Fail mock set_determinate with mock destroy failure */
  g_cupertino_indicator_mock_destroy_fail = 1;
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(ind == NULL);
  g_cupertino_indicator_mock_set_deter_fail = 0;
  g_cupertino_indicator_mock_destroy_fail = 0;

  /* Fail mock destroy in cupertino_indicator_destroy */
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_indicator_mock_destroy_fail = 1;
  rc = cupertino_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_indicator_mock_destroy_fail = 0;

  /* set_value with indicator->base == NULL and mock set_determinate failure */
  rc = cupertino_indicator_create(dummy_engine, &desc, &ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_cupertino_indicator_mock_set_deter_fail = 1;
  rc = cupertino_indicator_set_value(ind, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_indicator_mock_set_deter_fail = 0;

  struct ui_progress_base *saved_b = ind->base;
  ind->base = NULL;
  rc = cupertino_indicator_set_value(ind, 0.5f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ind->base = saved_b;

  rc = cupertino_indicator_destroy(ind);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#else
  rc = cupertino_indicator_create(NULL, &desc, &ind);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_indicator_suite) {
  RUN_TEST(test_indicator_invalid_args);
  RUN_TEST(test_indicator_activity_spinner);
  RUN_TEST(test_indicator_progress_bar);
  RUN_TEST(test_indicator_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_indicator_suite);
  GREATEST_MAIN_END();
}
