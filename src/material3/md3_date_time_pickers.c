/**
 * @file md3_date_time_pickers.c
 * @brief Implementation of Material 3 Date and Time Pickers.
 */

/* clang-format off */
#include "material3/md3_date_time_pickers.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include "ui_input_base.h"
#include "ui_date_range_picker_base.h"
#include "ui_popover_base.h"
#include <string.h>
/* clang-format on */

struct md3_date_picker {
  struct ui_component *component;
  struct ui_datepicker_base *base;
  struct ui_input_base *input;
  struct ui_popover_base *popover;
  struct ui_calendar_base *calendar;
  struct ui_control_value_accessor cva;
  struct ui_control_value_accessor calendar_cva;
};

struct md3_date_range_picker {
  struct ui_component *component;
  struct ui_date_range_picker_base *base;
  struct ui_control_value_accessor cva;
  struct ui_date_range range;
  struct ui_date hover_date;
  int is_dual_month;
};

struct md3_time_picker {
  struct ui_component *component;
  struct ui_timepicker_base *base;
  enum md3_time_picker_mode mode;
  enum ui_timepicker_format format;
  struct ui_control_value_accessor cva;
};

/* -------------------------------------------------------------------------
 * MD3 Date Picker
 * ------------------------------------------------------------------------- */

ui_error_t md3_date_picker_create(struct ui_engine *engine,
                                  struct md3_date_picker **out_picker) {
  struct md3_date_picker *picker;
  ui_error_t rc;

  if (!engine || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct md3_date_picker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_date_picker));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(picker, 0, sizeof(*picker));

  rc = ui_component_create(&picker->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  /* Mock dependencies for datepicker_base */
  rc = ui_input_base_create(&picker->input);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(picker->component);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_popover_base_create(&picker->popover);
  if (rc != UI_ERROR_NONE) {
    ui_input_base_destroy(picker->input);
    ui_component_destroy(picker->component);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_calendar_base_create(&picker->calendar, &picker->calendar_cva);
  if (rc != UI_ERROR_NONE) {
    ui_popover_base_destroy(picker->popover);
    ui_input_base_destroy(picker->input);
    ui_component_destroy(picker->component);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_datepicker_base_create(&picker->base, picker->input, picker->popover,
                                 picker->calendar, &picker->cva);
  if (rc != UI_ERROR_NONE) {
    ui_calendar_base_destroy(picker->calendar);
    ui_popover_base_destroy(picker->popover);
    ui_input_base_destroy(picker->input);
    ui_component_destroy(picker->component);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t md3_date_picker_destroy(struct md3_date_picker *picker) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t temp_rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->base) {
    temp_rc = ui_datepicker_base_destroy(picker->base);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }
  if (picker->calendar) {
    temp_rc = ui_calendar_base_destroy(picker->calendar);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }
  if (picker->popover) {
    temp_rc = ui_popover_base_destroy(picker->popover);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }
  if (picker->input) {
    temp_rc = ui_input_base_destroy(picker->input);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }
  if (picker->component) {
    temp_rc = ui_component_destroy(picker->component);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

/* -------------------------------------------------------------------------
 * MD3 Date Range Picker
 * ------------------------------------------------------------------------- */

ui_error_t
md3_date_range_picker_create(struct ui_engine *engine,
                             struct md3_date_range_picker **out_picker) {
  struct md3_date_range_picker *picker;
  ui_error_t rc;

  if (!engine || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct md3_date_range_picker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_date_range_picker));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(picker, 0, sizeof(*picker));

  rc = ui_component_create(&picker->component);
  picker->is_dual_month = 1;
  if (rc == UI_ERROR_NONE) {
    rc = ui_date_range_picker_base_create(&picker->base);
  }
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t md3_date_range_picker_destroy(struct md3_date_range_picker *picker) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t temp_rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->component) {
    temp_rc = ui_component_destroy(picker->component);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

/* -------------------------------------------------------------------------
 * MD3 Time Picker
 * ------------------------------------------------------------------------- */

ui_error_t md3_time_picker_create(struct ui_engine *engine,
                                  enum md3_time_picker_mode mode,
                                  enum ui_timepicker_format format,
                                  struct md3_time_picker **out_picker) {
  struct md3_time_picker *picker;
  ui_error_t rc;
  (void)format;

  if (!engine || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct md3_time_picker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_time_picker));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(picker, 0, sizeof(*picker));

  picker->mode = mode;
  picker->format = format;

  rc = ui_component_create(&picker->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_timepicker_base_create(&picker->base, &picker->cva);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(picker->component);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t md3_time_picker_destroy(struct md3_time_picker *picker) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t temp_rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (picker->base) {
    temp_rc = ui_timepicker_base_destroy(picker->base);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  if (picker->component) {
    temp_rc = ui_component_destroy(picker->component);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

ui_error_t
md3_date_range_picker_select_date(struct md3_date_range_picker *picker,
                                  const struct ui_date *date) {
  ui_error_t rc;

  if (!picker || !date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_date_range_picker_base_select_date(picker->base, date);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_date_range_picker_base_get_range(picker->base, &picker->range);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_date_range_picker_set_hover_date(struct md3_date_range_picker *picker,
                                     const struct ui_date *date) {
  ui_error_t rc;

  if (!picker || !date) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_date_range_picker_base_set_hover_date(picker->base, date);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  picker->hover_date = *date;
  return UI_ERROR_NONE;
}

ui_error_t
md3_date_range_picker_get_range(const struct md3_date_range_picker *picker,
                                struct ui_date_range *out_range) {
  if (!picker || !out_range) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_range = picker->range;
  return UI_ERROR_NONE;
}

ui_error_t md3_date_range_picker_clear(struct md3_date_range_picker *picker) {
  ui_error_t rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_date_range_picker_base_clear(picker->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  memset(&picker->range, 0, sizeof(picker->range));
  return UI_ERROR_NONE;
}
