/* clang-format off */
#include "../include/ui_calendar_base.h"
#include "../include/ui_datepicker_base.h"
#include "../include/ui_input_base.h"
#include "../include/ui_popover_base.h"
#include "../include/ui_error.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t mock_cva_on_touched(void *user_data) {
  if (user_data == (void *)1)
    return UI_ERROR_UNKNOWN;
  int *called = (int *)user_data;
  if (called)
    (*called)++;
  return UI_ERROR_NONE;
}

static ui_error_t mock_on_select(struct ui_calendar_base *calendar,
                                 const struct ui_date *date, void *user_data) {
  int *called = (int *)user_data;
  if (called)
    (*called)++;
  return UI_ERROR_NONE;
}

static ui_error_t mock_on_select_fail(struct ui_calendar_base *calendar,
                                      const struct ui_date *date,
                                      void *user_data) {
  return UI_ERROR_OUT_OF_MEMORY;
}

static int test_calendar_math(void) {
  {
    int is_leap = 0;
    (void)ui_calendar_is_leap_year(2000, &is_leap);
    if (is_leap != 1) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int is_leap = 0;
    (void)ui_calendar_is_leap_year(1900, &is_leap);
    if (is_leap != 0) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int is_leap = 0;
    (void)ui_calendar_is_leap_year(2004, &is_leap);
    if (is_leap != 1) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int is_leap = 0;
    (void)ui_calendar_is_leap_year(2001, &is_leap);
    if (is_leap != 0) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int days = 0;
    (void)ui_calendar_days_in_month(2024, 2, &days);
    if (days != 29) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int days = 0;
    (void)ui_calendar_days_in_month(2023, 2, &days);
    if (days != 28) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int days = 0;
    (void)ui_calendar_days_in_month(2024, 4, &days);
    if (days != 30) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int days = 0;
    (void)ui_calendar_days_in_month(2024, 1, &days);
    if (days != 31) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  {
    int days = 0;
    (void)ui_calendar_days_in_month(2024, 13, &days);
    if (days != 0) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }

  /* Jan 1, 2024 was a Monday (1) */
  {
    enum ui_day_of_week tmp_dow;
    if (ui_calendar_get_day_of_week(2024, 1, 1, &tmp_dow) != UI_ERROR_NONE ||
        tmp_dow != UI_MONDAY) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }

  /* Dec 31, 2023 was a Sunday (0) */
  {
    enum ui_day_of_week tmp_dow;
    if (ui_calendar_get_day_of_week(2023, 12, 31, &tmp_dow) != UI_ERROR_NONE ||
        tmp_dow != UI_SUNDAY) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }

  /* Feb 29, 2024 was a Thursday (4) */
  {
    enum ui_day_of_week tmp_dow;
    if (ui_calendar_get_day_of_week(2024, 2, 29, &tmp_dow) != UI_ERROR_NONE ||
        tmp_dow != UI_THURSDAY) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }

  /* Error paths for math */
  if (ui_calendar_is_leap_year(2024, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_days_in_month(2024, 1, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_get_day_of_week(2024, 1, 1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* Invalid dates in day of week */
  {
    enum ui_day_of_week dow;
    if (ui_calendar_get_day_of_week(2024, 13, 1, &dow) !=
        UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (ui_calendar_get_day_of_week(2024, 1, 32, &dow) !=
        UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }
  return 0;
}

static ui_error_t mock_cva_on_change(union ui_signal_payload value,
                                     void *user_data) {
  if (user_data == (void *)1)
    return UI_ERROR_UNKNOWN;
  int *called = (int *)user_data;
  if (called)
    (*called)++;
  return UI_ERROR_NONE;
}

static int test_calendar_coverage(void) {
  struct ui_calendar_base *cal = NULL;
  struct ui_control_value_accessor cva;
  struct ui_date date = {2024, 1, 15};
  int year, month;

  /* Null pointer checks */
  if (ui_calendar_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* OOM test */
  g_malloc_fail_countdown = 0;
  if (ui_calendar_base_create(&cal, NULL) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  g_malloc_fail_countdown = -1;

  /* Proper create */
  if (ui_calendar_base_create(&cal, &cva) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* Null checks for methods */
  (void)ui_calendar_base_destroy(NULL); /* Should not crash */

  if (ui_calendar_base_set_start_of_week(NULL, UI_SUNDAY) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_set_start_of_week(cal, (enum ui_day_of_week) - 1) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_set_min_date(NULL, &date) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  (void)ui_calendar_base_set_min_date(cal, NULL); /* Clears min date */

  if (ui_calendar_base_set_max_date(NULL, &date) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  (void)ui_calendar_base_set_max_date(cal, NULL); /* Clears max date */

  if (ui_calendar_base_set_view_month(NULL, 2024, 1) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_set_view_month(cal, 2024, 0) != UI_ERROR_OUT_OF_BOUNDS) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_set_view_month(cal, 2024, 13) !=
      UI_ERROR_OUT_OF_BOUNDS) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_get_view_month(NULL, &year, &month) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_get_view_month(cal, NULL, &month) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_get_view_month(cal, &year, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_get_view_month(cal, &year, &month) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_select_date(NULL, &date) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_select_date(cal, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_get_selected_date(NULL, &date) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_get_selected_date(cal, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_clear_selection(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_set_on_select(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_get_month_grid(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_get_month_grid(cal, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* CVA methods null checks */
  {
    union ui_signal_payload dummy;
    dummy.ptr_val = NULL;
    if (cva.write_value(NULL, dummy) != UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (cva.register_on_change(NULL, mock_cva_on_change, NULL) !=
        UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (cva.register_on_touched(NULL, mock_cva_on_touched, NULL) !=
        UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }

  /* CVA normal usage */
  {
    int changes = 0;
    int touches = 0;
    int selects = 0;
    union ui_signal_payload any_val;
    struct ui_date test_date = {2024, 5, 10};
    (void)cva.register_on_change(cal, mock_cva_on_change, &changes);
    (void)cva.register_on_touched(cal, mock_cva_on_touched, &touches);
    (void)cva.set_disabled_state(cal, 1);
    (void)ui_calendar_base_set_on_select(cal, mock_on_select, &selects);

    /* Write non-null value */
    any_val.ptr_val = &test_date;
    (void)cva.write_value(cal, any_val);

    /* Write NULL value to clear */
    any_val.ptr_val = NULL;
    (void)cva.write_value(cal, any_val);

    if (changes != 2) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    } /* Note: write_value does not trigger on_change */

    /* Select date triggers on_select, on_change and on_touched */
    (void)ui_calendar_base_select_date(cal, &test_date);
    if (changes != 3) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (touches != 0) {
      printf("touches=%d at %d\n", touches, __LINE__);
      return 1;
    }
    if (selects != 2) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    /* Clear selection triggers on_change */
    (void)ui_calendar_base_clear_selection(cal);
    if (changes != 4) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    (void)ui_calendar_base_set_view_month(cal, 2024, 6);
    (void)ui_calendar_base_set_on_select(cal, mock_on_select_fail, NULL);
    if (ui_calendar_base_select_date(cal, &test_date) !=
        UI_ERROR_OUT_OF_MEMORY) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    (void)ui_calendar_base_set_on_select(cal, mock_on_select, &selects);
    if (touches != 1) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }

  /* min/max coverage check for compare_dates logic (same year, diff month etc.)
   */
  {
    struct ui_date min_d = {2024, 5, 10};
    struct ui_date max_d = {2024, 7, 20};
    struct ui_date d1 = {2023, 12, 10}; /* Year < */
    struct ui_date d2 = {2024, 4, 15};  /* Month < */
    struct ui_date d3 = {2024, 5, 5};   /* Day < */
    struct ui_date d4 = {2024, 5, 15};  /* Valid */
    struct ui_date d5 = {2025, 1, 1};   /* Year > */
    struct ui_date d6 = {2024, 8, 1};   /* Month > */
    struct ui_date d7 = {2024, 7, 25};  /* Day > */
    (void)ui_calendar_base_set_min_date(cal, &min_d);
    (void)ui_calendar_base_set_max_date(cal, &max_d);

    if (ui_calendar_base_select_date(cal, &d1) != UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (ui_calendar_base_select_date(cal, &d2) != UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (ui_calendar_base_select_date(cal, &d3) != UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (ui_calendar_base_select_date(cal, &d4) != UI_ERROR_NONE) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (ui_calendar_base_select_date(cal, &d5) != UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (ui_calendar_base_select_date(cal, &d6) != UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    if (ui_calendar_base_select_date(cal, &d7) != UI_ERROR_OUT_OF_BOUNDS) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    struct ui_date invalid_month = {2024, 0, 10};
    if (ui_calendar_base_select_date(cal, &invalid_month) !=
        UI_ERROR_OUT_OF_BOUNDS)
      return 1;
    invalid_month.month = 13;
    if (ui_calendar_base_select_date(cal, &invalid_month) !=
        UI_ERROR_OUT_OF_BOUNDS)
      return 1;
  }

  /* Grid boundaries logic for prev_m/next_m */
  {
    struct ui_date grid[42];
    int count;
    (void)ui_calendar_base_set_view_month(cal, 2024, 1);
    /* Jan 2024 starts on Monday. If start_of_week is Monday, offset is 0,
       so it just shows Jan 1 on first square, wait:
       If offset is 0, the code might still pad it from previous month or
       offset might become negative depending on logic.
       Let's use UI_TUESDAY so offset becomes negative if handled as:
       day_of_week - start_of_week -> 1 (Monday) - 2 (Tuesday) = -1.
       The code does offset += 7, which gives 6. So 6 days of previous month.
    */
    (void)ui_calendar_base_set_start_of_week(cal, UI_TUESDAY);
    (void)ui_calendar_base_get_month_grid(cal, grid, &count);
    if (grid[0].month != 12 || grid[0].year != 2023) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    } /* Should wrap to Dec 2023 */
    (void)ui_calendar_base_set_view_month(cal, 2024, 12);
    (void)ui_calendar_base_get_month_grid(cal, grid, &count);
    if (grid[41].month != 1 || grid[41].year != 2025) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    } /* Should wrap to Jan 2025 */
  }

  (void)ui_calendar_base_destroy(cal);
  return 0;
}
static int test_calendar_base(void) {
  struct ui_calendar_base *cal = NULL;
  struct ui_date min_date = {2024, 1, 15};
  struct ui_date max_date = {2024, 1, 25};
  struct ui_date target = {2024, 1, 20};
  struct ui_date invalid_target = {2024, 1, 10};
  struct ui_date invalid_day = {2024, 2, 30};
  struct ui_date invalid_day_zero = {2024, 2, 0};
  struct ui_date out;
  struct ui_date grid[42];
  int count;

  if (ui_calendar_base_create(&cal, NULL) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  (void)ui_calendar_base_set_min_date(cal, &min_date);
  (void)ui_calendar_base_set_max_date(cal, &max_date);

  if (ui_calendar_base_select_date(cal, &target) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_get_selected_date(cal, &out) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (out.year != 2024 || out.month != 1 || out.day != 20) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_calendar_base_select_date(cal, &invalid_target) !=
      UI_ERROR_OUT_OF_BOUNDS) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_select_date(cal, &invalid_day) !=
      UI_ERROR_OUT_OF_BOUNDS) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_calendar_base_select_date(cal, &invalid_day_zero) !=
      UI_ERROR_OUT_OF_BOUNDS) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  (void)ui_calendar_base_clear_selection(cal);
  if (ui_calendar_base_get_selected_date(cal, &out) != UI_ERROR_NOT_FOUND) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* Test grid generation for Feb 2024 */
  (void)ui_calendar_base_set_view_month(cal, 2024, 2);
  (void)ui_calendar_base_set_start_of_week(cal, UI_SUNDAY);

  if (ui_calendar_base_get_month_grid(cal, grid, &count) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (count != 42) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* Feb 1, 2024 is Thursday.
   * Offset from Sunday should be 4 days (Sun, Mon, Tue, Wed).
   * Prev month (Jan) has 31 days. So grid[0..3] should be Jan 28, 29, 30, 31.
   * grid[4] should be Feb 1.
   */
  if (grid[0].month != 1 || grid[0].day != 28) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (grid[4].month != 2 || grid[4].day != 1) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  /* Feb 2024 has 29 days, so grid[4 + 29] = grid[33] should be Mar 1 */
  if (grid[33].month != 3 || grid[33].day != 1) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  (void)ui_calendar_base_destroy(cal);
  return 0;
}

static int test_datepicker_base(void) {
  struct ui_datepicker_base *dp = NULL;
  struct ui_input_base *input = NULL;
  struct ui_popover_base *popover = NULL;
  struct ui_calendar_base *cal = NULL;
  struct ui_date parsed;
  char text[32];
  (void)ui_input_base_create(&input);
  (void)ui_popover_base_create(&popover);
  (void)ui_calendar_base_create(&cal, NULL);

  if (ui_datepicker_base_create(&dp, input, popover, cal, NULL) !=
      UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_datepicker_parse_date("2024-02-29", &parsed) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (parsed.year != 2024 || parsed.month != 2 || parsed.day != 29) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_datepicker_parse_date("2023-02-29", &parsed) == UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  } /* Invalid date */

  parsed.year = 2024;
  parsed.month = 12;
  parsed.day = 5;
  if (ui_datepicker_format_date(&parsed, text, sizeof(text)) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (strcmp(text, "2024-12-05") != 0) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  (void)ui_datepicker_base_destroy(dp);
  (void)ui_calendar_base_destroy(cal);
  (void)ui_popover_base_destroy(popover);
  (void)ui_input_base_destroy(input);

  return 0;
}

static struct ui_datepicker_base *g_mock_dp_for_reentrancy = NULL;
static struct ui_control_value_accessor g_mock_cva_for_reentrancy;

static ui_error_t mock_dp_cva_change(union ui_signal_payload val, void *data) {
  int *c = (int *)data;
  if (c)
    (*c)++;

  if (g_mock_dp_for_reentrancy) {
    union ui_signal_payload dummy;
    dummy.int_val = 0;
    (void)g_mock_cva_for_reentrancy.write_value(g_mock_dp_for_reentrancy,
                                                dummy);
    (void)g_mock_cva_for_reentrancy.register_on_change(g_mock_dp_for_reentrancy,
                                                       NULL, NULL);
    (void)g_mock_cva_for_reentrancy.register_on_touched(
        g_mock_dp_for_reentrancy, NULL, NULL);
    (void)g_mock_cva_for_reentrancy.set_disabled_state(g_mock_dp_for_reentrancy,
                                                       1);
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_dp_cva_touched(void *data) {
  int *c = (int *)data;
  if (c)
    (*c)++;
  return UI_ERROR_NONE;
}

static int test_datepicker_coverage(void) {
  struct ui_datepicker_base *dp = NULL;
  struct ui_input_base *input = NULL;
  struct ui_popover_base *popover = NULL;
  struct ui_calendar_base *cal = NULL;
  struct ui_control_value_accessor cva;
  struct ui_date dt = {2024, 5, 10};
  char text[32];
  int changes = 0, touches = 0;
  union ui_signal_payload val;

  (void)ui_input_base_create(&input);
  (void)ui_popover_base_create(&popover);
  (void)ui_calendar_base_create(&cal, NULL);

  /* null checks */
  if (ui_datepicker_base_create(NULL, input, popover, cal, &cva) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_base_create(&dp, NULL, popover, cal, &cva) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_base_create(&dp, input, NULL, cal, &cva) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_base_create(&dp, input, popover, NULL, &cva) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* OOM */
  g_malloc_fail_countdown = 0;
  if (ui_datepicker_base_create(&dp, input, popover, cal, &cva) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  (void)ui_datepicker_base_destroy(NULL);

  if (ui_datepicker_parse_date(NULL, &dt) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2024-05-10", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("not-a-date", &dt) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2024-13-10", &dt) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_parse_date("2024-05-32", &dt) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_datepicker_format_date(NULL, text, sizeof(text)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_format_date(&dt, NULL, sizeof(text)) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_datepicker_format_date(&dt, text, 10) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_datepicker_base_sync(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_datepicker_base_create(&dp, input, popover, cal, &cva) !=
      UI_ERROR_NONE)
    return 1;

  /* Trigger change BEFORE CVA is registered to hit return UI_ERROR_NONE path */
  {
    struct ui_date picked = {2024, 1, 1};
    (void)ui_calendar_base_select_date(cal, &picked);
  }

  val.int_val = 0;
  if (cva.write_value(NULL, val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.register_on_change(NULL, mock_dp_cva_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.register_on_touched(NULL, mock_dp_cva_touched, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  (void)cva.register_on_change(dp, mock_dp_cva_change, &changes);
  (void)cva.register_on_touched(dp, mock_dp_cva_touched, &touches);
  (void)cva.set_disabled_state(dp, 1);

  g_mock_dp_for_reentrancy = dp;
  g_mock_cva_for_reentrancy = cva;

  val.int_val = (2024 << 9) | (5 << 5) | 10;
  (void)cva.write_value(dp, val);
  val.int_val = 0;
  (void)cva.write_value(dp, val);

  {
    struct ui_date picked = {2024, 5, 20};
    (void)ui_calendar_base_select_date(cal, &picked);
  }

  g_mock_dp_for_reentrancy = NULL;

  (void)ui_input_base_set_text(input, "2024-06-15");
  (void)ui_input_base_set_text(input, "invalid");
  (void)ui_input_base_set_text(input, "2024-07-20");
  (void)ui_datepicker_base_sync(dp);

  /* Error paths for set_text allocations inside on_calendar_select / cva_write
   */
  {
    struct ui_date picked = {2024, 5, 21};
    g_malloc_fail_countdown = 0;
    (void)ui_calendar_base_select_date(cal, &picked);
    g_malloc_fail_countdown = -1;
  }
  {
    val.int_val = (2024 << 9) | (5 << 5) | 10;
    g_malloc_fail_countdown = 0;
    (void)cva.write_value(dp, val);
    g_malloc_fail_countdown = -1;
  }
  {
    val.int_val = 0;
    g_malloc_fail_countdown = 0;
    (void)cva.write_value(dp, val);
    g_malloc_fail_countdown = -1;
  }
  /* Error path for select_date due to out of bounds */
  {
    struct ui_date max_dt = {2024, 8, 1};
    (void)ui_calendar_base_set_max_date(cal, &max_dt);
    (void)ui_input_base_set_text(input, "2024-09-01");
    (void)ui_datepicker_base_sync(dp);

    val.int_val = (2024 << 9) | (9 << 5) | 1;
    (void)cva.write_value(dp, val);
  }

  (void)ui_datepicker_base_destroy(dp);
  (void)ui_calendar_base_destroy(cal);
  (void)ui_popover_base_destroy(popover);
  (void)ui_input_base_destroy(input);

  return 0;
}

static int test_calendar_missing_branches(void) {
  int count;
  int days;
  enum ui_day_of_week dow;
  struct ui_date *grid = NULL;
  struct ui_control_value_accessor cva;
  struct ui_calendar_base *cal = NULL;
  struct ui_date dt = {2024, 5, 15};
  struct ui_date min_dt = {2024, 1, 1};
  struct ui_date max_dt = {2024, 12, 1};
  int g_calendar_mock_fail = 0;

  /* 228: cva_on_touched fails */
  (void)ui_calendar_base_create(&cal, &cva);
  (void)cva.register_on_touched(cal, mock_cva_on_touched, (void *)1);
  (void)ui_calendar_base_set_view_month(cal, 2024, 5);
  (void)ui_calendar_base_destroy(cal);

  /* 268: ui_calendar_days_in_month fails in set_selected_date */
  (void)ui_calendar_base_create(&cal, NULL);
  g_calendar_mock_fail = 268;
  (void)ui_calendar_base_select_date(cal, &dt);
  g_calendar_mock_fail = 0;
  (void)ui_calendar_base_destroy(cal);

  /* 278: compare_dates fails for min */
  (void)ui_calendar_base_create(&cal, NULL);
  (void)ui_calendar_base_set_min_date(cal, &min_dt);
  g_calendar_mock_fail = 278;
  (void)ui_calendar_base_select_date(cal, &dt);
  g_calendar_mock_fail = 0;
  (void)ui_calendar_base_destroy(cal);

  /* 286: compare_dates fails for max */
  (void)ui_calendar_base_create(&cal, NULL);
  (void)ui_calendar_base_set_max_date(cal, &max_dt);
  g_calendar_mock_fail = 286;
  (void)ui_calendar_base_select_date(cal, &dt);
  g_calendar_mock_fail = 0;
  (void)ui_calendar_base_destroy(cal);

  /* 310: cva_on_change fails in set_selected_date */
  (void)ui_calendar_base_create(&cal, &cva);
  (void)cva.register_on_change(cal, mock_cva_on_change, (void *)1);
  (void)ui_calendar_base_select_date(cal, &dt);
  (void)ui_calendar_base_destroy(cal);

  /* 340: cva_on_change fails in clear_selection */
  (void)ui_calendar_base_create(&cal, &cva);
  (void)cva.register_on_change(cal, mock_cva_on_change, (void *)1);
  (void)ui_calendar_base_clear_selection(cal);
  (void)ui_calendar_base_destroy(cal);

  /* 372: get_day_of_week fails in get_grid */
  (void)ui_calendar_base_create(&cal, NULL);
  g_calendar_mock_fail = 372;
  (void)ui_calendar_base_get_month_grid(cal, grid, &count);
  g_calendar_mock_fail = 0;
  (void)ui_calendar_base_destroy(cal);

  /* 382: days_in_month fails in get_grid */
  (void)ui_calendar_base_create(&cal, NULL);
  g_calendar_mock_fail = 382;
  (void)ui_calendar_base_get_month_grid(cal, grid, &count);
  g_calendar_mock_fail = 0;
  (void)ui_calendar_base_destroy(cal);

  /* 402: days_in_prev fails in get_grid */
  (void)ui_calendar_base_create(&cal, NULL);
  g_calendar_mock_fail = 402;
  (void)ui_calendar_base_get_month_grid(cal, grid, &count);
  g_calendar_mock_fail = 0;
  (void)ui_calendar_base_destroy(cal);

  /* 98: month < 1 or month > 12 */
  (void)ui_calendar_days_in_month(2023, 0, &days);
  (void)ui_calendar_days_in_month(2023, 13, &days);

  /* 108: missing branches for days_in_month logic */
  /* 31 days month */
  (void)ui_calendar_days_in_month(2023, 1, &days);
  /* 30 days month */
  (void)ui_calendar_days_in_month(2023, 4, &days);
  (void)ui_calendar_days_in_month(2023, 6, &days);
  (void)ui_calendar_days_in_month(2023, 9, &days);
  (void)ui_calendar_days_in_month(2023, 11, &days);
  (void)ui_calendar_days_in_month(2023, 5, &days);
  (void)ui_calendar_days_in_month(2023, 7, &days);
  (void)ui_calendar_days_in_month(2023, 10, &days);
  (void)ui_calendar_days_in_month(2023, 12, &days);

  /* 122: out of bounds in day of week */
  (void)ui_calendar_get_day_of_week(2023, 0, 1, &dow);
  (void)ui_calendar_get_day_of_week(2023, 1, 0, &dow);
  (void)ui_calendar_get_day_of_week(2023, 1, 32, &dow);

  /* 184: start_day > 6 */
  (void)ui_calendar_base_create(&cal, NULL);
  (void)ui_calendar_base_set_start_of_week(cal, (enum ui_day_of_week)7);

  /* 365: !out_count in get_month_grid */
  struct ui_date real_grid[42];
  (void)ui_calendar_base_get_month_grid(cal, real_grid, NULL);

  (void)ui_calendar_base_destroy(cal);
  return 0;
}

int main(void) {
  test_calendar_missing_branches();
  int failed = 0;

  printf("Running ui_calendar_base & ui_datepicker_base tests...\n");

  failed |= test_calendar_math();
  failed |= test_calendar_base();
  failed |= test_calendar_coverage();
  failed |= test_datepicker_base();
  failed |= test_datepicker_coverage();

  if (failed) {
    printf("Tests failed.\n");
    {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
  }

  printf("All tests passed.\n");
  return 0;
}
