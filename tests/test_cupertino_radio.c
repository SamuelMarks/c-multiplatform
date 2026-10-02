/**
 * @file test_cupertino_radio.c
 * @brief Unit tests for Cupertino Radio component.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_radio.h"
#include "ui_engine.h"
#include "ui_radio_group_base.h"
#include <math.h>
/* clang-format on */

SUITE(cupertino_radio_suite);

TEST test_radio_lifecycle_and_cva(void) {
  struct cupertino_radio *r1 = NULL;
  struct cupertino_radio *r2 = NULL;
  struct cupertino_radio *r3 = NULL;
  struct cupertino_radio *r4 = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_toggle_base *toggle = NULL;
  struct ui_toggle_base *saved_toggle = NULL;
  struct ui_radio_group_base *group = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  int checked = 0;
  union ui_signal_payload payload;
  ui_error_t rc;

  memset(&payload, 0, sizeof(payload));

  /* Invalid arguments */
  rc = cupertino_radio_create(NULL, &r1, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_create(dummy_engine, NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation with CVA */
  rc = cupertino_radio_create(dummy_engine, &r1, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(r1 != NULL);
  ASSERT(cva != NULL);
  ASSERT_EQ(0, r1->is_checked);
  ASSERT(fabs(r1->scale - 1.0f) < 1e-5f);
  ASSERT(fabs(r1->outer_diameter - CUPERTINO_RADIO_OUTER_DIAMETER) < 1e-5f);
  ASSERT(fabs(r1->inner_dot_diameter - CUPERTINO_RADIO_INNER_DOT_DIAMETER) <
         1e-5f);

  /* Toggle retrieval */
  rc = cupertino_radio_get_toggle(NULL, &toggle);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_get_toggle(r1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_get_toggle(r1, &toggle);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(toggle != NULL);

  /* Checked state & spring scale animation pop */
  rc = cupertino_radio_set_checked(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_get_checked(NULL, &checked);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_get_checked(r1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_radio_get_checked(r1, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, checked);

  rc = cupertino_radio_set_checked(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, r1->is_checked);
  ASSERT(fabs(r1->scale - 1.15f) < 1e-5f); /* Pop scale */
  rc = cupertino_radio_get_checked(r1, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, checked);

  rc = cupertino_radio_set_checked(r1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, r1->is_checked);
  ASSERT(fabs(r1->scale - 1.0f) < 1e-5f);

  /* Disabled state */
  rc = cupertino_radio_set_disabled(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_set_disabled(r1, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_radio_set_disabled(r1, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mutual exclusion radio group binding */
  rc = ui_radio_group_base_create(&group, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_radio_bind_group(NULL, group);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_bind_group(r1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_bind_group(r1, group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Create second radio and bind */
  rc = cupertino_radio_create(dummy_engine, &r2, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_radio_bind_group(r2, group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA methods */
  ASSERT(cva->write_value != NULL);
  ASSERT(cva->set_disabled_state != NULL);

  rc = cva->write_value(NULL, payload);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  payload.bool_val = 1;
  rc = cva->write_value(cva->component, payload);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_radio_get_checked(r1, &checked);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, checked);

  rc = cva->set_disabled_state(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cva->set_disabled_state(cva->component, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test unbound radio destruction (radio->group == NULL) */
  rc = cupertino_radio_create(dummy_engine, &r3, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_radio_destroy(r3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test radio destruction with group != NULL and toggle == NULL */
  rc = cupertino_radio_create(dummy_engine, &r3, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  saved_toggle = r3->toggle;
  r3->toggle = NULL;
  r3->group = group;
  rc = cupertino_radio_destroy(r3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_toggle_base_destroy(saved_toggle);

  /* Test NULL toggle branch percolation */
  rc = cupertino_radio_create(dummy_engine, &r4, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  saved_toggle = r4->toggle;
  r4->toggle = NULL;

  rc = cupertino_radio_set_checked(r4, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_set_disabled(r4, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_bind_group(r4, group);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Destroy with r4->toggle == NULL */
  rc = cupertino_radio_destroy(r4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ui_toggle_base_destroy(saved_toggle);

  /* Clean up */
  rc = cupertino_radio_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_radio_destroy(r1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_radio_destroy(r2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_radio_group_base_destroy(group);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_radio_oom_mock(void) {
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1000;
  struct cupertino_radio *r = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;

  g_malloc_fail_countdown = 0;
  rc = cupertino_radio_create(dummy_engine, &r, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, r);

  g_malloc_fail_countdown = 1;
  rc = cupertino_radio_create(dummy_engine, &r, NULL);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, r);

  g_malloc_fail_countdown = -1;
#endif

  PASS();
}

SUITE(cupertino_radio_suite) {
  RUN_TEST(test_radio_lifecycle_and_cva);
  RUN_TEST(test_radio_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_radio_suite);
  GREATEST_MAIN_END();
}
