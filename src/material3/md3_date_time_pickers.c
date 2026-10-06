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

#ifdef UI_TEST_MOCK_ALLOC
int g_md3_pickers_mock_fail = 0;

static ui_error_t mock_datepicker_base_destroy(struct ui_datepicker_base *p) {
  if (g_md3_pickers_mock_fail == 1) {
    (ui_datepicker_base_destroy)(p);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_datepicker_base_destroy)(p);
}
#undef ui_datepicker_base_destroy
/** @cond */
#define ui_datepicker_base_destroy mock_datepicker_base_destroy
/** @endcond */

static ui_error_t mock_calendar_base_destroy(struct ui_calendar_base *p) {
  if (g_md3_pickers_mock_fail == 2) {
    (ui_calendar_base_destroy)(p);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_calendar_base_destroy)(p);
}
#undef ui_calendar_base_destroy
/** @cond */
#define ui_calendar_base_destroy mock_calendar_base_destroy
/** @endcond */

static ui_error_t mock_popover_base_destroy(struct ui_popover_base *p) {
  if (g_md3_pickers_mock_fail == 3) {
    (ui_popover_base_destroy)(p);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_popover_base_destroy)(p);
}
#undef ui_popover_base_destroy
/** @cond */
#define ui_popover_base_destroy mock_popover_base_destroy
/** @endcond */

static ui_error_t mock_input_base_destroy(struct ui_input_base *p) {
  if (g_md3_pickers_mock_fail == 4) {
    (ui_input_base_destroy)(p);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_input_base_destroy)(p);
}
#undef ui_input_base_destroy
/** @cond */
#define ui_input_base_destroy mock_input_base_destroy
/** @endcond */

static ui_error_t mock_component_destroy(struct ui_component *p) {
  if (g_md3_pickers_mock_fail == 5) {
    (ui_component_destroy)(p);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(p);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_component_destroy
/** @endcond */

static ui_error_t
mock_date_range_picker_base_destroy(struct ui_date_range_picker_base *p) {
  if (g_md3_pickers_mock_fail == 6) {
    (ui_date_range_picker_base_destroy)(p);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_date_range_picker_base_destroy)(p);
}
#undef ui_date_range_picker_base_destroy
/** @cond */
#define ui_date_range_picker_base_destroy mock_date_range_picker_base_destroy
/** @endcond */

static ui_error_t mock_timepicker_base_destroy(struct ui_timepicker_base *p) {
  if (g_md3_pickers_mock_fail == 7) {
    (ui_timepicker_base_destroy)(p);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_timepicker_base_destroy)(p);
}
#undef ui_timepicker_base_destroy
/** @cond */
#define ui_timepicker_base_destroy mock_timepicker_base_destroy
/** @endcond */

extern ui_error_t
ui_date_range_picker_base_select_date(struct ui_date_range_picker_base *,
                                      const struct ui_date *);
static ui_error_t
mock_date_range_picker_base_select_date(struct ui_date_range_picker_base *p,
                                        const struct ui_date *d) {
  if (g_md3_pickers_mock_fail == 8)
    return UI_ERROR_UNKNOWN;
  return (ui_date_range_picker_base_select_date)(p, d);
}
#undef ui_date_range_picker_base_select_date
/** @cond */
#define ui_date_range_picker_base_select_date                                  \
  mock_date_range_picker_base_select_date
/** @endcond */

extern ui_error_t
ui_date_range_picker_base_get_range(const struct ui_date_range_picker_base *,
                                    struct ui_date_range *);
static ui_error_t
mock_date_range_picker_base_get_range(const struct ui_date_range_picker_base *p,
                                      struct ui_date_range *r) {
  if (g_md3_pickers_mock_fail == 9)
    return UI_ERROR_UNKNOWN;
  return (ui_date_range_picker_base_get_range)(p, r);
}
#undef ui_date_range_picker_base_get_range
/** @cond */
#define ui_date_range_picker_base_get_range                                    \
  mock_date_range_picker_base_get_range
/** @endcond */

extern ui_error_t
ui_date_range_picker_base_set_hover_date(struct ui_date_range_picker_base *,
                                         const struct ui_date *);
static ui_error_t
mock_date_range_picker_base_set_hover_date(struct ui_date_range_picker_base *p,
                                           const struct ui_date *d) {
  if (g_md3_pickers_mock_fail == 10)
    return UI_ERROR_UNKNOWN;
  return (ui_date_range_picker_base_set_hover_date)(p, d);
}
#undef ui_date_range_picker_base_set_hover_date
/** @cond */
#define ui_date_range_picker_base_set_hover_date                               \
  mock_date_range_picker_base_set_hover_date
/** @endcond */

extern ui_error_t
ui_date_range_picker_base_clear(struct ui_date_range_picker_base *);
static ui_error_t
mock_date_range_picker_base_clear(struct ui_date_range_picker_base *p) {
  if (g_md3_pickers_mock_fail == 11)
    return UI_ERROR_UNKNOWN;
  return (ui_date_range_picker_base_clear)(p);
}
#undef ui_date_range_picker_base_clear
/** @cond */
#define ui_date_range_picker_base_clear mock_date_range_picker_base_clear
/** @endcond */

#endif

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
    md3_date_picker_destroy(picker);
    return rc;
  }

  /* Mock dependencies for datepicker_base */
  rc = ui_input_base_create(&picker->input);
  if (rc != UI_ERROR_NONE) {
    md3_date_picker_destroy(picker);
    return rc;
  }

  rc = ui_popover_base_create(&picker->popover);
  if (rc != UI_ERROR_NONE) {
    md3_date_picker_destroy(picker);
    return rc;
  }

  rc = ui_calendar_base_create(&picker->calendar, &picker->calendar_cva);
  if (rc != UI_ERROR_NONE) {
    md3_date_picker_destroy(picker);
    return rc;
  }

  rc = ui_datepicker_base_create(&picker->base, picker->input, picker->popover,
                                 picker->calendar, &picker->cva);
  if (rc != UI_ERROR_NONE) {
    md3_date_picker_destroy(picker);
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
    md3_date_range_picker_destroy(picker);
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

  if (picker->base) {
    temp_rc = ui_date_range_picker_base_destroy(picker->base);
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
    md3_time_picker_destroy(picker);
    return rc;
  }

  rc = ui_timepicker_base_create(&picker->base, &picker->cva);
  if (rc != UI_ERROR_NONE) {
    md3_time_picker_destroy(picker);
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
