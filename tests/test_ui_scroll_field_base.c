/**
 * @file test_ui_scroll_field_base.c
 * @brief Unit tests and OOM mocks for ui_scroll_field_base CDK primitive.
 */

/* clang-format off */
#include "ui_scroll_field_base.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
extern int g_scroll_field_mock_fail;
extern int g_scroll_field_destroy_mock_fail;
#endif

SUITE(ui_scroll_field_base_suite);

TEST test_ui_scroll_field_create_destroy(void) {
  struct ui_scroll_field_base *field = NULL;
  struct ui_component *comp = NULL;
  const char *role_val = NULL;
  ui_error_t rc;

  /* Null checks */
  rc = ui_scroll_field_base_create(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_destroy(NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_component(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Normal creation */
  rc = ui_scroll_field_base_create(&field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(field != NULL);

  rc = ui_scroll_field_base_get_component(field, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_scroll_field_base_get_component(field, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);
  ASSERT(comp->shadow_root != NULL);

  rc = ui_dom_node_get_attribute(comp->shadow_root, "role", &role_val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("spinbutton", role_val);

  /* Destroy with comp set to NULL branch */
  {
    struct ui_scroll_field_base *field_no_comp = NULL;
    struct ui_component *saved_comp = NULL;
    rc = ui_scroll_field_base_create(&field_no_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_scroll_field_base_get_component(field_no_comp, &saved_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_component_destroy(saved_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    *(struct ui_component **)field_no_comp = NULL;
    rc = ui_scroll_field_base_destroy(field_no_comp);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* Destroy mock failure */
  {
    struct ui_scroll_field_base *field_mock = NULL;
    rc = ui_scroll_field_base_create(&field_mock);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_scroll_field_destroy_mock_fail = 2;
    rc = ui_scroll_field_base_destroy(field_mock);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_scroll_field_destroy_mock_fail = 0;
  }
#endif

  rc = ui_scroll_field_base_destroy(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_ui_scroll_field_range_and_value(void) {
  struct ui_scroll_field_base *field = NULL;
  int min_v = 0;
  int max_v = 0;
  int step_v = 0;
  int val = 0;
  ui_error_t rc;

  rc = ui_scroll_field_base_create(&field);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null and invalid checks */
  rc = ui_scroll_field_base_set_range(NULL, 0, 100, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_set_range(field, 100, 50, 1); /* min >= max */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_set_range(field, 0, 50, 0); /* step <= 0 */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Get range null checks for each argument */
  rc = ui_scroll_field_base_get_range(NULL, &min_v, &max_v, &step_v);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_get_range(field, NULL, &max_v, &step_v);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_get_range(field, &min_v, NULL, &step_v);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_get_range(field, &min_v, &max_v, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_scroll_field_base_set_value(NULL, 10);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_get_value(NULL, &val);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_get_value(field, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = ui_scroll_field_base_step_up(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_step_down(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Configure range 10 to 50, step 5 */
  rc = ui_scroll_field_base_set_range(field, 10, 50, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_scroll_field_base_get_range(field, &min_v, &max_v, &step_v);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, min_v);
  ASSERT_EQ(50, max_v);
  ASSERT_EQ(5, step_v);

  /* Value was 0, now clamped to min (10) */
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, val);

  /* Set value to 50 */
  rc = ui_scroll_field_base_set_value(field, 50);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Reconfigure range to 0..30 -> clamps current_value to max_val (30) */
  rc = ui_scroll_field_base_set_range(field, 0, 30, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(30, val);

  /* Reconfigure to 10..50 */
  rc = ui_scroll_field_base_set_range(field, 10, 50, 5);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Step up */
  rc = ui_scroll_field_base_step_up(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(35, val);

  /* Step down */
  rc = ui_scroll_field_base_step_down(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(30, val);

  /* Set value below minimum without looping -> clamped to min */
  rc = ui_scroll_field_base_set_value(field, -100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, val);

  /* Clamp at minimum without looping when stepping down */
  rc = ui_scroll_field_base_step_down(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(10, val);

  /* Set value beyond maximum clamps at max */
  rc = ui_scroll_field_base_set_value(field, 999);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(50, val);

#ifdef UI_TEST_MOCK_ALLOC
  /* Test set_value failure via OOM during attribute updates */
  g_malloc_fail_countdown = 0;
  rc = ui_scroll_field_base_set_value(field, 20);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  rc = ui_scroll_field_base_set_value(field, 20);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 2;
  rc = ui_scroll_field_base_set_value(field, 20);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 3;
  rc = ui_scroll_field_base_set_value(field, 20);
  ASSERT(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;
#endif

  rc = ui_scroll_field_base_destroy(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_ui_scroll_field_looping(void) {
  struct ui_scroll_field_base *field = NULL;
  int looping = 0;
  int val = 0;
  ui_error_t rc;

  rc = ui_scroll_field_base_create(&field);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = ui_scroll_field_base_set_looping(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_is_looping(NULL, &looping);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = ui_scroll_field_base_is_looping(field, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Configure range 0 to 59 (minutes), step 1 */
  rc = ui_scroll_field_base_set_range(field, 0, 59, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Enable looping */
  rc = ui_scroll_field_base_set_looping(field, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_is_looping(field, &looping);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, looping);

  /* Set value with looping < min_val */
  rc = ui_scroll_field_base_set_value(field, -65);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(val >= 0 && val <= 59);

  /* Set value with looping > max_val */
  rc = ui_scroll_field_base_set_value(field, 150);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(val >= 0 && val <= 59);

  /* Start at 0, step down wraps to 59 */
  rc = ui_scroll_field_base_set_value(field, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_step_down(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(59, val);

  /* Step up from 59 wraps to 0 */
  rc = ui_scroll_field_base_step_up(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_get_value(field, &val);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, val);

  /* Disable looping */
  rc = ui_scroll_field_base_set_looping(field, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_scroll_field_base_is_looping(field, &looping);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, looping);

  rc = ui_scroll_field_base_destroy(field);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_ui_scroll_field_oom(void) {
  int i;
  ui_error_t rc;

  /* Test create under OOM countdown loops */
  for (i = 0; i < 40; ++i) {
    struct ui_scroll_field_base *field = NULL;
    g_malloc_fail_countdown = i;
    rc = ui_scroll_field_base_create(&field);
    if (rc == UI_ERROR_NONE) {
      ui_error_t rc_cleanup = ui_scroll_field_base_destroy(field);
      ASSERT_EQ(UI_ERROR_NONE, rc_cleanup);
      break;
    } else {
      ASSERT(rc != UI_ERROR_NONE);
      ASSERT(field == NULL);
    }
  }
  g_malloc_fail_countdown = -1;

  /* Mock append_child failure for text_node */
  {
    struct ui_scroll_field_base *field = NULL;
    g_scroll_field_mock_fail = 1;
    rc = ui_scroll_field_base_create(&field);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(field == NULL);
    g_scroll_field_mock_fail = 0;
  }

  /* Mock append_child failure for value_node */
  {
    struct ui_scroll_field_base *field = NULL;
    g_scroll_field_mock_fail = 2;
    rc = ui_scroll_field_base_create(&field);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(field == NULL);
    g_scroll_field_mock_fail = 0;
  }

  /* Mock destroy failures during cleanup */
  {
    struct ui_scroll_field_base *field = NULL;
    g_scroll_field_mock_fail = 1;
    g_scroll_field_destroy_mock_fail = 1;
    rc = ui_scroll_field_base_create(&field);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(field == NULL);
    g_scroll_field_mock_fail = 0;
    g_scroll_field_destroy_mock_fail = 0;
  }

  {
    struct ui_scroll_field_base *field = NULL;
    g_scroll_field_mock_fail = 1;
    g_scroll_field_destroy_mock_fail = 2;
    rc = ui_scroll_field_base_create(&field);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(field == NULL);
    g_scroll_field_mock_fail = 0;
    g_scroll_field_destroy_mock_fail = 0;
  }

  PASS();
}
#endif

SUITE(ui_scroll_field_base_suite) {
  RUN_TEST(test_ui_scroll_field_create_destroy);
  RUN_TEST(test_ui_scroll_field_range_and_value);
  RUN_TEST(test_ui_scroll_field_looping);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_ui_scroll_field_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_scroll_field_base_suite);
  GREATEST_MAIN_END();
}
