/* clang-format off */
#include "ui_timepicker_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER)
#define NUM_FORMAT "%02d:%02d"
#else
#define NUM_FORMAT "%02d:%02d"
#endif

/** \brief ui_timepicker_base */
struct ui_timepicker_base {
  int hour;
  int minute;
  enum ui_timepicker_format format;
  int disabled;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;

  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;
};

static ui_error_t trigger_cva_change(struct ui_timepicker_base *tp) {
  if (tp && tp->cva_on_change) {
    union ui_signal_payload payload;
    /* Payload is seconds since midnight */
    payload.int_val = (tp->hour * 3600) + (tp->minute * 60);
    return tp->cva_on_change(payload, tp->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t trigger_cva_touched(struct ui_timepicker_base *tp) {
  if (tp && tp->cva_on_touched) {
    return tp->cva_on_touched(tp->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t timepicker_cva_write_value(void *component,
                                             union ui_signal_payload value) {
  struct ui_timepicker_base *tp = (struct ui_timepicker_base *)component;
  int seconds;
  int h, m;

  if (!tp)
    return UI_ERROR_INVALID_ARGUMENT;

  seconds = value.int_val;
  if (seconds < 0)
    seconds = 0;

  h = (seconds / 3600) % 24;
  m = (seconds / 60) % 60;

  return ui_timepicker_base_set_time(tp, h, m);
}

/** \brief timepicker_cva_register_on_change */
static ui_error_t timepicker_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_timepicker_base *tp = (struct ui_timepicker_base *)component;
  if (!tp)
    return UI_ERROR_INVALID_ARGUMENT;
  tp->cva_on_change = callback;
  tp->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t timepicker_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_timepicker_base *tp = (struct ui_timepicker_base *)component;
  if (!tp)
    return UI_ERROR_INVALID_ARGUMENT;
  tp->cva_on_touched = callback;
  tp->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t timepicker_cva_set_disabled_state(void *component,
                                                    int is_disabled) {
  struct ui_timepicker_base *tp = (struct ui_timepicker_base *)component;
  if (!tp)
    return UI_ERROR_INVALID_ARGUMENT;
  tp->disabled = is_disabled;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_timepicker_base_create(struct ui_timepicker_base **out_timepicker,
                          struct ui_control_value_accessor *out_cva) {
  struct ui_timepicker_base *tp;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_timepicker) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  tp = (struct ui_timepicker_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_timepicker_base));
  if (!tp) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  tp->hour = 0;
  tp->minute = 0;
  tp->format = UI_TIMEPICKER_FORMAT_24H;
  tp->disabled = 0;
  tp->cva_on_change = NULL;
  tp->cva_on_change_user_data = NULL;
  tp->cva_on_touched = NULL;
  tp->cva_on_touched_user_data = NULL;

  if (out_cva) {
    out_cva->write_value = timepicker_cva_write_value;
    out_cva->register_on_change = timepicker_cva_register_on_change;
    out_cva->register_on_touched = timepicker_cva_register_on_touched;
    out_cva->set_disabled_state = timepicker_cva_set_disabled_state;
  }

  *out_timepicker = tp;

cleanup:
  return rc;
}

ui_error_t ui_timepicker_base_destroy(struct ui_timepicker_base *timepicker) {
  if (!timepicker) {
    return UI_ERROR_NONE;
  }
  C_MULTIPLATFORM_FREE(timepicker);
  return UI_ERROR_NONE;
}

ui_error_t ui_timepicker_base_set_time(struct ui_timepicker_base *timepicker,
                                       int hour, int minute) {
  if (!timepicker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (hour < 0)
    hour = 0;
  if (hour > 23)
    hour = 23;
  if (minute < 0)
    minute = 0;
  if (minute > 59)
    minute = 59;

  if (timepicker->hour != hour || timepicker->minute != minute) {
    ui_error_t rc;
    timepicker->hour = hour;
    timepicker->minute = minute;
    rc = trigger_cva_touched(timepicker);
    if (rc != UI_ERROR_NONE)
      return rc;
    rc = trigger_cva_change(timepicker);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_timepicker_base_get_time(const struct ui_timepicker_base *timepicker,
                            int *out_hour, int *out_minute) {
  if (!timepicker || !out_hour || !out_minute) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_hour = timepicker->hour;
  *out_minute = timepicker->minute;

  return UI_ERROR_NONE;
}

/** \brief ui_timepicker_base_get_formatted_time */
ui_error_t ui_timepicker_base_get_formatted_time(
    const struct ui_timepicker_base *timepicker, int *out_hour, int *out_minute,
    enum ui_timepicker_period *out_period) {
  if (!timepicker || !out_hour || !out_minute || !out_period) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_minute = timepicker->minute;

  if (timepicker->format == UI_TIMEPICKER_FORMAT_12H) {
    if (timepicker->hour == 0) {
      *out_hour = 12;
      *out_period = UI_TIMEPICKER_PERIOD_AM;
    } else if (timepicker->hour == 12) {
      *out_hour = 12;
      *out_period = UI_TIMEPICKER_PERIOD_PM;
    } else if (timepicker->hour > 12) {
      *out_hour = timepicker->hour - 12;
      *out_period = UI_TIMEPICKER_PERIOD_PM;
    } else {
      *out_hour = timepicker->hour;
      *out_period = UI_TIMEPICKER_PERIOD_AM;
    }
  } else {
    *out_hour = timepicker->hour;
    *out_period = UI_TIMEPICKER_PERIOD_AM; /* default ignored */
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_timepicker_base_set_format(struct ui_timepicker_base *timepicker,
                                         enum ui_timepicker_format format) {
  if (!timepicker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  timepicker->format = format;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_timepicker_base_get_format(const struct ui_timepicker_base *timepicker,
                              enum ui_timepicker_format *out_format) {
  if (!timepicker || !out_format) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_format = timepicker->format;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_timepicker_base_get_time_string(const struct ui_timepicker_base *timepicker,
                                   char **out_string) {
  int h, m;
  enum ui_timepicker_period p;
  char *buf = NULL;
  size_t required_size;

  ui_error_t rc;

  if (!timepicker || !out_string) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_timepicker_base_get_formatted_time(timepicker, &h, &m, &p);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* We allocate 32 bytes to prevent GCC format-overflow warnings on int fields
   */
  required_size = 32;
  buf = (char *)C_MULTIPLATFORM_MALLOC(required_size);
  if (!buf) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  if (timepicker->format == UI_TIMEPICKER_FORMAT_12H) {
    sprintf_s(buf, required_size, NUM_FORMAT " %s", h, m,
              p == UI_TIMEPICKER_PERIOD_AM ? "AM" : "PM");
  } else {
    sprintf_s(buf, required_size, NUM_FORMAT, h, m);
  }
#else
  if (timepicker->format == UI_TIMEPICKER_FORMAT_12H) {
    sprintf(buf, NUM_FORMAT " %s", h, m,
            p == UI_TIMEPICKER_PERIOD_AM ? "AM" : "PM");
  } else {
    sprintf(buf, NUM_FORMAT, h, m);
  }
#endif

  *out_string = buf;

  return UI_ERROR_NONE;
}
