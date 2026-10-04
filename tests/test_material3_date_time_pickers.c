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
TEST test_md3_date_time_pickers_oom(void) {
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

  int i;
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_date_picker_create(engine, &dp);
    if (rc == UI_ERROR_NONE) {
      md3_date_picker_destroy(dp);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  for (i = 0; i < 2; i++) {
    g_malloc_fail_countdown = i;
    rc = md3_date_range_picker_create(engine, &drp);
    if (rc == UI_ERROR_NONE) {
      md3_date_range_picker_destroy(drp);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }

  for (i = 0; i < 2; i++) {
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
  RUN_TEST(test_md3_date_time_pickers_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_date_time_pickers_suite);
  GREATEST_MAIN_END();
}
