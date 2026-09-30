/**
 * @file md3_overlay.h
 * @brief Material 3 Overlays, Dialogs, Sheets, Menus, Tooltips, Snackbars,
 * Badges, Pickers, Pull-to-refresh.
 */

#ifndef MATERIAL3_MD3_OVERLAY_H
#define MATERIAL3_MD3_OVERLAY_H

/* clang-format off */
#include "ui_badge_base.h"
#include "ui_bottom_sheet_base.h"
#include "ui_calendar_base.h"
#include "ui_control_value_accessor.h"
#include "ui_datepicker_base.h"
#include "ui_dialog_base.h"
#include "ui_error.h"
#include "ui_input_base.h"
#include "ui_menu_base.h"
#include "ui_overlay_director.h"
#include "ui_popover_base.h"
#include "ui_pull_to_refresh_base.h"
#include "ui_side_sheet_base.h"
#include "ui_snackbar_base.h"
#include "ui_timepicker_base.h"
#include "ui_timer.h"
#include "ui_tooltip_base.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/* -------------------------------------------------------------------------
 * MD3 Dialog (Basic Alert Dialog and Full-screen Dialog)
 * ------------------------------------------------------------------------- */

/**
 * @enum md3_dialog_variant
 * @brief Material 3 Dialog variants.
 */
enum md3_dialog_variant {
  MD3_DIALOG_ALERT = 0,  /**< Centered modal alert with actions */
  MD3_DIALOG_FULL_SCREEN /**< Edge-to-edge modal layout on mobile */
};

/**
 * @struct md3_dialog
 * @brief Material 3 Dialog wrapping ui_dialog_base.
 */
struct md3_dialog {
  struct ui_dialog_base *base;
  enum md3_dialog_variant variant;
  char headline[128];
  char supporting_text[256];
  int is_open;
};

/**
 * @brief Creates a Material 3 Dialog.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Dialog variant (Alert, Full-screen).
 * @param out_dialog Pointer to receive newly created dialog.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_dialog_create(struct ui_engine *engine, enum md3_dialog_variant variant,
                  struct md3_dialog **out_dialog);

/**
 * @brief Destroys a Material 3 Dialog.
 *
 * @param dialog Dialog to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_dialog_destroy(struct md3_dialog *dialog);

/**
 * @brief Sets dialog headline text.
 *
 * @param dialog The dialog.
 * @param headline Headline string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_dialog_set_headline(struct md3_dialog *dialog, const char *headline);

/**
 * @brief Sets dialog supporting text.
 *
 * @param dialog The dialog.
 * @param text Supporting text string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_dialog_set_supporting_text(struct md3_dialog *dialog, const char *text);

/**
 * @brief Opens or closes the dialog.
 *
 * @param dialog The dialog.
 * @param is_open Non-zero to open, 0 to close.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_dialog_set_open(struct md3_dialog *dialog, int is_open);

/* -------------------------------------------------------------------------
 * MD3 Bottom Sheet (Standard, Modal, Expressive Floating Island)
 * ------------------------------------------------------------------------- */

/**
 * @enum md3_bottom_sheet_variant
 * @brief Material 3 Bottom Sheet variants.
 */
enum md3_bottom_sheet_variant {
  MD3_BOTTOM_SHEET_STANDARD = 0, /**< Non-modal co-existing with screen */
  MD3_BOTTOM_SHEET_MODAL,        /**< Modal with dimming scrim */
  MD3_BOTTOM_SHEET_FLOATING /**< Expressive floating island style with margin */
};

/**
 * @struct md3_bottom_sheet
 * @brief Material 3 Bottom Sheet wrapping ui_bottom_sheet_base.
 */
struct md3_bottom_sheet {
  struct ui_bottom_sheet_base *base;
  enum md3_bottom_sheet_variant variant;
  int is_open;
};

/**
 * @brief Creates a Material 3 Bottom Sheet.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Sheet variant (Standard, Modal, Floating Island).
 * @param out_sheet Pointer to receive newly created bottom sheet.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_bottom_sheet_create(
    struct ui_engine *engine, enum md3_bottom_sheet_variant variant,
    struct md3_bottom_sheet **out_sheet);

/**
 * @brief Destroys a Material 3 Bottom Sheet.
 *
 * @param sheet Sheet to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_bottom_sheet_destroy(struct md3_bottom_sheet *sheet);

/**
 * @brief Opens or closes the bottom sheet.
 *
 * @param sheet The sheet.
 * @param is_open Non-zero to open, 0 to close.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_bottom_sheet_set_open(struct md3_bottom_sheet *sheet, int is_open);

/* -------------------------------------------------------------------------
 * MD3 Side Sheet (Standard and Modal side sheets)
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_side_sheet
 * @brief Material 3 Side Sheet wrapping ui_side_sheet_base.
 */
struct md3_side_sheet {
  struct ui_side_sheet_base *base;
  int is_modal;
  int is_open;
};

/**
 * @brief Creates a Material 3 Side Sheet.
 *
 * @param engine Pointer to ui_engine.
 * @param is_modal Non-zero for modal sheet with scrim backdrop.
 * @param out_sheet Pointer to receive newly created side sheet.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_side_sheet_create(
    struct ui_engine *engine, int is_modal, struct md3_side_sheet **out_sheet);

/**
 * @brief Destroys a Material 3 Side Sheet.
 *
 * @param sheet Sheet to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_side_sheet_destroy(struct md3_side_sheet *sheet);

/**
 * @brief Opens or closes the side sheet.
 *
 * @param sheet The sheet.
 * @param is_open Non-zero to open, 0 to close.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_side_sheet_set_open(struct md3_side_sheet *sheet, int is_open);

/* -------------------------------------------------------------------------
 * MD3 Menu (Dropdown menu with auto-flip collision avoidance)
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_menu
 * @brief Material 3 Menu wrapping ui_menu_base.
 */
struct md3_menu {
  struct ui_menu_base *base;
  int is_open;
};

/**
 * @brief Creates a Material 3 Dropdown Menu.
 *
 * @param engine Pointer to ui_engine.
 * @param out_menu Pointer to receive newly created menu.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_menu_create(struct ui_engine *engine, struct md3_menu **out_menu);

/**
 * @brief Destroys a Material 3 Dropdown Menu.
 *
 * @param menu Menu to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_menu_destroy(struct md3_menu *menu);

/* -------------------------------------------------------------------------
 * MD3 Tooltip (Plain 24dp and Rich multi-line Tooltips)
 * ------------------------------------------------------------------------- */

/**
 * @enum md3_tooltip_variant
 * @brief Material 3 Tooltip variants.
 */
enum md3_tooltip_variant {
  MD3_TOOLTIP_PLAIN = 0, /**< Plain single-line label */
  MD3_TOOLTIP_RICH       /**< Rich multi-line container with title/action */
};

/**
 * @struct md3_tooltip
 * @brief Material 3 Tooltip wrapping ui_tooltip_base.
 */
struct md3_tooltip {
  struct ui_tooltip_base *base;
  enum md3_tooltip_variant variant;
  char text[128];
  char subhead[128];
};

/**
 * @brief Creates a Material 3 Tooltip.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Plain or Rich tooltip.
 * @param text Tooltip message text.
 * @param out_tooltip Pointer to receive newly created tooltip.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tooltip_create(struct ui_engine *engine, enum md3_tooltip_variant variant,
                   const char *text, struct md3_tooltip **out_tooltip);

/**
 * @brief Destroys a Material 3 Tooltip.
 *
 * @param tooltip Tooltip to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_tooltip_destroy(struct md3_tooltip *tooltip);

/* -------------------------------------------------------------------------
 * MD3 Snackbar (Single-line, Two-line, Expressive Floating Pill)
 * ------------------------------------------------------------------------- */

/**
 * @enum md3_snackbar_variant
 * @brief Material 3 Snackbar variants.
 */
enum md3_snackbar_variant {
  MD3_SNACKBAR_SINGLE_LINE = 0, /**< Single-line bottom anchored */
  MD3_SNACKBAR_TWO_LINE,        /**< Two-line bottom anchored */
  MD3_SNACKBAR_FLOATING_PILL    /**< Expressive floating pill */
};

/**
 * @struct md3_snackbar
 * @brief Material 3 Snackbar wrapping ui_snackbar_base.
 */
struct md3_snackbar {
  struct ui_snackbar_base *base;
  struct ui_timer *timer;
  struct ui_overlay_director *director;
  enum md3_snackbar_variant variant;
  char message[256];
  char action_label[64];
};

/**
 * @brief Creates a Material 3 Snackbar.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Visual variant (Single-line, Two-line, Floating Pill).
 * @param message Message text to display.
 * @param out_snackbar Pointer to receive newly created snackbar.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_snackbar_create(struct ui_engine *engine, enum md3_snackbar_variant variant,
                    const char *message, struct md3_snackbar **out_snackbar);

/**
 * @brief Destroys a Material 3 Snackbar.
 *
 * @param snackbar Snackbar to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_snackbar_destroy(struct md3_snackbar *snackbar);

/* -------------------------------------------------------------------------
 * MD3 Badge (Small 6dp dot and Large with count)
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_badge
 * @brief Material 3 Badge wrapping ui_badge_base.
 */
struct md3_badge {
  struct ui_badge_base *base;
  int count;
  int has_count;
};

/**
 * @brief Creates a Material 3 Badge.
 *
 * @param engine Pointer to ui_engine.
 * @param count Initial badge count (-1 for small dot badge without number).
 * @param out_badge Pointer to receive newly created badge.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_badge_create(
    struct ui_engine *engine, int count, struct md3_badge **out_badge);

/**
 * @brief Destroys a Material 3 Badge.
 *
 * @param badge Badge to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_badge_destroy(struct md3_badge *badge);

/**
 * @brief Updates the count displayed in the badge.
 *
 * @param badge The badge.
 * @param count New count (-1 to clear number and display small dot).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_badge_set_count(struct md3_badge *badge, int count);

/* -------------------------------------------------------------------------
 * MD3 Datepicker & Timepicker
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_datepicker
 * @brief Material 3 Datepicker wrapping ui_datepicker_base.
 */
struct md3_datepicker {
  struct ui_datepicker_base *base;
  struct ui_input_base *input;
  struct ui_popover_base *popover;
  struct ui_calendar_base *calendar;
  struct ui_control_value_accessor cva;
  int year;
  int month;
  int day;
};

/**
 * @brief Creates a Material 3 Datepicker.
 *
 * @param engine Pointer to ui_engine.
 * @param out_picker Pointer to receive newly created datepicker.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_datepicker_create(
    struct ui_engine *engine, struct md3_datepicker **out_picker);

/**
 * @brief Destroys a Material 3 Datepicker.
 *
 * @param picker Datepicker to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_datepicker_destroy(struct md3_datepicker *picker);

/**
 * @struct md3_timepicker
 * @brief Material 3 Timepicker wrapping ui_timepicker_base.
 */
struct md3_timepicker {
  struct ui_timepicker_base *base;
  int hour;
  int minute;
  int is_24h;
};

/**
 * @brief Creates a Material 3 Timepicker.
 *
 * @param engine Pointer to ui_engine.
 * @param is_24h Non-zero for 24-hour military clock format.
 * @param out_picker Pointer to receive newly created timepicker.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_timepicker_create(
    struct ui_engine *engine, int is_24h, struct md3_timepicker **out_picker);

/**
 * @brief Destroys a Material 3 Timepicker.
 *
 * @param picker Timepicker to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_timepicker_destroy(struct md3_timepicker *picker);

/* -------------------------------------------------------------------------
 * MD3 Pull-to-Refresh (Expressive spring-loaded physics)
 * ------------------------------------------------------------------------- */

/**
 * @struct md3_pull_to_refresh
 * @brief Material 3 Expressive Pull-to-refresh component wrapping
 * ui_pull_to_refresh_base.
 */
struct md3_pull_to_refresh {
  struct ui_pull_to_refresh_base *base;
  float spring_resistance;
};

/**
 * @brief Creates a Material 3 Pull-to-refresh component.
 *
 * @param engine Pointer to ui_engine.
 * @param out_ptr Pointer to receive newly created pull-to-refresh instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_pull_to_refresh_create(
    struct ui_engine *engine, struct md3_pull_to_refresh **out_ptr);

/**
 * @brief Destroys a Material 3 Pull-to-refresh component.
 *
 * @param ptr Component to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_pull_to_refresh_destroy(struct md3_pull_to_refresh *ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_OVERLAY_H */
