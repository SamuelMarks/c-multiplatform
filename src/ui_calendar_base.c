/* clang-format off */
#include "ui_calendar_base.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_calendar_base
 * @brief Internal representation of a calendar component.
 */
struct ui_calendar_base {
  struct ui_date selected_date;      /**< Currently selected date */
  struct ui_date min_date;           /**< Minimum selectable date */
  struct ui_date max_date;           /**< Maximum selectable date */
  int has_min;                       /**< 1 if minimum date is set */
  int has_max;                       /**< 1 if maximum date is set */
  int has_selection;                 /**< 1 if a date is currently selected */
  int view_year;                     /**< The year currently being viewed */
  int view_month;                    /**< The month currently being viewed */
  enum ui_day_of_week start_of_week; /**< Day that starts the week */

  ui_calendar_on_select_t on_select; /**< Callback fired upon selection */
  void *on_select_user_data; /**< User data for the selection callback */

  /**
   * @brief CVA: Callback fired when value changes
   */
  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data; /**< CVA: User data for on_change callback */

  /**
   * @brief CVA: Callback fired when input is touched
   */
  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data; /**< CVA: User data for on_touched callback */

  int is_disabled; /**< 1 if the calendar is disabled */
};

static ui_error_t calendar_cva_write_value(void *component,
                                           union ui_signal_payload value) {
  struct ui_calendar_base *cal = (struct ui_calendar_base *)component;
  struct ui_date *date;

  if (!cal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!value.ptr_val) {
    return ui_calendar_base_clear_selection(cal);
  }

  date = (struct ui_date *)value.ptr_val;
  return ui_calendar_base_select_date(cal, date);
}

static ui_error_t calendar_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_calendar_base *cal = (struct ui_calendar_base *)component;
  if (!cal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  cal->cva_on_change = callback;
  cal->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t calendar_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_calendar_base *cal = (struct ui_calendar_base *)component;
  if (!cal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  cal->cva_on_touched = callback;
  cal->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t calendar_cva_set_disabled_state(void *component,
                                                  int is_disabled) {
  struct ui_calendar_base *cal = (struct ui_calendar_base *)component;
  if (!cal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  cal->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_is_leap_year(int year, int *out_is_leap) {
  if (!out_is_leap) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (year % 400 == 0) {
    *out_is_leap = 1;
  } else if (year % 100 == 0) {
    *out_is_leap = 0;
  } else if (year % 4 == 0) {
    *out_is_leap = 1;
  } else {
    *out_is_leap = 0;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_days_in_month(int year, int month, int *out_days) {
  if (!out_days) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (month < 1 || month > 12) {
    *out_days = 0;
    return UI_ERROR_NONE;
  }
  if (month == 2) {
    int is_leap = 0;
    (void)ui_calendar_is_leap_year(year, &is_leap);
    *out_days = is_leap ? 29 : 28;
    return UI_ERROR_NONE;
  }
  if (month == 4 || month == 6 || month == 9 || month == 11) {
    *out_days = 30;
    return UI_ERROR_NONE;
  }
  *out_days = 31;
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_get_day_of_week(int year, int month, int day,
                                       enum ui_day_of_week *out_dow) {
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  int dow;
  if (!out_dow) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (month < 1 || month > 12 || day < 1 || day > 31) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  if (month < 3) {
    year -= 1;
  }
  dow = (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
  *out_dow = (enum ui_day_of_week)dow;
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_create(struct ui_calendar_base **out_calendar,
                                   struct ui_control_value_accessor *out_cva) {
  struct ui_calendar_base *cal;

  if (!out_calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cal = (struct ui_calendar_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_calendar_base));
  if (!cal) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  cal->has_min = 0;
  cal->has_max = 0;
  cal->has_selection = 0;
  cal->view_year = 2026; /* Default fallback */
  cal->view_month = 1;
  cal->start_of_week = UI_SUNDAY;
  cal->on_select = NULL;
  cal->on_select_user_data = NULL;
  cal->cva_on_change = NULL;
  cal->cva_on_change_user_data = NULL;
  cal->cva_on_touched = NULL;
  cal->cva_on_touched_user_data = NULL;
  cal->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = calendar_cva_write_value;
    out_cva->register_on_change = calendar_cva_register_on_change;
    out_cva->register_on_touched = calendar_cva_register_on_touched;
    out_cva->set_disabled_state = calendar_cva_set_disabled_state;
  }

  *out_calendar = cal;
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_destroy(struct ui_calendar_base *calendar) {
  if (!calendar) {
    return UI_ERROR_NONE;
  }
  C_MULTIPLATFORM_FREE(calendar);
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_set_start_of_week(struct ui_calendar_base *calendar,
                                              enum ui_day_of_week start_day) {
  if (!calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)start_day < 0 || (int)start_day > 6) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  calendar->start_of_week = start_day;
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_set_min_date(struct ui_calendar_base *calendar,
                                         const struct ui_date *min_date) {
  if (!calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (min_date) {
    calendar->min_date = *min_date;
    calendar->has_min = 1;
  } else {
    calendar->has_min = 0;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_set_max_date(struct ui_calendar_base *calendar,
                                         const struct ui_date *max_date) {
  if (!calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (max_date) {
    calendar->max_date = *max_date;
    calendar->has_max = 1;
  } else {
    calendar->has_max = 0;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_set_view_month(struct ui_calendar_base *calendar,
                                           int year, int month) {
  if (!calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (month < 1 || month > 12) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  calendar->view_year = year;
  calendar->view_month = month;

  if (calendar->cva_on_touched) {
    ui_error_t rc =
        calendar->cva_on_touched(calendar->cva_on_touched_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_calendar_base_get_view_month(const struct ui_calendar_base *calendar,
                                int *out_year, int *out_month) {
  if (!calendar || !out_year || !out_month) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_year = calendar->view_year;
  *out_month = calendar->view_month;
  return UI_ERROR_NONE;
}

static ui_error_t compare_dates(const struct ui_date *d1,
                                const struct ui_date *d2, int *out_cmp) {
  if (d1->year != d2->year) {
    *out_cmp = d1->year - d2->year;
  } else if (d1->month != d2->month) {
    *out_cmp = d1->month - d2->month;
  } else {
    *out_cmp = d1->day - d2->day;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_select_date(struct ui_calendar_base *calendar,
                                        const struct ui_date *date) {
  int cmp_res = 0;
  if (!calendar || !date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (date->month < 1 || date->month > 12) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  {
    int days = 0;
    (void)ui_calendar_days_in_month(date->year, date->month, &days);
    if (date->day < 1) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
    if (date->day > days) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
  }

  if (calendar->has_min) {
    (void)compare_dates(date, &calendar->min_date, &cmp_res);
    if (cmp_res < 0) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
  }
  if (calendar->has_max) {
    (void)compare_dates(date, &calendar->max_date, &cmp_res);
    if (cmp_res > 0) {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
  }

  calendar->selected_date = *date;
  calendar->has_selection = 1;
  calendar->view_year = date->year;
  calendar->view_month = date->month;

  if (calendar->on_select) {
    ui_error_t select_rc =
        calendar->on_select(calendar, date, calendar->on_select_user_data);
    if (select_rc != UI_ERROR_NONE) {
      return select_rc;
    }
  }

  if (calendar->cva_on_change) {
    union ui_signal_payload payload;
    ui_error_t rc;
    payload.ptr_val = &calendar->selected_date;
    rc = calendar->cva_on_change(payload, calendar->cva_on_change_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_calendar_base_get_selected_date(const struct ui_calendar_base *calendar,
                                   struct ui_date *out_date) {
  if (!calendar || !out_date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!calendar->has_selection) {
    return UI_ERROR_NOT_FOUND;
  }
  *out_date = calendar->selected_date;
  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_clear_selection(struct ui_calendar_base *calendar) {
  if (!calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  calendar->has_selection = 0;

  if (calendar->cva_on_change) {
    union ui_signal_payload payload;
    ui_error_t rc;
    payload.ptr_val = NULL;
    rc = calendar->cva_on_change(payload, calendar->cva_on_change_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_calendar_base_set_on_select(struct ui_calendar_base *calendar,
                                          ui_calendar_on_select_t on_select,
                                          void *user_data) {
  if (!calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  calendar->on_select = on_select;
  calendar->on_select_user_data = user_data;
  return UI_ERROR_NONE;
}

ui_error_t
ui_calendar_base_get_month_grid(const struct ui_calendar_base *calendar,
                                struct ui_date *out_grid, int *out_count) {
  int first_dow, days_in_month, days_in_prev;
  int prev_y, prev_m, next_y, next_m;
  int offset, i, d;

  if (!calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!out_grid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    enum ui_day_of_week tmp_dow = UI_SUNDAY;
    (void)ui_calendar_get_day_of_week(calendar->view_year, calendar->view_month,
                                      1, &tmp_dow);
    first_dow = tmp_dow;
  }
  {
    days_in_month = 0;
    (void)ui_calendar_days_in_month(calendar->view_year, calendar->view_month,
                                    &days_in_month);
  }

  prev_y = calendar->view_year;
  prev_m = calendar->view_month - 1;
  if (prev_m < 1) {
    prev_m = 12;
    prev_y--;
  }

  next_y = calendar->view_year;
  next_m = calendar->view_month + 1;
  if (next_m > 12) {
    next_m = 1;
    next_y++;
  }

  { (void)ui_calendar_days_in_month(prev_y, prev_m, &days_in_prev); }

  offset = first_dow - (int)calendar->start_of_week;
  if (offset < 0) {
    offset += 7;
  }

  *out_count = 0;

  for (i = 0; i < offset; i++) {
    out_grid[*out_count].year = prev_y;
    out_grid[*out_count].month = prev_m;
    out_grid[*out_count].day = days_in_prev - offset + i + 1;
    (*out_count)++;
  }

  for (d = 1; d <= days_in_month; d++) {
    out_grid[*out_count].year = calendar->view_year;
    out_grid[*out_count].month = calendar->view_month;
    out_grid[*out_count].day = d;
    (*out_count)++;
  }

  d = 1;
  while (*out_count < 42) {
    out_grid[*out_count].year = next_y;
    out_grid[*out_count].month = next_m;
    out_grid[*out_count].day = d++;
    (*out_count)++;
  }

  return UI_ERROR_NONE;
}
