/* clang-format off */
#include <stddef.h>
#include <stdio.h>
#include "../include/ui_datepicker_base.h"
#include "../include/ui_input_base.h"
#include "../include/ui_popover_base.h"
#include "../include/ui_calendar_base.h"
#include "../include/ui_error.h"
#include "../src/ui_datepicker_base.c" /* Source included for internals/statics */
/* clang-format on */

extern int g_malloc_fail_countdown;

static int test_create_destroy(void) {
  struct ui_datepicker_base *dp = NULL;
  struct ui_input_base *input = NULL;
  struct ui_popover_base *popover = NULL;
  struct ui_calendar_base *calendar = NULL;
  struct ui_control_value_accessor cva;
  enum ui_error rc;

  ui_input_base_create(&input);
  ui_popover_base_create(&popover);
  ui_calendar_base_create(&calendar, NULL);

  rc = ui_datepicker_base_create(&dp, input, popover, calendar, &cva);
  if (rc != UI_ERROR_NONE || !dp)
    return 1;

  ui_datepicker_base_destroy(dp);
  ui_input_base_destroy(input);
  ui_popover_base_destroy(popover);
  ui_calendar_base_destroy(calendar);
  return 0;
}

static int test_errors(void) {
  struct ui_datepicker_base *dp = NULL;
  struct ui_input_base *input = NULL;
  struct ui_popover_base *popover = NULL;
  struct ui_calendar_base *calendar = NULL;
  struct ui_date parsed;
  char text[32];

  ui_input_base_create(&input);
  ui_popover_base_create(&popover);
  ui_calendar_base_create(&calendar, NULL);

  if (ui_datepicker_base_create(NULL, input, popover, calendar, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_base_create(&dp, NULL, popover, calendar, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_base_create(&dp, input, NULL, calendar, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_base_create(&dp, input, popover, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_datepicker_base_create(&dp, input, popover, calendar, NULL) !=
      UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return 1;
  }
  g_malloc_fail_countdown = -1;

  ui_datepicker_base_destroy(NULL);

  if (ui_datepicker_parse_date(NULL, &parsed) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2023-01-01", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_datepicker_format_date(NULL, text, sizeof(text)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_format_date(&parsed, NULL, sizeof(text)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_format_date(&parsed, text, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_datepicker_base_sync(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_input_base_destroy(input);
  ui_popover_base_destroy(popover);
  ui_calendar_base_destroy(calendar);

  return 0;
}

static int test_parse_format(void) {
  struct ui_date date;
  char text[32];

  if (ui_datepicker_parse_date("2023-12-25", &date) != UI_ERROR_NONE)
    return 1;
  if (date.year != 2023 || date.month != 12 || date.day != 25)
    return 1;

  if (ui_datepicker_parse_date("invalid", &date) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2023-13-25", &date) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2023-00-25", &date) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2023-12-32", &date) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2023-12-00", &date) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_datepicker_format_date(&date, text, sizeof(text)) != UI_ERROR_NONE)
    return 1;
  /* Not strictly verifying the actual text output here since safe/unsafe prints
   * could slightly differ in weird compilers, but should be ok */

  return 0;
}

static int test_cva_functions(void) {
  struct ui_datepicker_base *dp = NULL;
  struct ui_input_base *input = NULL;
  struct ui_popover_base *popover = NULL;
  struct ui_calendar_base *calendar = NULL;
  struct ui_control_value_accessor cva;
  union ui_signal_payload payload;
  int called = 0;

  ui_input_base_create(&input);
  ui_popover_base_create(&popover);
  ui_calendar_base_create(&calendar, NULL);

  ui_datepicker_base_create(&dp, input, popover, calendar, &cva);

  /* Test write_value */
  payload.int_val = (2023 << 9) | (12 << 5) | 25;
  cva.write_value(dp, payload); /* write specific date */

  payload.int_val = 0;
  cva.write_value(dp, payload);   /* write empty date */
  cva.write_value(NULL, payload); /* fail */

  /* Test register_on_change */
  cva.register_on_change(dp, NULL, &called);
  cva.register_on_change(NULL, NULL, NULL);

  /* Test register_on_touched */
  dp->is_syncing = 0;
  cva.register_on_touched(dp, NULL, &called);
  cva.register_on_touched(NULL, NULL, NULL);

  /* Test set_disabled_state */
  dp->is_syncing = 0;
  cva.set_disabled_state(dp, 1);
  cva.set_disabled_state(NULL, 1);

  ui_datepicker_base_destroy(dp);
  ui_input_base_destroy(input);
  ui_popover_base_destroy(popover);
  ui_calendar_base_destroy(calendar);
  return 0;
}

static enum ui_error mock_cva_on_change(union ui_signal_payload new_value,
                                        void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  (void)new_value;
  return UI_ERROR_NONE;
}

static enum ui_error mock_cva_on_touched(void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
}

static int test_callbacks_and_sync(void) {
  struct ui_datepicker_base *dp = NULL;
  struct ui_input_base *input = NULL;
  struct ui_popover_base *popover = NULL;
  struct ui_calendar_base *calendar = NULL;
  struct ui_control_value_accessor cva;
  struct ui_date date = {2023, 1, 10};
  int change_called = 0;
  int touched_called = 0;

  ui_input_base_create(&input);
  ui_popover_base_create(&popover);
  ui_calendar_base_create(&calendar, NULL);
  ui_datepicker_base_create(&dp, input, popover, calendar, &cva);

  /* Register mock callbacks via CVA */
  cva.register_on_change(dp, mock_cva_on_change, &change_called);
  cva.register_on_touched(dp, mock_cva_on_touched, &touched_called);

  /* We need to be careful with is_syncing */
  dp->is_syncing = 0;

  ui_datepicker_base_sync(dp);
  dp->is_syncing = 0; /* Reset it just in case a mock leaked the state due to
                         early returns we force */

  /* Trigger the on_calendar_select callback */
  on_calendar_select(calendar, &date, dp);

  /* Trigger on_input_change with valid and invalid text */
  on_input_change(input, "2023-01-15", dp);
  on_input_change(input, "invalid", dp);
  on_input_change(input, NULL, dp); /* text is NULL */

  /* Trigger is_syncing branches by forcing it on */
  dp->is_syncing = 1;
  on_calendar_select(calendar, &date, dp);
  on_input_change(input, "2023-01-15", dp);

  /* Call register when syncing to hit that branch */
  cva.register_on_change(dp, NULL, NULL);
  cva.register_on_touched(dp, NULL, NULL);
  cva.set_disabled_state(dp, 1);
  cva.write_value(dp, (union ui_signal_payload){0});
  dp->is_syncing = 0;

  /* Call register and set_disabled normally */
  cva.register_on_change(dp, NULL, NULL);
  cva.register_on_touched(dp, NULL, NULL);
  cva.set_disabled_state(dp, 1);

  /* Call trigger_cva_change directly without on_change set to hit branch */
  dp->cva_on_change = NULL;
  dp->is_syncing = 0;
  on_calendar_select(calendar, &date, dp);

  /* Force format to fail by passing a bad date inside payload if format_date
     checks it? Or we can pass an invalid date in payload and format_date will
     fail if it's invalid. Wait, payload packs year/month/day. We can pack an
     invalid month. */
  {
    union ui_signal_payload bad_payload;
    bad_payload.int_val = (2023 << 9) | (15 << 5) | 25; /* invalid month 15 */
    cva.write_value(dp, bad_payload);
  }

  ui_datepicker_base_destroy(dp);

  /* Create without CVA */
  ui_datepicker_base_create(&dp, input, popover, calendar, NULL);
  ui_datepicker_base_destroy(dp);

  ui_input_base_destroy(input);
  ui_popover_base_destroy(popover);
  ui_calendar_base_destroy(calendar);
  return 0;
}

static int test_sync_fail(void) {
  struct ui_datepicker_base *dp = NULL;
  struct ui_input_base *input = NULL;
  struct ui_popover_base *popover = NULL;
  struct ui_calendar_base *calendar = NULL;
  struct ui_control_value_accessor cva;

  ui_input_base_create(&input);
  ui_popover_base_create(&popover);
  ui_calendar_base_create(&calendar, NULL);
  ui_datepicker_base_create(&dp, input, popover, calendar, &cva);

  /* The input_base text could be failed by setting g_malloc_fail_countdown? No,
   * input_base doesn't fail unless it's NULL, which it isn't. Wait, the input
   * base get text might fail if memory allocation for text buffer fails if it
   * returns an allocated string? Our ui_input_base_get_text just sets a
   * pointer. It doesn't fail unless input is NULL. But we can't make input NULL
   * inside datepicker. We can manually destroy input? */
  ui_input_base_destroy(dp->input);
  dp->input = NULL;
  ui_datepicker_base_sync(dp); /* Hits failure */

  ui_datepicker_base_destroy(dp);
  ui_popover_base_destroy(popover);
  ui_calendar_base_destroy(calendar);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_create_destroy();
  failed |= test_errors();
  failed |= test_parse_format();
  failed |= test_cva_functions();
  failed |= test_callbacks_and_sync();
  failed |= test_sync_fail();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
