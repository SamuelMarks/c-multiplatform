/**
 * @file md3_overlay.c
 * @brief Material 3 Overlays, Dialogs, Sheets, Menus, Tooltips, Badges, Pickers
 * implementation.
 */

/* clang-format off */
#include "material3/md3_overlay.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/* =========================================================================
 * MD3 Dialog
 * ========================================================================= */

ui_error_t md3_dialog_create(struct ui_engine *engine,
                             enum md3_dialog_variant variant,
                             struct md3_dialog **out_dialog) {
  struct md3_dialog *dialog;
  ui_error_t rc;

  if (!engine || !out_dialog ||
      (unsigned)variant > (unsigned)MD3_DIALOG_FULL_SCREEN) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog =
      (struct md3_dialog *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_dialog));
  if (!dialog) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(dialog, 0, sizeof(struct md3_dialog));
  dialog->variant = variant;

  rc = ui_dialog_base_create(&dialog->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(dialog);
    return rc;
  }

  *out_dialog = dialog;
  return UI_ERROR_NONE;
}

ui_error_t md3_dialog_destroy(struct md3_dialog *dialog) {
  ui_error_t rc;

  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_dialog_base_destroy(dialog->base);
  C_MULTIPLATFORM_FREE(dialog);
  return rc;
}

ui_error_t md3_dialog_set_headline(struct md3_dialog *dialog,
                                   const char *headline) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (headline) {
#if defined(_MSC_VER)
    strncpy_s(dialog->headline, sizeof(dialog->headline), headline,
              sizeof(dialog->headline) - 1);
#else
    strncpy(dialog->headline, headline, sizeof(dialog->headline) - 1);
    dialog->headline[sizeof(dialog->headline) - 1] = '\0';
#endif
  } else {
    dialog->headline[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_dialog_set_supporting_text(struct md3_dialog *dialog,
                                          const char *text) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (text) {
#if defined(_MSC_VER)
    strncpy_s(dialog->supporting_text, sizeof(dialog->supporting_text), text,
              sizeof(dialog->supporting_text) - 1);
#else
    strncpy(dialog->supporting_text, text, sizeof(dialog->supporting_text) - 1);
    dialog->supporting_text[sizeof(dialog->supporting_text) - 1] = '\0';
#endif
  } else {
    dialog->supporting_text[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_dialog_set_open(struct md3_dialog *dialog, int is_open) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog->is_open = is_open ? 1 : 0;
  return ui_dialog_base_set_open(dialog->base, dialog->is_open);
}

/* =========================================================================
 * MD3 Bottom Sheet
 * ========================================================================= */

ui_error_t md3_bottom_sheet_create(struct ui_engine *engine,
                                   enum md3_bottom_sheet_variant variant,
                                   struct md3_bottom_sheet **out_sheet) {
  struct md3_bottom_sheet *sheet;
  ui_error_t rc;

  if (!engine || !out_sheet ||
      (unsigned)variant > (unsigned)MD3_BOTTOM_SHEET_FLOATING) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet = (struct md3_bottom_sheet *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_bottom_sheet));
  if (!sheet) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sheet, 0, sizeof(struct md3_bottom_sheet));
  sheet->variant = variant;

  rc = ui_bottom_sheet_base_create(&sheet->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(sheet);
    return rc;
  }

  *out_sheet = sheet;
  return UI_ERROR_NONE;
}

ui_error_t md3_bottom_sheet_destroy(struct md3_bottom_sheet *sheet) {
  ui_error_t rc;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_bottom_sheet_base_destroy(sheet->base);
  C_MULTIPLATFORM_FREE(sheet);
  return rc;
}

ui_error_t md3_bottom_sheet_set_open(struct md3_bottom_sheet *sheet,
                                     int is_open) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet->is_open = is_open ? 1 : 0;
  return ui_bottom_sheet_base_set_open(sheet->base, sheet->is_open);
}

/* =========================================================================
 * MD3 Side Sheet
 * ========================================================================= */

ui_error_t md3_side_sheet_create(struct ui_engine *engine, int is_modal,
                                 struct md3_side_sheet **out_sheet) {
  struct md3_side_sheet *sheet;
  ui_error_t rc;

  if (!engine || !out_sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet = (struct md3_side_sheet *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_side_sheet));
  if (!sheet) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(sheet, 0, sizeof(struct md3_side_sheet));
  sheet->is_modal = is_modal ? 1 : 0;

  rc = ui_side_sheet_base_create(&sheet->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(sheet);
    return rc;
  }

  *out_sheet = sheet;
  return UI_ERROR_NONE;
}

ui_error_t md3_side_sheet_destroy(struct md3_side_sheet *sheet) {
  ui_error_t rc;

  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_side_sheet_base_destroy(sheet->base);
  C_MULTIPLATFORM_FREE(sheet);
  return rc;
}

ui_error_t md3_side_sheet_set_open(struct md3_side_sheet *sheet, int is_open) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet->is_open = is_open ? 1 : 0;
  return ui_side_sheet_base_set_open(sheet->base, sheet->is_open);
}

/* =========================================================================
 * MD3 Menu
 * ========================================================================= */

ui_error_t md3_menu_create(struct ui_engine *engine,
                           struct md3_menu **out_menu) {
  struct md3_menu *menu;
  ui_error_t rc;

  if (!engine || !out_menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  menu = (struct md3_menu *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_menu));
  if (!menu) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(menu, 0, sizeof(struct md3_menu));

  rc = ui_menu_base_create(&menu->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(menu);
    return rc;
  }

  *out_menu = menu;
  return UI_ERROR_NONE;
}

ui_error_t md3_menu_destroy(struct md3_menu *menu) {
  ui_error_t rc;

  if (!menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_menu_base_destroy(menu->base);
  C_MULTIPLATFORM_FREE(menu);
  return rc;
}

/* =========================================================================
 * MD3 Tooltip
 * ========================================================================= */

ui_error_t md3_tooltip_create(struct ui_engine *engine,
                              enum md3_tooltip_variant variant,
                              const char *text,
                              struct md3_tooltip **out_tooltip) {
  struct md3_tooltip *tooltip;
  struct ui_tooltip_config cfg;
  ui_error_t rc;

  if (!engine || !out_tooltip ||
      (unsigned)variant > (unsigned)MD3_TOOLTIP_RICH) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tooltip =
      (struct md3_tooltip *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_tooltip));
  if (!tooltip) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tooltip, 0, sizeof(struct md3_tooltip));
  tooltip->variant = variant;

  if (text) {
#if defined(_MSC_VER)
    strncpy_s(tooltip->text, sizeof(tooltip->text), text,
              sizeof(tooltip->text) - 1);
#else
    strncpy(tooltip->text, text, sizeof(tooltip->text) - 1);
    tooltip->text[sizeof(tooltip->text) - 1] = '\0';
#endif
  }

  memset(&cfg, 0, sizeof(cfg));
  cfg.hover_delay_secs = 0.5;
  cfg.focus_delay_secs = 0.5;
  cfg.touch_hold_delay_secs = 1.0;
  cfg.hide_delay_secs = 1.5;

  rc = ui_tooltip_base_create(&tooltip->base, &cfg);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(tooltip);
    return rc;
  }

  *out_tooltip = tooltip;
  return UI_ERROR_NONE;
}

ui_error_t md3_tooltip_destroy(struct md3_tooltip *tooltip) {
  ui_error_t rc;

  if (!tooltip) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_tooltip_base_destroy(tooltip->base);
  C_MULTIPLATFORM_FREE(tooltip);
  return rc;
}

/* =========================================================================
 * MD3 Snackbar
 * ========================================================================= */

ui_error_t md3_snackbar_create(struct ui_engine *engine,
                               enum md3_snackbar_variant variant,
                               const char *message,
                               struct md3_snackbar **out_snackbar) {
  struct md3_snackbar *snackbar;
  struct ui_dom_node *root_node = NULL;
  ui_error_t rc;

  if (!engine || !out_snackbar ||
      (unsigned)variant > (unsigned)MD3_SNACKBAR_FLOATING_PILL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  snackbar = (struct md3_snackbar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_snackbar));
  if (!snackbar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(snackbar, 0, sizeof(struct md3_snackbar));
  snackbar->variant = variant;

  if (message) {
#if defined(_MSC_VER)
    strncpy_s(snackbar->message, sizeof(snackbar->message), message,
              sizeof(snackbar->message) - 1);
#else
    strncpy(snackbar->message, message, sizeof(snackbar->message) - 1);
    snackbar->message[sizeof(snackbar->message) - 1] = '\0';
#endif
  }

  rc = ui_timer_create_monotonic(&snackbar->timer);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(snackbar);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    ui_timer_destroy(snackbar->timer);
    C_MULTIPLATFORM_FREE(snackbar);
    return rc;
  }

  rc = ui_overlay_director_create(root_node, &snackbar->director);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root_node);
    ui_timer_destroy(snackbar->timer);
    C_MULTIPLATFORM_FREE(snackbar);
    return rc;
  }

  rc = ui_snackbar_base_create(snackbar->timer, snackbar->director,
                               &snackbar->base);
  if (rc != UI_ERROR_NONE) {
    ui_overlay_director_destroy(snackbar->director);
    ui_timer_destroy(snackbar->timer);
    C_MULTIPLATFORM_FREE(snackbar);
    return rc;
  }

  *out_snackbar = snackbar;
  return UI_ERROR_NONE;
}

ui_error_t md3_snackbar_destroy(struct md3_snackbar *snackbar) {
  ui_error_t rc;

  if (!snackbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_snackbar_base_destroy(snackbar->base);
  ui_overlay_director_destroy(snackbar->director);
  ui_timer_destroy(snackbar->timer);

  C_MULTIPLATFORM_FREE(snackbar);
  return rc;
}

/* =========================================================================
 * MD3 Badge
 * ========================================================================= */

ui_error_t md3_badge_create(struct ui_engine *engine, int count,
                            struct md3_badge **out_badge) {
  struct md3_badge *badge;
  ui_error_t rc;

  if (!engine || !out_badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  badge = (struct md3_badge *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_badge));
  if (!badge) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(badge, 0, sizeof(struct md3_badge));
  if (count >= 0) {
    badge->count = count;
    badge->has_count = 1;
  }

  rc = ui_badge_base_create(&badge->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }

  *out_badge = badge;
  return UI_ERROR_NONE;
}

ui_error_t md3_badge_destroy(struct md3_badge *badge) {
  ui_error_t rc;

  if (!badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_badge_base_destroy(badge->base);
  C_MULTIPLATFORM_FREE(badge);
  return rc;
}

ui_error_t md3_badge_set_count(struct md3_badge *badge, int count) {
  if (!badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (count < 0) {
    badge->count = 0;
    badge->has_count = 0;
  } else {
    badge->count = count;
    badge->has_count = 1;
  }

  return UI_ERROR_NONE;
}

/* =========================================================================
 * MD3 Datepicker & Timepicker
 * ========================================================================= */

ui_error_t md3_datepicker_create(struct ui_engine *engine,
                                 struct md3_datepicker **out_picker) {
  struct md3_datepicker *picker;
  ui_error_t rc;

  if (!engine || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct md3_datepicker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_datepicker));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(picker, 0, sizeof(struct md3_datepicker));
  picker->year = 2026;
  picker->month = 1;
  picker->day = 1;

  rc = ui_input_base_create(&picker->input);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_popover_base_create(&picker->popover);
  if (rc != UI_ERROR_NONE) {
    ui_input_base_destroy(picker->input);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_calendar_base_create(&picker->calendar, NULL);
  if (rc != UI_ERROR_NONE) {
    ui_popover_base_destroy(picker->popover);
    ui_input_base_destroy(picker->input);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  rc = ui_datepicker_base_create(&picker->base, picker->input, picker->popover,
                                 picker->calendar, &picker->cva);
  if (rc != UI_ERROR_NONE) {
    ui_calendar_base_destroy(picker->calendar);
    ui_popover_base_destroy(picker->popover);
    ui_input_base_destroy(picker->input);
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t md3_datepicker_destroy(struct md3_datepicker *picker) {
  ui_error_t rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_datepicker_base_destroy(picker->base);
  ui_calendar_base_destroy(picker->calendar);
  ui_popover_base_destroy(picker->popover);
  ui_input_base_destroy(picker->input);

  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

ui_error_t md3_timepicker_create(struct ui_engine *engine, int is_24h,
                                 struct md3_timepicker **out_picker) {
  struct md3_timepicker *picker;
  ui_error_t rc;

  if (!engine || !out_picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  picker = (struct md3_timepicker *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_timepicker));
  if (!picker) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(picker, 0, sizeof(struct md3_timepicker));
  picker->is_24h = is_24h ? 1 : 0;

  rc = ui_timepicker_base_create(&picker->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(picker);
    return rc;
  }

  *out_picker = picker;
  return UI_ERROR_NONE;
}

ui_error_t md3_timepicker_destroy(struct md3_timepicker *picker) {
  ui_error_t rc;

  if (!picker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_timepicker_base_destroy(picker->base);
  C_MULTIPLATFORM_FREE(picker);
  return rc;
}

/* =========================================================================
 * MD3 Pull-to-Refresh
 * ========================================================================= */

ui_error_t md3_pull_to_refresh_create(struct ui_engine *engine,
                                      struct md3_pull_to_refresh **out_ptr) {
  struct md3_pull_to_refresh *ptr;
  ui_error_t rc;

  if (!engine || !out_ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ptr = (struct md3_pull_to_refresh *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_pull_to_refresh));
  if (!ptr) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ptr, 0, sizeof(struct md3_pull_to_refresh));
  ptr->spring_resistance = 0.5f;

  rc = ui_pull_to_refresh_base_create(&ptr->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(ptr);
    return rc;
  }

  *out_ptr = ptr;
  return UI_ERROR_NONE;
}

ui_error_t md3_pull_to_refresh_destroy(struct md3_pull_to_refresh *ptr) {
  ui_error_t rc;

  if (!ptr) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_pull_to_refresh_base_destroy(ptr->base);
  C_MULTIPLATFORM_FREE(ptr);
  return rc;
}
