/* clang-format off */
#include "ui_datepicker_base.h"
#include "ui_internal_mem.h"
#include "ui_input_base.h"
#include "ui_popover_base.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

struct ui_datepicker_base {
  struct ui_input_base *input;
  struct ui_popover_base *popover;
  struct ui_calendar_base *calendar;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
  int is_syncing;
};

static ui_error_t trigger_cva_change(struct ui_datepicker_base *dp,
                                     const struct ui_date *date);

static ui_error_t on_calendar_select(struct ui_calendar_base *calendar,
                                     const struct ui_date *date,
                                     void *user_data) {
  struct ui_datepicker_base *datepicker =
      (struct ui_datepicker_base *)user_data;
  char text[32];
  ui_error_t rc;
  (void)calendar; /* unused */

  if (datepicker->is_syncing)
    return UI_ERROR_NONE;
  datepicker->is_syncing = 1;

  (void)ui_datepicker_format_date(date, text, sizeof(text));

  rc = ui_input_base_set_text(datepicker->input, text);
  if (rc != UI_ERROR_NONE) {
    datepicker->is_syncing = 0;
    return rc;
  }

  /* Close popover after selection */
  (void)ui_popover_base_close(datepicker->popover);

  rc = trigger_cva_change(datepicker, date);
  datepicker->is_syncing = 0;
  return rc;
}

static ui_error_t on_input_change(struct ui_input_base *input, const char *text,
                                  void *user_data) {
  struct ui_datepicker_base *datepicker =
      (struct ui_datepicker_base *)user_data;
  struct ui_date parsed_date;
  ui_error_t rc;
  if (datepicker->is_syncing)
    return UI_ERROR_NONE;
  datepicker->is_syncing = 1;
  (void)input;

  if (text) {
    ui_error_t parse_rc = ui_datepicker_parse_date(text, &parsed_date);
    if (parse_rc != UI_ERROR_NONE) {
      if (0)
        return parse_rc;
    }
    if (parse_rc == UI_ERROR_NONE) {
      rc = ui_calendar_base_select_date(datepicker->calendar, &parsed_date);
      if (rc != UI_ERROR_NONE) {
        datepicker->is_syncing = 0;
        return rc;
      }
      /* `ui_calendar_base_select_date` will trigger `on_calendar_select` which
         updates the CVA. However, because we are the coordinator, we can just
         let that propagate or directly trigger it. We'll directly trigger it
         here to ensure signal gets sent even if calendar was already at that
         date. */
      rc = trigger_cva_change(datepicker, &parsed_date);
      datepicker->is_syncing = 0;
      return rc;
    }
  }

  (void)ui_calendar_base_clear_selection(datepicker->calendar);
  rc = trigger_cva_change(datepicker, NULL);
  datepicker->is_syncing = 0;
  return rc;
}

static ui_error_t trigger_cva_change(struct ui_datepicker_base *dp,
                                     const struct ui_date *date) {
  union ui_signal_payload payload;
  /* Since memory ownership is complex for pointers across scopes,
     and ui_date can easily fit in an INT32 using bit-packing or
     we can just pass a pointer to a temporarily tracked/internal copy.
     For this application, we'll pack it into INT32 since Y/M/D is small.
     Year (12 bits), Month (4 bits), Day (5 bits) = 21 bits total. */
  if (dp->cva_on_change) {
    if (date) {
      payload.int_val = (date->year << 9) | (date->month << 5) | date->day;
    } else {
      payload.int_val = 0; /* 0 means no date selected */
    }
    return dp->cva_on_change(payload, dp->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t datepicker_cva_write_value(void *component,
                                             union ui_signal_payload value) {
  struct ui_datepicker_base *dp = (struct ui_datepicker_base *)component;
  struct ui_date date;
  char text[32];
  ui_error_t rc;

  if (!dp)
    return UI_ERROR_INVALID_ARGUMENT;
  if (dp->is_syncing)
    return UI_ERROR_NONE;
  dp->is_syncing = 1;

  if (value.int_val == 0) {
    rc = ui_input_base_set_text(dp->input, "");
    if (rc != UI_ERROR_NONE) {
      dp->is_syncing = 0;
      return rc;
    }
    (void)ui_calendar_base_clear_selection(dp->calendar);
    dp->is_syncing = 0;
    return UI_ERROR_NONE;
  } else {
    date.year = (value.int_val >> 9) & 0xFFF;
    date.month = (value.int_val >> 5) & 0xF;
    date.day = value.int_val & 0x1F;

    (void)ui_datepicker_format_date(&date, text, sizeof(text));

    rc = ui_input_base_set_text(dp->input, text);
    if (rc != UI_ERROR_NONE) {
      dp->is_syncing = 0;
      return rc;
    }
    rc = ui_calendar_base_select_date(dp->calendar, &date);
    dp->is_syncing = 0;
    return rc;
  }
}

/** \brief datepicker_cva_register_on_change */
static ui_error_t datepicker_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_datepicker_base *dp = (struct ui_datepicker_base *)component;
  if (!dp)
    return UI_ERROR_INVALID_ARGUMENT;
  if (dp->is_syncing)
    return UI_ERROR_NONE;
  dp->is_syncing = 1;
  dp->cva_on_change = callback;
  dp->cva_on_change_user_data = user_data;
  dp->is_syncing = 0;
  return UI_ERROR_NONE;
}

static ui_error_t datepicker_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_datepicker_base *dp = (struct ui_datepicker_base *)component;
  if (!dp)
    return UI_ERROR_INVALID_ARGUMENT;
  if (dp->is_syncing)
    return UI_ERROR_NONE;
  dp->is_syncing = 1;
  dp->cva_on_touched = callback;
  dp->cva_on_touched_user_data = user_data;
  dp->is_syncing = 0;
  return UI_ERROR_NONE;
}

static ui_error_t datepicker_cva_set_disabled_state(void *component,
                                                    int is_disabled) {
  struct ui_datepicker_base *dp = (struct ui_datepicker_base *)component;
  ui_error_t rc;
  if (!dp)
    return UI_ERROR_INVALID_ARGUMENT;
  if (dp->is_syncing)
    return UI_ERROR_NONE;
  dp->is_syncing = 1;
  dp->is_disabled = is_disabled;

  (void)ui_input_base_set_disabled(dp->input, is_disabled);
  dp->is_syncing = 0;
  return UI_ERROR_NONE;
}

/** \brief ui_datepicker_base_create */
ui_error_t ui_datepicker_base_create(
    struct ui_datepicker_base **out_datepicker, struct ui_input_base *input,
    struct ui_popover_base *popover, struct ui_calendar_base *calendar,
    struct ui_control_value_accessor *out_cva) {
  struct ui_datepicker_base *dp;
  ui_error_t rc;

  if (!out_datepicker || !input || !popover || !calendar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dp = (struct ui_datepicker_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_datepicker_base));
  if (!dp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  dp->input = input;
  dp->popover = popover;
  dp->calendar = calendar;
  dp->cva_on_change = NULL;
  dp->cva_on_change_user_data = NULL;
  dp->cva_on_touched = NULL;
  dp->cva_on_touched_user_data = NULL;
  dp->is_disabled = 0;
  dp->is_syncing = 0;

  (void)ui_calendar_base_set_on_select(calendar, on_calendar_select, dp);
  (void)ui_input_base_set_on_change(input, on_input_change, dp);

  if (out_cva) {
    out_cva->write_value = datepicker_cva_write_value;
    out_cva->register_on_change = datepicker_cva_register_on_change;
    out_cva->register_on_touched = datepicker_cva_register_on_touched;
    out_cva->set_disabled_state = datepicker_cva_set_disabled_state;
  }

  *out_datepicker = dp;
  return UI_ERROR_NONE;
}

ui_error_t ui_datepicker_base_destroy(struct ui_datepicker_base *datepicker) {
  if (!datepicker)
    return UI_ERROR_NONE;

  /* Unhook to prevent dangling pointer if input/calendar outlives datepicker */
  (void)ui_calendar_base_set_on_select(datepicker->calendar, NULL, NULL);
  (void)ui_input_base_set_on_change(datepicker->input, NULL, NULL);

  C_MULTIPLATFORM_FREE(datepicker);
  return UI_ERROR_NONE;
}

ui_error_t ui_datepicker_parse_date(const char *text,
                                    struct ui_date *out_date) {
  int y, m, d;
  ui_error_t rc;
  if (!text || !out_date)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER)
  if (sscanf_s(text, "%d-%d-%d", &y, &m, &d) != 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#else
  if (sscanf(text, "%d-%d-%d", &y, &m, &d) != 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  if (m < 1 || m > 12)
    return UI_ERROR_INVALID_ARGUMENT;
  {
    int days = 0;
    (void)ui_calendar_days_in_month(y, m, &days);

    if (d < 1 || d > days)
      return UI_ERROR_INVALID_ARGUMENT;
  }

  out_date->year = y;
  out_date->month = m;
  out_date->day = d;

  return UI_ERROR_NONE;
}

ui_error_t ui_datepicker_format_date(const struct ui_date *date, char *out_text,
                                     int max_len) {
  if (!date || !out_text || max_len < 11)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER)
  sprintf_s(out_text, max_len, "%04d-%02d-%02d", date->year, date->month,
            date->day);
#else
  sprintf(out_text, "%04d-%02d-%02d", date->year, date->month, date->day);
#endif

  return UI_ERROR_NONE;
}

ui_error_t ui_datepicker_base_sync(struct ui_datepicker_base *datepicker) {
  const char *text = NULL;
  ui_error_t rc;
  if (!datepicker)
    return UI_ERROR_INVALID_ARGUMENT;

  (void)ui_input_base_get_text(datepicker->input, &text);
  rc = on_input_change(datepicker->input, text, datepicker);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}
