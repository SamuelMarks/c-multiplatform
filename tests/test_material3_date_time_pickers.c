/**
 * @file test_material3_date_time_pickers.c
 * @brief Unit tests for Material 3 Date and Time Pickers.
 */

/* clang-format off */
#include "material3/md3_date_time_pickers.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_pickers_mock_fail;
extern int g_malloc_fail_countdown;
#endif

SUITE(md3_date_time_pickers_suite);

TEST test_md3_date_picker_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_date_picker *picker = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_picker_create(NULL, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_picker_create(engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_picker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_date_picker_create(engine, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);

  rc = md3_date_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_date_range_picker_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_date_range_picker *picker = NULL;
  ui_error_t rc;
  struct ui_date date;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_range_picker_create(NULL, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_create(engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_date_range_picker_create(engine, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);

  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_range_picker_create(engine, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&date, 0, sizeof(date));
  date.year = 2024;
  date.month = 1;
  date.day = 1;

  rc = md3_date_range_picker_select_date(NULL, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_select_date(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_date_range_picker_select_date(picker, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_range_picker_set_hover_date(NULL, &date);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_set_hover_date(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_set_hover_date(picker, &date);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_range_picker_clear(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_clear(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_range_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

TEST test_md3_time_picker_lifecycle(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_time_picker *picker = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_time_picker_create(NULL, MD3_TIME_PICKER_MODE_DIAL,
                              UI_TIMEPICKER_FORMAT_12H, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_time_picker_create(engine, MD3_TIME_PICKER_MODE_DIAL,
                              UI_TIMEPICKER_FORMAT_12H, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_time_picker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_time_picker_create(engine, MD3_TIME_PICKER_MODE_DIAL,
                              UI_TIMEPICKER_FORMAT_12H, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);
  rc = md3_time_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_time_picker_create(engine, MD3_TIME_PICKER_MODE_INPUT,
                              UI_TIMEPICKER_FORMAT_24H, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);
  rc = md3_time_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_md3_pickers_mock_fail;
TEST test_md3_date_time_pickers_oom(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_date_picker *dp = NULL;
  struct md3_date_range_picker *drp = NULL;
  struct md3_time_picker *tp = NULL;
  ui_error_t rc;
  struct ui_date date;
  struct ui_date_range range;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  int i;
  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_date_picker_create(engine, &dp);
    if (rc == UI_ERROR_NONE) {
      md3_date_picker_destroy(dp);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_date_range_picker_create(engine, &drp);
    if (rc == UI_ERROR_NONE) {
      md3_date_range_picker_destroy(drp);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_time_picker_create(engine, MD3_TIME_PICKER_MODE_DIAL,
                                UI_TIMEPICKER_FORMAT_12H, &tp);
    if (rc == UI_ERROR_NONE) {
      md3_time_picker_destroy(tp);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  g_malloc_fail_countdown = -1;
  rc = md3_date_range_picker_create(engine, &drp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&date, 0, sizeof(date));
  date.year = 2024;
  date.month = 1;
  date.day = 1;

  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_date_range_picker_select_date(drp, &date);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_date_range_picker_set_hover_date(drp, &date);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  rc = md3_date_range_picker_get_range(NULL, &range);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_get_range(drp, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_date_range_picker_get_range(drp, &range);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 500; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_date_range_picker_clear(drp);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      break;
    }
  }

  md3_date_range_picker_destroy(drp);

  g_malloc_fail_countdown = -1;

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md3_date_time_pickers_destroy_errors(void) {
  struct ui_engine *engine = NULL;
  struct ui_engine_config engine_cfg;
  struct md3_date_picker *dp = NULL;
  struct md3_date_range_picker *drp = NULL;
  struct md3_time_picker *tp = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_date_picker_create(engine, &dp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 1;
  rc = md3_date_picker_destroy(dp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = md3_date_picker_create(engine, &dp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 2;
  rc = md3_date_picker_destroy(dp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = md3_date_picker_create(engine, &dp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 3;
  rc = md3_date_picker_destroy(dp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = md3_date_picker_create(engine, &dp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 4;
  rc = md3_date_picker_destroy(dp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = md3_date_picker_create(engine, &dp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 5;
  rc = md3_date_picker_destroy(dp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_pickers_mock_fail = 0;

  rc = md3_date_range_picker_create(engine, &drp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 6;
  rc = md3_date_range_picker_destroy(drp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = md3_date_range_picker_create(engine, &drp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 5;
  rc = md3_date_range_picker_destroy(drp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_pickers_mock_fail = 0;

  rc = md3_time_picker_create(engine, MD3_TIME_PICKER_MODE_DIAL,
                              UI_TIMEPICKER_FORMAT_12H, &tp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 7;
  rc = md3_time_picker_destroy(tp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = md3_time_picker_create(engine, MD3_TIME_PICKER_MODE_DIAL,
                              UI_TIMEPICKER_FORMAT_12H, &tp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_md3_pickers_mock_fail = 5;
  rc = md3_time_picker_destroy(tp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_pickers_mock_fail = 0;

  rc = md3_date_range_picker_create(engine, &drp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  struct ui_date test_date;
  memset(&test_date, 0, sizeof(test_date));
  test_date.year = 2024;
  test_date.month = 1;
  test_date.day = 1;
  struct ui_date_range test_range;
  memset(&test_range, 0, sizeof(test_range));

  g_md3_pickers_mock_fail = 8;
  rc = md3_date_range_picker_select_date(drp, &test_date);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_pickers_mock_fail = 9;
  rc = md3_date_range_picker_select_date(drp, &test_date);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_pickers_mock_fail = 10;
  rc = md3_date_range_picker_set_hover_date(drp, &test_date);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_pickers_mock_fail = 11;
  rc = md3_date_range_picker_clear(drp);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_md3_pickers_mock_fail = 0;
  md3_date_range_picker_destroy(drp);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  PASS();
}
#endif

SUITE(md3_date_time_pickers_suite) {
  RUN_TEST(test_md3_date_picker_lifecycle);
  RUN_TEST(test_md3_date_range_picker_lifecycle);
  RUN_TEST(test_md3_time_picker_lifecycle);
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_md3_pickers_mock_fail;
  RUN_TEST(test_md3_date_time_pickers_oom);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md3_date_time_pickers_destroy_errors);
#endif
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_date_time_pickers_suite);
  GREATEST_MAIN_END();
}
