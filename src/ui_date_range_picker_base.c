/**
 * @file ui_date_range_picker_base.c
 * @brief ui_date_range_picker_base.c implementation.
 */
/* clang-format off */
#include "ui_date_range_picker_base.h"
#include "ui_internal_mem.h"
#include <string.h>

#ifdef UI_TEST_MOCK_ALLOC
int g_date_range_picker_mock_fail = 0;

static ui_error_t mock_drp_calendar_days_in_month(int year, int month,
                                                  int *out_days) {
  if (g_date_range_picker_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_date_range_picker_mock_fail == 2) {
    static int days_call_count = 0;
    days_call_count++;
    if (days_call_count > 1) {
      days_call_count = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
  return (ui_calendar_days_in_month)(year, month, out_days);
}
#undef ui_calendar_days_in_month
/** @cond */
#define ui_calendar_days_in_month mock_drp_calendar_days_in_month
/** @endcond */

static ui_error_t mock_drp_component_destroy(struct ui_component *comp) {
  if (g_date_range_picker_mock_fail == 3) {
    (ui_component_destroy)(comp);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_drp_component_destroy
/** @endcond */

#endif
/* clang-format on */

/**
 * @struct ui_date_range_picker_base
 * @struct ui_date_range_picker_base
 * @brief Internal representation of a date range picker component.
 */
struct ui_date_range_picker_base {
  /* @brief Associated UI component. */
  struct ui_component *component; /**< component */
  /* @brief Current state of the picker. */
  enum ui_date_range_picker_state state; /**< state */
  /* @brief Start date of the range. */
  struct ui_date start_date; /**< start_date */
  /* @brief End date of the range. */
  struct ui_date end_date; /**< end_date */
  /* @brief Date currently hovered. */
  struct ui_date hover_date; /**< hover_date */

  /* @brief Predicate callback to disable certain dates. */
  ui_date_predicate_cb predicate_cb; /**< predicate_cb */
  /* @brief User data for the predicate callback. */
  void *predicate_user_data; /**< predicate_user_data */

  /* @brief Callback fired when the selected range changes. */
  ui_date_range_on_change_cb on_change_cb; /**< on_change_cb */
  /* @brief User data for the on_change callback. */
  void *on_change_user_data; /**< on_change_user_data */
};

/**
 * @brief ui_date_compare.
 * @param a Parameter a.
 * @param b Parameter b.
 * @param out_result Parameter out_result.
 * @return Return value.
 */
ui_error_t ui_date_compare(const struct ui_date *a, const struct ui_date *b,
                           int *out_result) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_date_range_picker_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_date_range_picker_mock_fail == 6) {
    static int compare_call_count = 0;
    compare_call_count++;
    if (compare_call_count > 1) {
      compare_call_count = 0;
      return UI_ERROR_UNKNOWN;
    }
  }
#endif
  if (!a || !b || !out_result) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (a->year != b->year) {
    *out_result = a->year - b->year;
    return UI_ERROR_NONE;
  }
  if (a->month != b->month) {
    *out_result = a->month - b->month;
    return UI_ERROR_NONE;
  }
  *out_result = a->day - b->day;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_is_valid.
 * @param date Parameter date.
 * @param out_is_valid Parameter out_is_valid.
 * @return Return value.
 */
ui_error_t ui_date_is_valid(const struct ui_date *date,
                            ui_bool_t *out_is_valid) {
  int days;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_date_range_picker_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
#endif

  if (!date || !out_is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_valid = UI_FALSE;

  if (date->month < 1 || date->month > 12) {
    return UI_ERROR_NONE;
  }
  if (date->day < 1) {
    return UI_ERROR_NONE;
  }

  rc = ui_calendar_days_in_month(date->year, date->month, &days);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (date->day <= days) {
    *out_is_valid = UI_TRUE;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_create.
 * @param out_picker Parameter out_picker.
 * @return Return value.
 */
ui_error_t ui_date_range_picker_base_create(
    struct ui_date_range_picker_base **out_picker) {
  struct ui_date_range_picker_base *picker;
  ui_error_t rc;

  if (!out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct ui_date_range_picker_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_date_range_picker_base));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(picker, 0, sizeof(struct ui_date_range_picker_base));

  rc = ui_component_create(&picker->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  picker->state = UI_DATE_RANGE_PICKER_STATE_IDLE;

  *out_picker = picker;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_destroy.
 * @param picker Parameter picker.
 * @return Return value.
 */
ui_error_t
ui_date_range_picker_base_destroy(struct ui_date_range_picker_base *picker) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (picker->component) {
    ui_error_t rc_cleanup = ui_component_destroy(picker->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      rc = rc_cleanup;
    }
  }
  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

/**
 * @brief ui_date_range_picker_base_set_disable_predicate.
 * @param picker Parameter picker.
 * @param predicate Parameter predicate.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
ui_error_t ui_date_range_picker_base_set_disable_predicate(
    struct ui_date_range_picker_base *picker, ui_date_predicate_cb predicate,
    void *user_data) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->predicate_cb = predicate;
  picker->predicate_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_set_on_change.
 * @param picker Parameter picker.
 * @param on_change Parameter on_change.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
ui_error_t ui_date_range_picker_base_set_on_change(
    struct ui_date_range_picker_base *picker,
    ui_date_range_on_change_cb on_change, void *user_data) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->on_change_cb = on_change;
  picker->on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/* Helper to check if any date in a range is disabled. */
/**
 * @brief check_range_validity.
 * @param picker Parameter picker.
 * @param start Parameter start.
 * @param end Parameter end.
 * @param out_valid Parameter out_valid.
 * @return Return value.
 */
static ui_error_t check_range_validity(struct ui_date_range_picker_base *picker,
                                       const struct ui_date *start,
                                       const struct ui_date *end,
                                       ui_bool_t *out_valid) {
  struct ui_date current;
  int cmp;
  int days;
  ui_error_t rc;

  *out_valid = UI_TRUE;
  if (!picker->predicate_cb) {
    return UI_ERROR_NONE;
  }

  current = *start;
  for (;;) {
    rc = ui_date_compare(&current, end, &cmp);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    if (cmp > 0) {
      break;
    }

    if (picker->predicate_cb(&current, picker->predicate_user_data)) {
      *out_valid = UI_FALSE;
      break;
    }

    /* Advance current by 1 day */
    rc = ui_calendar_days_in_month(current.year, current.month, &days);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    current.day++;
    if (current.day > days) {
      current.day = 1;
      current.month++;
      if (current.month > 12) {
        current.month = 1;
        current.year++;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_select_date.
 * @param picker Parameter picker.
 * @param date Parameter date.
 * @return Return value.
 */
ui_error_t
ui_date_range_picker_base_select_date(struct ui_date_range_picker_base *picker,
                                      const struct ui_date *date) {
  ui_bool_t is_valid, range_valid;
  ui_error_t rc;
  int cmp;

  if (!picker || !date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_date_is_valid(date, &is_valid);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  if (!is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->predicate_cb &&
      picker->predicate_cb(date, picker->predicate_user_data)) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->state == UI_DATE_RANGE_PICKER_STATE_IDLE) {
    picker->start_date = *date;
    picker->hover_date = *date;
    picker->state = UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE;
  } else if (picker->state == UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE) {
    rc = ui_date_compare(date, &picker->start_date, &cmp);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }

    if (cmp < 0) {
      /* Date selected is before start_date, restart selection */
      picker->start_date = *date;
      picker->hover_date = *date;
    } else {
      /* Validate the whole range doesn't contain disabled dates */
      rc =
          check_range_validity(picker, &picker->start_date, date, &range_valid);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }

      if (range_valid) {
        struct ui_date_range range;
        picker->end_date = *date;
        picker->state = UI_DATE_RANGE_PICKER_STATE_IDLE;

        if (picker->on_change_cb) {
          range.start_date = picker->start_date;
          range.end_date = picker->end_date;
          rc =
              picker->on_change_cb(picker, &range, picker->on_change_user_data);
          if (rc != UI_ERROR_NONE) {
            return rc;
          }
        }
      } else {
        /* If disabled dates in between, restart selection from the new date */
        picker->start_date = *date;
        picker->hover_date = *date;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_set_hover_date.
 * @param picker Parameter picker.
 * @param date Parameter date.
 * @return Return value.
 */
ui_error_t ui_date_range_picker_base_set_hover_date(
    struct ui_date_range_picker_base *picker, const struct ui_date *date) {
  ui_bool_t is_valid;
  ui_error_t rc;

  if (!picker || !date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_date_is_valid(date, &is_valid);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  if (!is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->state == UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE) {
    picker->hover_date = *date;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_get_state.
 * @param picker Parameter picker.
 * @param out_state Parameter out_state.
 * @return Return value.
 */
ui_error_t ui_date_range_picker_base_get_state(
    const struct ui_date_range_picker_base *picker,
    enum ui_date_range_picker_state *out_state) {
  if (!picker || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_state = picker->state;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_get_range.
 * @param picker Parameter picker.
 * @param out_range Parameter out_range.
 * @return Return value.
 */
ui_error_t ui_date_range_picker_base_get_range(
    const struct ui_date_range_picker_base *picker,
    struct ui_date_range *out_range) {
  int cmp;
  ui_error_t rc;

  if (!picker || !out_range) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(out_range, 0, sizeof(struct ui_date_range));

  if (picker->state == UI_DATE_RANGE_PICKER_STATE_IDLE) {
    out_range->start_date = picker->start_date;
    out_range->end_date = picker->end_date;
  } else {
    out_range->start_date = picker->start_date;
    rc = ui_date_compare(&picker->hover_date, &picker->start_date, &cmp);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    if (cmp < 0) {
      out_range->end_date = picker->start_date;
    } else {
      out_range->end_date = picker->hover_date;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_date_range_picker_base_clear.
 * @param picker Parameter picker.
 * @return Return value.
 */
ui_error_t
ui_date_range_picker_base_clear(struct ui_date_range_picker_base *picker) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->state = UI_DATE_RANGE_PICKER_STATE_IDLE;
  memset(&picker->start_date, 0, sizeof(struct ui_date));
  memset(&picker->end_date, 0, sizeof(struct ui_date));
  memset(&picker->hover_date, 0, sizeof(struct ui_date));
  return UI_ERROR_NONE;
}
