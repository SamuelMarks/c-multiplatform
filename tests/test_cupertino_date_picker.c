/**
 * @file test_cupertino_date_picker.c
 * @brief Unit tests for Cupertino Date and Time Pickers.
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_date_picker.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_date_picker_suite);

TEST test_date_picker_invalid_arguments(void) {
  struct cupertino_date_picker_descriptor desc;
  struct cupertino_date_picker *picker = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_date_picker_mode mode;
  enum cupertino_date_picker_style style;
  const char *text = NULL;
  int yr, mo, da, hr, mi;
  float w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.year = 2026;
  desc.month = 10;
  desc.day = 1;
  desc.hour = 9;
  desc.minute = 41;
  desc.minute_interval = 5;

  /* Creation invalid */
  rc = cupertino_date_picker_create(NULL, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_create(dummy_engine, NULL, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Mode / Style invalid */
  desc.mode = (enum cupertino_date_picker_mode) - 1;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.mode = (enum cupertino_date_picker_mode)5;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.mode = CUPERTINO_DATE_PICKER_MODE_DATE;

  desc.style = (enum cupertino_date_picker_style) - 1;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.style = (enum cupertino_date_picker_style)4;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.style = CUPERTINO_DATE_PICKER_STYLE_COMPACT;

  /* Year / month / day / hour / minute out of range */
  desc.year = 1800;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.year = 2200;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.year = 2026;

  desc.month = 0;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.month = 13;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.month = 10;

  desc.day = 0;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.day = 32;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.day = 1;

  desc.hour = -1;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.hour = 25;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.hour = 9;

  desc.minute = -1;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.minute = 60;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.minute = 41;

  /* Destruction invalid */
  rc = cupertino_date_picker_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Set date invalid arguments */
  rc = cupertino_date_picker_set_date(NULL, 2026, 10, 1, 9, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Create valid picker to test set_date bounds */
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_set_date(picker, 1800, 10, 1, 9, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2200, 10, 1, 9, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 0, 1, 9, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 13, 1, 9, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 10, 0, 9, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 10, 32, 9, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 10, 1, -1, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 10, 1, 24, 41);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 10, 1, 9, -1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_date(picker, 2026, 10, 1, 9, 60);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid set_date */
  rc = cupertino_date_picker_set_date(picker, 2028, 5, 20, 15, 30);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Get date invalid pointer combinations */
  rc = cupertino_date_picker_get_date(NULL, &yr, &mo, &da, &hr, &mi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_date(picker, NULL, &mo, &da, &hr, &mi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_date(picker, &yr, NULL, &da, &hr, &mi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_date(picker, &yr, &mo, NULL, &hr, &mi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_date(picker, &yr, &mo, &da, NULL, &mi);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_date(picker, &yr, &mo, &da, &hr, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Mode / style / text / popover / dimensions / cva invalid */
  rc = cupertino_date_picker_set_mode(NULL, CUPERTINO_DATE_PICKER_MODE_TIME);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_mode(picker,
                                      (enum cupertino_date_picker_mode) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_mode(picker,
                                      (enum cupertino_date_picker_mode)5);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_mode(NULL, &mode);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_mode(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_date_picker_set_style(NULL,
                                       CUPERTINO_DATE_PICKER_STYLE_COMPACT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_style(picker,
                                       (enum cupertino_date_picker_style) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_set_style(picker,
                                       (enum cupertino_date_picker_style)4);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_style(NULL, &style);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_style(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_date_picker_get_formatted_text(NULL, &text);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_date_picker_set_popover_open(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_date_picker_get_dimensions(NULL, &w, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_dimensions(picker, NULL, &h);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_dimensions(picker, &w, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_date_picker_get_cva(NULL, &cva);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_date_picker_get_cva(picker, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_date_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_date_picker_modes_and_cva(void) {
  struct cupertino_date_picker_descriptor desc;
  struct cupertino_date_picker *picker = NULL;
  struct ui_control_value_accessor *cva = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  const char *text = NULL;
  enum cupertino_date_picker_mode mode;
  enum cupertino_date_picker_style style;
  int new_date[5] = {2027, 12, 25, 14, 30};
  int yr, mo, da, hr, mi;
  float w, h;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.mode = CUPERTINO_DATE_PICKER_MODE_DATE;
  desc.style = CUPERTINO_DATE_PICKER_STYLE_COMPACT;
  desc.year = 2026;
  desc.month = 10;
  desc.day = 1;
  desc.hour = 9;
  desc.minute = 41;

  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(picker != NULL);

  rc = cupertino_date_picker_get_date(picker, &yr, &mo, &da, &hr, &mi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2026, yr);
  ASSERT_EQ(10, mo);
  ASSERT_EQ(1, da);

  /* Format check: "Oct 1, 2026" */
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("Oct 1, 2026", text);

  /* Check get_mode and get_style */
  rc = cupertino_date_picker_get_mode(picker, &mode);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_MODE_DATE, mode);
  rc = cupertino_date_picker_get_style(picker, &style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_STYLE_COMPACT, style);

  /* Switch style to Wheels */
  rc = cupertino_date_picker_set_style(picker,
                                       CUPERTINO_DATE_PICKER_STYLE_WHEELS);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_dimensions(picker, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_WHEELS_WIDTH, w);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_WHEELS_HEIGHT, h);

  /* Switch mode to Time 12h */
  rc = cupertino_date_picker_set_mode(picker, CUPERTINO_DATE_PICKER_MODE_TIME);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("9:41 AM", text);

  /* Test CVA writing */
  rc = cupertino_date_picker_get_cva(picker, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(cva != NULL);

  {
    union ui_signal_payload payload;
    memset(&payload, 0, sizeof(payload));
    payload.ptr_val = new_date;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  rc = cupertino_date_picker_get_date(picker, &yr, &mo, &da, &hr, &mi);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2027, yr);
  ASSERT_EQ(12, mo);
  ASSERT_EQ(25, da);
  ASSERT_EQ(14, hr);
  ASSERT_EQ(30, mi);

  /* Test CVA set disabled */
  rc = cva->set_disabled_state(cva->component, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Popover toggle */
  rc = cupertino_date_picker_set_popover_open(picker, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, picker->is_popover_open);

  /* Test 24h mode format and midnight/noon in 12h mode */
  picker->is_24h = 1;
  picker->hour = 0;
  picker->minute = 5;
  rc = cupertino_date_picker_set_mode(picker, CUPERTINO_DATE_PICKER_MODE_TIME);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("00:05", text);

  picker->is_24h = 0;
  picker->hour = 0; /* 12:05 AM */
  rc = cupertino_date_picker_set_mode(picker, CUPERTINO_DATE_PICKER_MODE_TIME);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12:05 AM", text);

  picker->hour = 12; /* 12:05 PM */
  rc = cupertino_date_picker_set_mode(picker, CUPERTINO_DATE_PICKER_MODE_TIME);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("12:05 PM", text);

  /* Mode DATE_AND_TIME */
  picker->hour = 0;
  rc = cupertino_date_picker_set_mode(picker,
                                      CUPERTINO_DATE_PICKER_MODE_DATE_AND_TIME);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  picker->hour = 14;
  rc = cupertino_date_picker_set_mode(picker,
                                      CUPERTINO_DATE_PICKER_MODE_DATE_AND_TIME);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mode COUNT_DOWN_TIMER */
  rc = cupertino_date_picker_set_mode(
      picker, CUPERTINO_DATE_PICKER_MODE_COUNTDOWN_TIMER);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_formatted_text(picker, &text);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Month clamping in update_formatted_text (month < 1 and month > 12) */
  picker->month = 0;
  rc = cupertino_date_picker_set_mode(picker, CUPERTINO_DATE_PICKER_MODE_DATE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->month = 15;
  rc = cupertino_date_picker_set_mode(picker, CUPERTINO_DATE_PICKER_MODE_DATE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  picker->month = 10;

  /* Style INLINE */
  rc = cupertino_date_picker_set_style(picker,
                                       CUPERTINO_DATE_PICKER_STYLE_INLINE);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_dimensions(picker, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_INLINE_WIDTH, w);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_INLINE_HEIGHT, h);

  /* Style COMPACT */
  rc = cupertino_date_picker_set_style(picker,
                                       CUPERTINO_DATE_PICKER_STYLE_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_date_picker_get_dimensions(picker, &w, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_COMPACT_WIDTH, w);
  ASSERT_EQ(CUPERTINO_DATE_PICKER_COMPACT_HEIGHT, h);

  /* CVA with NULL ptr_val and NULL component */
  {
    union ui_signal_payload payload;
    memset(&payload, 0, sizeof(payload));
    payload.ptr_val = NULL;
    rc = cva->write_value(cva->component, payload);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = cva->write_value(NULL, payload);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cva->set_disabled_state(NULL, 1);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
    rc = cva->set_disabled_state(cva->component, -1);
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  }

  rc = cupertino_date_picker_destroy(picker);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_date_picker_oom_mock(void) {
  struct cupertino_date_picker_descriptor desc;
  struct cupertino_date_picker *picker = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.year = 2026;
  desc.month = 10;
  desc.day = 1;

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  rc = cupertino_date_picker_create(dummy_engine, &desc, &picker);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, picker);

  g_malloc_fail_countdown = -1;
#else
  rc = cupertino_date_picker_create(NULL, &desc, &picker);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_date_picker_suite) {
  RUN_TEST(test_date_picker_invalid_arguments);
  RUN_TEST(test_date_picker_modes_and_cva);
  RUN_TEST(test_date_picker_oom_mock);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_date_picker_suite);
  GREATEST_MAIN_END();
}
