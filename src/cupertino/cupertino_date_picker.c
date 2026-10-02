/**
 * @file cupertino_date_picker.c
 * @brief Cupertino Date and Time Pickers implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_date_picker.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static const char *s_months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static void update_formatted_text(struct cupertino_date_picker *picker) {
  int m_idx;
  int h12;
  const char *ampm;

  m_idx = picker->month - 1;
  if (m_idx < 0) {
    m_idx = 0;
  } else if (m_idx > 11) {
    m_idx = 11;
  }

  if (picker->mode == CUPERTINO_DATE_PICKER_MODE_TIME) {
    if (picker->is_24h) {
#if defined(_MSC_VER)
      sprintf_s(picker->formatted_text, sizeof(picker->formatted_text),
                "%02d:%02d", picker->hour, picker->minute);
#else
      sprintf(picker->formatted_text, "%02d:%02d", picker->hour,
              picker->minute);
#endif
    } else {
      ampm = (picker->hour >= 12) ? "PM" : "AM";
      h12 = picker->hour % 12;
      if (h12 == 0) {
        h12 = 12;
      }
#if defined(_MSC_VER)
      sprintf_s(picker->formatted_text, sizeof(picker->formatted_text),
                "%d:%02d %s", h12, picker->minute, ampm);
#else
      sprintf(picker->formatted_text, "%d:%02d %s", h12, picker->minute, ampm);
#endif
    }
  } else if (picker->mode == CUPERTINO_DATE_PICKER_MODE_DATE) {
#if defined(_MSC_VER)
    sprintf_s(picker->formatted_text, sizeof(picker->formatted_text),
              "%s %d, %d", s_months[m_idx], picker->day, picker->year);
#else
    sprintf(picker->formatted_text, "%s %d, %d", s_months[m_idx], picker->day,
            picker->year);
#endif
  } else if (picker->mode == CUPERTINO_DATE_PICKER_MODE_DATE_AND_TIME) {
    ampm = (picker->hour >= 12) ? "PM" : "AM";
    h12 = picker->hour % 12;
    if (h12 == 0) {
      h12 = 12;
    }
#if defined(_MSC_VER)
    sprintf_s(picker->formatted_text, sizeof(picker->formatted_text),
              "%s %d, %d at %d:%02d %s", s_months[m_idx], picker->day,
              picker->year, h12, picker->minute, ampm);
#else
    sprintf(picker->formatted_text, "%s %d, %d at %d:%02d %s", s_months[m_idx],
            picker->day, picker->year, h12, picker->minute, ampm);
#endif
  } else {
#if defined(_MSC_VER)
    sprintf_s(picker->formatted_text, sizeof(picker->formatted_text),
              "%d hours %d min", picker->hour, picker->minute);
#else
    sprintf(picker->formatted_text, "%d hours %d min", picker->hour,
            picker->minute);
#endif
  }
}

static void update_dimensions(struct cupertino_date_picker *picker) {
  if (picker->style == CUPERTINO_DATE_PICKER_STYLE_COMPACT) {
    picker->width = CUPERTINO_DATE_PICKER_COMPACT_WIDTH;
    picker->height = CUPERTINO_DATE_PICKER_COMPACT_HEIGHT;
  } else if (picker->style == CUPERTINO_DATE_PICKER_STYLE_INLINE) {
    picker->width = CUPERTINO_DATE_PICKER_INLINE_WIDTH;
    picker->height = CUPERTINO_DATE_PICKER_INLINE_HEIGHT;
  } else {
    picker->width = CUPERTINO_DATE_PICKER_WHEELS_WIDTH;
    picker->height = CUPERTINO_DATE_PICKER_WHEELS_HEIGHT;
  }
}

static ui_error_t date_picker_cva_write_value(void *component,
                                              union ui_signal_payload value) {
  struct cupertino_date_picker *picker =
      (struct cupertino_date_picker *)component;
  const int *ints;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value.ptr_val) {
    ints = (const int *)value.ptr_val;
    picker->year = ints[0];
    picker->month = ints[1];
    picker->day = ints[2];
    picker->hour = ints[3];
    picker->minute = ints[4];
  }

  update_formatted_text(picker);
  return UI_ERROR_NONE;
}

static ui_error_t date_picker_cva_set_disabled(void *component,
                                               ui_bool_t is_disabled) {
  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (is_disabled < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

ui_error_t cupertino_date_picker_create(
    struct ui_engine *engine,
    const struct cupertino_date_picker_descriptor *desc,
    struct cupertino_date_picker **out_picker) {
  struct cupertino_date_picker *picker;

  if (!engine || !desc || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->mode < 0 || (int)desc->mode > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->style < 0 || (int)desc->style > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->year < 1900 || desc->year > 2100) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->month < 1 || desc->month > 12) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->day < 1 || desc->day > 31) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->hour < 0 || desc->hour > 23) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (desc->minute < 0 || desc->minute > 59) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct cupertino_date_picker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_date_picker));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(picker, 0, sizeof(*picker));
  picker->mode = desc->mode;
  picker->style = desc->style;
  picker->year = desc->year;
  picker->month = desc->month;
  picker->day = desc->day;
  picker->hour = desc->hour;
  picker->minute = desc->minute;
  picker->is_24h = desc->is_24h ? 1 : 0;
  picker->minute_interval =
      (desc->minute_interval > 0) ? desc->minute_interval : 1;
  picker->is_popover_open = 0;

  update_formatted_text(picker);
  update_dimensions(picker);

  /* Set up CVA */
  picker->cva.component = picker;
  picker->cva.write_value = date_picker_cva_write_value;
  picker->cva.set_disabled_state = date_picker_cva_set_disabled;

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_date_picker_destroy(struct cupertino_date_picker *picker) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(picker);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_date_picker_set_date(struct cupertino_date_picker *picker,
                                          int year, int month, int day,
                                          int hour, int minute) {
  if (!picker || year < 1900 || year > 2100 || month < 1 || month > 12 ||
      day < 1 || day > 31 || hour < 0 || hour > 23 || minute < 0 ||
      minute > 59) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->year = year;
  picker->month = month;
  picker->day = day;
  picker->hour = hour;
  picker->minute = minute;

  update_formatted_text(picker);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_get_date(const struct cupertino_date_picker *picker,
                               int *out_year, int *out_month, int *out_day,
                               int *out_hour, int *out_minute) {
  if (!picker || !out_year || !out_month || !out_day || !out_hour ||
      !out_minute) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_year = picker->year;
  *out_month = picker->month;
  *out_day = picker->day;
  *out_hour = picker->hour;
  *out_minute = picker->minute;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_set_mode(struct cupertino_date_picker *picker,
                               enum cupertino_date_picker_mode mode) {
  if (!picker || (int)mode < 0 || (int)mode > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->mode = mode;
  update_formatted_text(picker);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_get_mode(const struct cupertino_date_picker *picker,
                               enum cupertino_date_picker_mode *out_mode) {
  if (!picker || !out_mode) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_mode = picker->mode;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_set_style(struct cupertino_date_picker *picker,
                                enum cupertino_date_picker_style style) {
  if (!picker || (int)style < 0 || (int)style > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->style = style;
  update_dimensions(picker);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_get_style(const struct cupertino_date_picker *picker,
                                enum cupertino_date_picker_style *out_style) {
  if (!picker || !out_style) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_style = picker->style;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_date_picker_get_formatted_text(
    const struct cupertino_date_picker *picker, const char **out_text) {
  if (!picker || !out_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_text = picker->formatted_text;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_set_popover_open(struct cupertino_date_picker *picker,
                                       int is_open) {
  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker->is_popover_open = is_open ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_get_dimensions(const struct cupertino_date_picker *picker,
                                     float *out_width, float *out_height) {
  if (!picker || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = picker->width;
  *out_height = picker->height;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_date_picker_get_cva(struct cupertino_date_picker *picker,
                              struct ui_control_value_accessor **out_cva) {
  if (!picker || !out_cva) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_cva = &picker->cva;
  return UI_ERROR_NONE;
}
