/**
 * @file test_cupertino_segmented_control.c
 * @brief Unit tests for Cupertino Segmented Control component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_segmented_control.h"
#include "ui_engine.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_segmented_control_suite);

TEST test_segmented_control_lifecycle_and_cva(void) {
  struct cupertino_segmented_control *control = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_segmented_control_base *base = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  int idx = -1;
  int count = 0;
  union ui_signal_payload payload;
  ui_error_t rc;

  memset(&payload, 0, sizeof(payload));

  /* Invalid arguments */
  rc = cupertino_segmented_control_create(NULL, &control, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with CVA */
  rc = cupertino_segmented_control_create(dummy_engine, &control, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(control != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(-1, control->selected_index);
  ASSERT_EQ(0, control->segment_count);

  /* Get base */
  rc = cupertino_segmented_control_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_get_base(control, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_get_base(control, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Adding segments */
  rc = cupertino_segmented_control_add_segment(NULL, "Map", &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_add_segment(control, NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_add_segment(control, "Map", NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_segmented_control_add_segment(control, "Map", &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);
  ASSERT_EQ(0, control->selected_index); /* First segment auto-selected */

  rc = cupertino_segmented_control_add_segment(control, "Transit", &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  rc = cupertino_segmented_control_add_segment(control, "Satellite", &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  rc = cupertino_segmented_control_get_segment_count(NULL, &count);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_get_segment_count(control, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_get_segment_count(control, &count);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, count);

  /* Segment selection */
  rc = cupertino_segmented_control_select_index(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_select_index(control, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_select_index(control, 5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_segmented_control_select_index(control, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, control->selected_index);
  ASSERT(control->thumb_offset_x > 0.0f);

  rc = cupertino_segmented_control_get_selected_index(NULL, &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_get_selected_index(control, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_get_selected_index(control, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  /* CVA methods */
  ASSERT(cva->write_value != NULL);
  ASSERT(cva->set_disabled_state != NULL);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  payload.int_val = 2;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_segmented_control_get_selected_index(control, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, (ui_bool_t)2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, UI_FALSE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cva->set_disabled_state(cva->component, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add segments until MAX_SEGMENTS limit */
  while (control->segment_count < CUPERTINO_SEGMENTED_CONTROL_MAX_SEGMENTS) {
    rc = cupertino_segmented_control_add_segment(control, "Extra", &idx);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_segmented_control_add_segment(control, "Overflow", &idx);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy with base = NULL */
  {
    struct cupertino_segmented_control *null_base_control = NULL;
    rc = cupertino_segmented_control_create(dummy_engine, &null_base_control,
                                            NULL);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    ASSERT(null_base_control != NULL);
    ui_segmented_control_base_destroy(null_base_control->base);
    null_base_control->base = NULL;
    rc = cupertino_segmented_control_destroy(null_base_control);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Destroy */
  rc = cupertino_segmented_control_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_segmented_control_destroy(control);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_segmented_control_oom_mock(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_segmented_control *control = NULL;
  int idx = -1;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;
  extern int g_cupertino_segmented_control_mock_base_destroy_fail;
  extern int g_cupertino_segmented_control_mock_button_destroy_fail;

  g_malloc_fail_countdown = 0;
  rc = cupertino_segmented_control_create(dummy_engine, &control, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, control);

  g_malloc_fail_countdown = 1;
  rc = cupertino_segmented_control_create(dummy_engine, &control, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, control);

  g_malloc_fail_countdown = -1;

  rc = cupertino_segmented_control_create(dummy_engine, &control, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(control != NULL);

  /* OOM during add_segment: button create failure */
  g_malloc_fail_countdown = 0;
  rc = cupertino_segmented_control_add_segment(control, "Tab1", &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* OOM during add_segment: append_segment failure with button destroy success
   */
  g_malloc_fail_countdown = 2;
  rc = cupertino_segmented_control_add_segment(control, "Tab2", &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);

  /* OOM during add_segment: append_segment failure with button destroy failure
   */
  g_malloc_fail_countdown = 2;
  g_cupertino_segmented_control_mock_button_destroy_fail = 1;
  rc = cupertino_segmented_control_add_segment(control, "Tab3", &idx);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_cupertino_segmented_control_mock_button_destroy_fail = 0;

  g_malloc_fail_countdown = -1;

  /* Test base destroy failure */
  g_cupertino_segmented_control_mock_base_destroy_fail = 1;
  rc = cupertino_segmented_control_destroy(control);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_segmented_control_mock_base_destroy_fail = 0;

  rc = cupertino_segmented_control_destroy(control);
  ASSERT_EQ(UI_ERROR_NONE, rc);
#endif

  PASS();
}

SUITE(cupertino_segmented_control_suite) {
  RUN_TEST(test_segmented_control_lifecycle_and_cva);
  RUN_TEST(test_segmented_control_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_segmented_control_suite);
  GREATEST_MAIN_END();
}
