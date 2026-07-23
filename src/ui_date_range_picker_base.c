/* clang-format off */
#include "ui_date_range_picker_base.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

struct ui_date_range_picker_base {
  struct ui_component *component;
  enum ui_date_range_picker_state state;
  struct ui_date start_date;
  struct ui_date end_date;
  struct ui_date hover_date;

  ui_date_predicate_cb predicate_cb;
  void *predicate_user_data;

  ui_date_range_on_change_cb on_change_cb;
  void *on_change_user_data;
};

enum ui_error ui_date_compare(const struct ui_date *a, const struct ui_date *b,
                              int *out_result) {
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

enum ui_error ui_date_is_valid(const struct ui_date *date,
                               ui_bool_t *out_is_valid) {
  int days;

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

  ui_calendar_days_in_month(date->year, date->month, &days);

  if (date->day <= days) {
    *out_is_valid = UI_TRUE;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_date_range_picker_base_create */
enum ui_error ui_date_range_picker_base_create(
    struct ui_date_range_picker_base **out_picker) {
  struct ui_date_range_picker_base *picker;
  enum ui_error rc;

  if (!out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct ui_date_range_picker_base *)UI_MALLOC(
      sizeof(struct ui_date_range_picker_base));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(picker, 0, sizeof(struct ui_date_range_picker_base));

  rc = ui_component_create(&picker->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(picker);
    return rc;
  }

  picker->state = UI_DATE_RANGE_PICKER_STATE_IDLE;

  *out_picker = picker;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_date_range_picker_base_destroy(struct ui_date_range_picker_base *picker) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  ui_component_destroy(picker->component);
  UI_FREE(picker);
  return UI_ERROR_NONE;
}

/** \brief ui_date_range_picker_base_set_disable_predicate */
enum ui_error ui_date_range_picker_base_set_disable_predicate(
    struct ui_date_range_picker_base *picker, ui_date_predicate_cb predicate,
    void *user_data) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  picker->predicate_cb = predicate;
  picker->predicate_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief ui_date_range_picker_base_set_on_change */
enum ui_error ui_date_range_picker_base_set_on_change(
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
static enum ui_error
check_range_validity(struct ui_date_range_picker_base *picker,
                     const struct ui_date *start, const struct ui_date *end,
                     ui_bool_t *out_valid) {
  struct ui_date current;
  int cmp;
  int days;

  *out_valid = UI_TRUE;
  if (!picker->predicate_cb) {
    return UI_ERROR_NONE;
  }

  current = *start;
  while (1) {
    ui_date_compare(&current, end, &cmp);
    if (cmp > 0) {
      break;
    }

    if (picker->predicate_cb(&current, picker->predicate_user_data)) {
      *out_valid = UI_FALSE;
      break;
    }

    /* Advance current by 1 day */
    ui_calendar_days_in_month(current.year, current.month, &days);
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

/** \brief ui_error */
enum ui_error
ui_date_range_picker_base_select_date(struct ui_date_range_picker_base *picker,
                                      const struct ui_date *date) {
  ui_bool_t is_valid, range_valid;

  int cmp;

  if (!picker || !date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_date_is_valid(date, &is_valid);
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
    ui_date_compare(date, &picker->start_date, &cmp);

    if (cmp < 0) {
      /* Date selected is before start_date, restart selection */
      picker->start_date = *date;
      picker->hover_date = *date;
    } else {
      /* Validate the whole range doesn't contain disabled dates */
      check_range_validity(picker, &picker->start_date, date, &range_valid);

      if (range_valid) {
        struct ui_date_range range;
        picker->end_date = *date;
        picker->state = UI_DATE_RANGE_PICKER_STATE_IDLE;

        if (picker->on_change_cb) {
          range.start_date = picker->start_date;
          range.end_date = picker->end_date;
          picker->on_change_cb(picker, &range, picker->on_change_user_data);
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

/** \brief ui_date_range_picker_base_set_hover_date */
enum ui_error ui_date_range_picker_base_set_hover_date(
    struct ui_date_range_picker_base *picker, const struct ui_date *date) {
  ui_bool_t is_valid;

  if (!picker || !date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_date_is_valid(date, &is_valid);
  if (!is_valid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->state == UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE) {
    picker->hover_date = *date;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_date_range_picker_base_get_state */
enum ui_error ui_date_range_picker_base_get_state(
    const struct ui_date_range_picker_base *picker,
    enum ui_date_range_picker_state *out_state) {
  if (!picker || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_state = picker->state;
  return UI_ERROR_NONE;
}

/** \brief ui_date_range_picker_base_get_range */
enum ui_error ui_date_range_picker_base_get_range(
    const struct ui_date_range_picker_base *picker,
    struct ui_date_range *out_range) {
  int cmp;

  if (!picker || !out_range) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(out_range, 0, sizeof(struct ui_date_range));

  if (picker->state == UI_DATE_RANGE_PICKER_STATE_IDLE) {
    out_range->start_date = picker->start_date;
    out_range->end_date = picker->end_date;
  } else {
    out_range->start_date = picker->start_date;
    ui_date_compare(&picker->hover_date, &picker->start_date, &cmp);
    if (cmp < 0) {
      out_range->end_date = picker->start_date;
    } else {
      out_range->end_date = picker->hover_date;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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
