/**
 * @file test_cupertino_checkbox.c
 * @brief Unit tests for Cupertino Checkbox component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_checkbox.h"
#include "ui_engine.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_checkbox_suite);

TEST test_checkbox_lifecycle_and_cva(void) {
  struct cupertino_checkbox *cb = NULL;
  struct cupertino_checkbox *cb2 = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_checkbox_base *base = NULL;
  struct ui_checkbox_base *saved_base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  enum ui_checkbox_state st;
  union ui_signal_payload payload;
  ui_error_t rc;

  memset(&payload, 0, sizeof(payload));

  /* Invalid arguments */
  rc = cupertino_checkbox_create(NULL, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with CVA */
  rc = cupertino_checkbox_create(dummy_engine, &cb, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cb != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(UI_CHECKBOX_STATE_UNCHECKED, cb->state);

  /* Get base */
  rc = cupertino_checkbox_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_get_base(cb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_get_base(cb, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Set / get state */
  rc = cupertino_checkbox_set_state(NULL, UI_CHECKBOX_STATE_CHECKED);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_get_state(NULL, &st);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_get_state(cb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_checkbox_get_state(cb, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CHECKBOX_STATE_UNCHECKED, st);

  rc = cupertino_checkbox_set_state(cb, UI_CHECKBOX_STATE_CHECKED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_get_state(cb, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CHECKBOX_STATE_CHECKED, st);

  rc = cupertino_checkbox_set_state(cb, UI_CHECKBOX_STATE_INDETERMINATE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_get_state(cb, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CHECKBOX_STATE_INDETERMINATE, st);

  /* Toggle */
  rc = cupertino_checkbox_toggle(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  /* From indeterminate, toggle -> checked */
  rc = cupertino_checkbox_toggle(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_get_state(cb, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CHECKBOX_STATE_CHECKED, st);

  /* From checked, toggle -> unchecked */
  rc = cupertino_checkbox_toggle(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_get_state(cb, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CHECKBOX_STATE_UNCHECKED, st);

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_cupertino_checkbox_mock_get_state_fail;
    g_cupertino_checkbox_mock_get_state_fail = 1;
    rc = cupertino_checkbox_toggle(cb);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    g_cupertino_checkbox_mock_get_state_fail = 0;
  }
#endif

  /* Label */
  rc = cupertino_checkbox_set_label(NULL, "Remember Me");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_set_label(cb, "Remember Me");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Remember Me", cb->label);

  rc = cupertino_checkbox_set_label(cb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* CVA methods */
  ASSERT(cva->write_value != NULL);
  ASSERT(cva->set_disabled_state != NULL);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  payload.bool_val = 1;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_get_state(cb, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CHECKBOX_STATE_CHECKED, st);

  payload.bool_val = 0;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_get_state(cb, &st);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(UI_CHECKBOX_STATE_UNCHECKED, st);

  rc = cva->set_disabled_state(NULL, UI_TRUE);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, (ui_bool_t)42);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva->set_disabled_state(cva->component, UI_TRUE);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test cb2 with base == NULL */
  rc = cupertino_checkbox_create(dummy_engine, &cb2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  saved_base = cb2->base;
  cb2->base = NULL;
  rc = cupertino_checkbox_set_state(cb2, UI_CHECKBOX_STATE_CHECKED);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_toggle(cb2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_set_label(cb2, "Test");
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_destroy(cb2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_checkbox_base_destroy(saved_base);

  /* Destroy */
  rc = cupertino_checkbox_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_destroy(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_checkbox_animated_stroke(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_checkbox *cb = NULL;
  float prog = 0.0f;
  float x1 = 0.0f;
  float y1 = 0.0f;
  float x2 = 0.0f;
  float y2 = 0.0f;
  float x3 = 0.0f;
  float y3 = 0.0f;
  ui_error_t rc;

  rc = cupertino_checkbox_create(dummy_engine, &cb, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_checkbox_get_stroke_progress(cb, &prog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(0.0f, prog, 0.01f);

  /* Set state checked -> stroke progress 1.0 */
  rc = cupertino_checkbox_set_state(cb, UI_CHECKBOX_STATE_CHECKED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_get_stroke_progress(cb, &prog);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(1.0f, prog, 0.01f);

  /* Progress in segment 1 (e.g. 0.2f) */
  rc = cupertino_checkbox_set_stroke_progress(cb, 0.2f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, &y1, &x2, &y2, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(4.5f, x1, 0.01f);
  ASSERT_IN_RANGE(9.0f, y1, 0.01f);
  ASSERT_GT(x2, 4.5f);
  ASSERT_LT(x2, 7.5f);

  /* Progress in segment 2 (e.g. 0.8f) */
  rc = cupertino_checkbox_set_stroke_progress(cb, 0.8f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, &y1, &x2, &y2, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(7.5f, x2, 0.01f);
  ASSERT_IN_RANGE(13.0f, y2, 0.01f);
  ASSERT_GT(x3, 7.5f);
  ASSERT_LT(y3, 13.0f);

  /* Fully drawn 1.0 */
  rc = cupertino_checkbox_set_stroke_progress(cb, 1.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, &y1, &x2, &y2, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_IN_RANGE(13.5f, x3, 0.01f);
  ASSERT_IN_RANGE(5.0f, y3, 0.01f);

  /* Invalid inputs */
  rc = cupertino_checkbox_set_stroke_progress(NULL, 0.5f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_set_stroke_progress(cb, -0.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_set_stroke_progress(cb, 1.1f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_get_stroke_progress(NULL, &prog);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_get_stroke_progress(cb, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_compute_checkmark_path(NULL, &x1, &y1, &x2, &y2, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, NULL, &y1, &x2, &y2, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, NULL, &x2, &y2, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, &y1, NULL, &y2, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, &y1, &x2, NULL, &x3,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, &y1, &x2, &y2, NULL,
                                                 &y3);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_checkbox_compute_checkmark_path(cb, &x1, &y1, &x2, &y2, &x3,
                                                 NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_checkbox_destroy(cb);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_checkbox_oom_mock(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_checkbox *cb = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;

  g_malloc_fail_countdown = 0;
  rc = cupertino_checkbox_create(dummy_engine, &cb, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, cb);

  g_malloc_fail_countdown = 1;
  rc = cupertino_checkbox_create(dummy_engine, &cb, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, cb);

  g_malloc_fail_countdown = -1;
#endif

  PASS();
}

SUITE(cupertino_checkbox_suite) {
  RUN_TEST(test_checkbox_lifecycle_and_cva);
  RUN_TEST(test_checkbox_animated_stroke);
  RUN_TEST(test_checkbox_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_checkbox_suite);
  GREATEST_MAIN_END();
}
