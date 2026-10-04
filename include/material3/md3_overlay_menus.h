/**
 * @file md3_overlay_menus.h
 * @brief Material 3 & Expressive Overlay, Dialog, Menu, and Picker Components.
 *
 * Provides spec-compliant Material 3 implementations for:
 * - md3_date_range_picker (wrapping ui_date_range_picker_base)
 * - md3_hover_card (wrapping ui_hover_card_base)
 * - md3_popover (wrapping ui_popover_base)
 * - md3_banner (wrapping ui_banner_base)
 * - md3_inline_alert (wrapping ui_alert_base)
 * - md3_menubar (wrapping ui_menubar_base)
 * - md3_context_menu (wrapping ui_context_menu_base)
 * - md3_action_sheet (wrapping ui_action_sheet_base)
 * - md3_coachmark (wrapping ui_coachmark_base)
 * - md3_color_picker (wrapping ui_color_picker_base)
 * - md3_command_palette (wrapping ui_command_palette_base)
 */

#ifndef MATERIAL3_MD3_OVERLAY_MENUS_H
#define MATERIAL3_MD3_OVERLAY_MENUS_H

/* clang-format off */
#include "ui_action_sheet_base.h"
#include "ui_alert_base.h"
#include "ui_banner_base.h"
#include "ui_coachmark_base.h"
#include "ui_color_picker_base.h"
#include "ui_command_palette_base.h"
#include "ui_component.h"
#include "ui_context_menu_base.h"
#include "ui_control_value_accessor.h"
#include "ui_date_range_picker_base.h"
#include "ui_error.h"
#include "ui_hover_card_base.h"
#include "ui_menubar_base.h"
#include "ui_popover_base.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/* ========================================================================= */
/* md3_hover_card                                                            */
/* ========================================================================= */

/**
 * @struct md3_hover_card
 * @brief Material 3 Hover Card wrapping ui_hover_card_base.
 */
struct md3_hover_card {
  struct ui_hover_card_base *base;
  struct ui_dom_node *anchor_node;
  int is_visible;
  float open_delay_ms;
  float close_delay_ms;
};

/**
 * @brief Creates a Material 3 Hover Card.
 *
 * @param engine Pointer to ui_engine instance.
 * @param anchor_node DOM node anchor element.
 * @param out_hover_card Pointer to receive allocated hover card.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_hover_card_create(struct ui_engine *engine, struct ui_dom_node *anchor_node,
                      struct md3_hover_card **out_hover_card);

/**
 * @brief Destroys a Hover Card instance.
 *
 * @param hover_card Hover card to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_hover_card_destroy(struct md3_hover_card *hover_card);

/**
 * @brief Simulates mouse entering the trigger anchor.
 *
 * @param hover_card Hover card instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_hover_card_show(struct md3_hover_card *hover_card);

/**
 * @brief Simulates mouse leaving the trigger anchor.
 *
 * @param hover_card Hover card instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_hover_card_hide(struct md3_hover_card *hover_card);

/* ========================================================================= */
/* md3_popover                                                               */
/* ========================================================================= */

/**
 * @struct md3_popover
 * @brief Material 3 Popover wrapping ui_popover_base.
 */
struct md3_popover {
  struct ui_popover_base *base;
  struct ui_dom_node *anchor_node;
  int is_open;
  int elevation_level;
};

/**
 * @brief Creates a Material 3 Popover.
 *
 * @param engine Pointer to ui_engine instance.
 * @param anchor_node Target anchor DOM node.
 * @param out_popover Pointer to receive allocated popover.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_popover_create(struct ui_engine *engine, struct ui_dom_node *anchor_node,
                   struct md3_popover **out_popover);

/**
 * @brief Destroys a Popover instance.
 *
 * @param popover Popover to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_popover_destroy(struct md3_popover *popover);

/**
 * @brief Opens the popover overlay.
 *
 * @param popover Popover instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_popover_open(struct md3_popover *popover);

/**
 * @brief Closes the popover overlay.
 *
 * @param popover Popover instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_popover_close(struct md3_popover *popover);

/**
 * @brief Queries whether the popover is currently open.
 *
 * @param popover Popover instance.
 * @param out_is_open Pointer to receive open status (1 open, 0 closed).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_popover_is_open(const struct md3_popover *popover, int *out_is_open);

/* ========================================================================= */
/* md3_banner                                                                */
/* ========================================================================= */

/**
 * @struct md3_banner
 * @brief Material 3 Banner wrapping ui_banner_base.
 */
struct md3_banner {
  struct ui_banner_base *base;
  char message[256];
  char action_primary[64];
  char action_secondary[64];
  int is_open;
  int is_dismissible;
};

/**
 * @brief Creates a Material 3 Banner.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_banner Pointer to receive allocated banner.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_banner_create(struct ui_engine *engine, struct md3_banner **out_banner);

/**
 * @brief Destroys a Banner instance.
 *
 * @param banner Banner to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_banner_destroy(struct md3_banner *banner);

/**
 * @brief Sets banner supporting text message.
 *
 * @param banner Banner instance.
 * @param text Text message string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_banner_set_text(struct md3_banner *banner, const char *text);

/**
 * @brief Configures a banner action button (index 0=primary, 1=secondary).
 *
 * @param banner Banner instance.
 * @param action_index 0 for primary action, 1 for secondary action.
 * @param label Action button text label.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_banner_set_action(
    struct md3_banner *banner, int action_index, const char *label);

/**
 * @brief Sets banner open status.
 *
 * @param banner Banner instance.
 * @param is_open 1 to show banner, 0 to dismiss.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_banner_set_open(struct md3_banner *banner, int is_open);

/**
 * @brief Checks if banner is currently visible.
 *
 * @param banner Banner instance.
 * @param out_is_open Pointer to receive 1 if open, 0 if dismissed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_banner_is_open(const struct md3_banner *banner, int *out_is_open);

/* ========================================================================= */
/* md3_inline_alert                                                          */
/* ========================================================================= */

/**
 * @enum md3_alert_severity
 * @brief Alert severity levels mapping to Material 3 tonal containers.
 */
enum md3_alert_severity {
  MD3_ALERT_SEVERITY_INFO = 0,    /**< Primary Container */
  MD3_ALERT_SEVERITY_SUCCESS = 1, /**< Secondary Container */
  MD3_ALERT_SEVERITY_WARNING = 2, /**< Tertiary Container */
  MD3_ALERT_SEVERITY_ERROR = 3    /**< Error Container */
};

/**
 * @struct md3_inline_alert
 * @brief Material 3 Contextual Inline Alert wrapping ui_alert_base.
 */
struct md3_inline_alert {
  struct ui_alert_base *base;
  enum md3_alert_severity severity;
  char title[128];
  char message[256];
  int is_dismissible;
  int is_dismissed;
};

/**
 * @brief Creates a Material 3 Inline Alert.
 *
 * @param engine Pointer to ui_engine instance.
 * @param severity Severity level (Info, Success, Warning, Error).
 * @param out_alert Pointer to receive allocated alert.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_inline_alert_create(
    struct ui_engine *engine, enum md3_alert_severity severity,
    struct md3_inline_alert **out_alert);

/**
 * @brief Destroys an Inline Alert instance.
 *
 * @param alert Alert to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_inline_alert_destroy(struct md3_inline_alert *alert);

/**
 * @brief Sets alert title headline.
 *
 * @param alert Alert instance.
 * @param title Title headline string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_inline_alert_set_title(struct md3_inline_alert *alert, const char *title);

/**
 * @brief Sets alert message description body.
 *
 * @param alert Alert instance.
 * @param message Description message string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_inline_alert_set_message(
    struct md3_inline_alert *alert, const char *message);

/**
 * @brief Sets whether the alert displays a trailing dismiss button.
 *
 * @param alert Alert instance.
 * @param dismissible 1 if dismissible, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_inline_alert_set_dismissible(
    struct md3_inline_alert *alert, int dismissible);

/* ========================================================================= */
/* md3_menubar                                                               */
/* ========================================================================= */

/**
 * @struct md3_menubar
 * @brief Material 3 Menubar wrapping ui_menubar_base.
 */
struct md3_menubar {
  struct ui_menubar_base *base;
  size_t item_count;
};

/**
 * @brief Creates a Material 3 Menubar.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_menubar Pointer to receive allocated menubar.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_menubar_create(struct ui_engine *engine, struct md3_menubar **out_menubar);

/**
 * @brief Destroys a Menubar instance.
 *
 * @param menubar Menubar to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_menubar_destroy(struct md3_menubar *menubar);

/**
 * @brief Appends a top-level menu item to the menubar.
 *
 * @param menubar Menubar instance.
 * @param item Menu item component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_menubar_append_item(struct md3_menubar *menubar, struct ui_component *item);

/* ========================================================================= */
/* md3_context_menu                                                          */
/* ========================================================================= */

/**
 * @struct md3_context_menu
 * @brief Material 3 Context Menu wrapping ui_context_menu_base.
 */
struct md3_context_menu {
  struct ui_context_menu_base *base;
  int is_open;
  int current_x;
  int current_y;
};

/**
 * @brief Creates a Material 3 Context Menu.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_context_menu Pointer to receive allocated context menu.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_context_menu_create(
    struct ui_engine *engine, struct md3_context_menu **out_context_menu);

/**
 * @brief Destroys a Context Menu instance.
 *
 * @param menu Context menu to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_context_menu_destroy(struct md3_context_menu *menu);

/**
 * @brief Opens the context menu at target coordinates with viewport collision
 * clamping.
 *
 * @param menu Context menu instance.
 * @param x Screen X coordinate.
 * @param y Screen Y coordinate.
 * @param viewport_w Viewport width.
 * @param viewport_h Viewport height.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_context_menu_open_at(struct md3_context_menu *menu, int x, int y,
                         int viewport_w, int viewport_h);

/* ========================================================================= */
/* md3_action_sheet                                                          */
/* ========================================================================= */

/**
 * @struct md3_action_sheet_item
 * @brief Stacked action item inside an action sheet.
 */
struct md3_action_sheet_item {
  char label[64];
  char icon[32];
};

/**
 * @struct md3_action_sheet
 * @brief Material 3 Action Sheet wrapping ui_action_sheet_base.
 */
struct md3_action_sheet {
  struct ui_action_sheet_base *base;
  struct md3_action_sheet_item actions[16];
  size_t action_count;
  char cancel_label[64];
  int is_open;
};

/**
 * @brief Creates a Material 3 Action Sheet.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_sheet Pointer to receive allocated action sheet.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_action_sheet_create(
    struct ui_engine *engine, struct md3_action_sheet **out_sheet);

/**
 * @brief Destroys an Action Sheet instance.
 *
 * @param sheet Action sheet to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_action_sheet_destroy(struct md3_action_sheet *sheet);

/**
 * @brief Appends an action button option to the action sheet.
 *
 * @param sheet Action sheet instance.
 * @param label Action button label text.
 * @param icon Optional Material Symbols icon name.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_action_sheet_add_action(
    struct md3_action_sheet *sheet, const char *label, const char *icon);

/**
 * @brief Sets dedicated cancel action button label separated by an 8dp gap.
 *
 * @param sheet Action sheet instance.
 * @param cancel_label Cancel button text label.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_action_sheet_set_cancel_action(
    struct md3_action_sheet *sheet, const char *cancel_label);

/**
 * @brief Toggles action sheet open/close state.
 *
 * @param sheet Action sheet instance.
 * @param is_open 1 to open sheet, 0 to close.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_action_sheet_set_open(struct md3_action_sheet *sheet, int is_open);

/**
 * @brief Checks if action sheet is currently open.
 *
 * @param sheet Action sheet instance.
 * @param out_is_open Pointer to receive 1 if open, 0 if closed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_action_sheet_is_open(
    const struct md3_action_sheet *sheet, int *out_is_open);

/* ========================================================================= */
/* md3_coachmark                                                             */
/* ========================================================================= */

/**
 * @struct md3_coachmark
 * @brief Material 3 Coachmark Guided Tour wrapping ui_coachmark_tour.
 */
struct md3_coachmark {
  struct ui_coachmark_tour *base;
  struct ui_dom_node *target_node;
  char title[128];
  char description[256];
  int current_step;
  int total_steps;
  int is_active;
};

/**
 * @brief Creates a Material 3 Coachmark guided tour element.
 *
 * @param engine Pointer to ui_engine instance.
 * @param target_node Target DOM node element to spotlight.
 * @param out_coachmark Pointer to receive allocated coachmark.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_coachmark_create(struct ui_engine *engine, struct ui_dom_node *target_node,
                     struct md3_coachmark **out_coachmark);

/**
 * @brief Destroys a Coachmark tour element.
 *
 * @param coachmark Coachmark to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_coachmark_destroy(struct md3_coachmark *coachmark);

/**
 * @brief Sets headline, descriptive text, and step metadata.
 *
 * @param coachmark Coachmark instance.
 * @param title Step headline text.
 * @param description Detailed description text.
 * @param step_index Current 0-based step index.
 * @param total_steps Total steps in guided walkthrough.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_coachmark_set_content(
    struct md3_coachmark *coachmark, const char *title, const char *description,
    int step_index, int total_steps);

/**
 * @brief Activates the spotlight overlay and pulsing beacon ring.
 *
 * @param coachmark Coachmark instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_coachmark_start(struct md3_coachmark *coachmark);

/* ========================================================================= */
/* md3_color_picker                                                          */
/* ========================================================================= */

/**
 * @struct md3_color_picker
 * @brief Material 3 Color Picker wrapping ui_color_picker_base.
 */
struct md3_color_picker {
  struct ui_color_picker_base *base;
  struct ui_control_value_accessor cva;
  struct ui_color_hsv hsv;
  struct ui_color_rgb rgb;
  char hex_string[16];
};

/**
 * @brief Creates a Material 3 Color Picker.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_picker Pointer to receive allocated color picker.
 * @param out_cva Optional pointer to receive CVA handle.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_picker_create(
    struct ui_engine *engine, struct md3_color_picker **out_picker,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Color Picker instance.
 *
 * @param picker Color picker to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_color_picker_destroy(struct md3_color_picker *picker);

/**
 * @brief Sets color using HSV color coordinates.
 *
 * @param picker Color picker instance.
 * @param h Hue in degrees [0, 360).
 * @param s Saturation in [0.0, 1.0].
 * @param v Value/brightness in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_picker_set_hsv(
    struct md3_color_picker *picker, double h, double s, double v);

/**
 * @brief Gets current HSV color coordinates.
 *
 * @param picker Color picker instance.
 * @param out_h Pointer to receive hue.
 * @param out_s Pointer to receive saturation.
 * @param out_v Pointer to receive value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_color_picker_get_hsv(const struct md3_color_picker *picker, double *out_h,
                         double *out_s, double *out_v);

/**
 * @brief Sets color using RGB color channels.
 *
 * @param picker Color picker instance.
 * @param r Red channel [0, 255].
 * @param g Green channel [0, 255].
 * @param b Blue channel [0, 255].
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_color_picker_set_rgb(struct md3_color_picker *picker, unsigned char r,
                         unsigned char g, unsigned char b);

/**
 * @brief Gets current RGB color channels.
 *
 * @param picker Color picker instance.
 * @param out_r Pointer to receive red channel.
 * @param out_g Pointer to receive green channel.
 * @param out_b Pointer to receive blue channel.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_picker_get_rgb(
    const struct md3_color_picker *picker, unsigned char *out_r,
    unsigned char *out_g, unsigned char *out_b);

/**
 * @brief Sets color from hexadecimal string representation.
 *
 * @param picker Color picker instance.
 * @param hex Hex color string (e.g. "#FF5722").
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_color_picker_set_hex(struct md3_color_picker *picker, const char *hex);

/**
 * @brief Gets current color as hexadecimal string.
 *
 * @param picker Color picker instance.
 * @param out_hex Buffer to receive hex string.
 * @param hex_size Capacity of out_hex buffer (at least 8 bytes).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_picker_get_hex(
    const struct md3_color_picker *picker, char *out_hex, size_t hex_size);

/* ========================================================================= */
/* md3_command_palette                                                       */
/* ========================================================================= */

/**
 * @struct md3_command_item
 * @brief Action entry in a command palette.
 */
struct md3_command_item {
  char category[64];
  char title[128];
  char shortcut[32];
};

/**
 * @struct md3_command_palette
 * @brief Material 3 Command Palette wrapping ui_command_palette_base.
 */
struct md3_command_palette {
  struct ui_command_palette_base base;
  struct md3_command_item commands[64];
  size_t command_count;
  int is_open;
};

/**
 * @brief Creates a Material 3 Command Palette.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_palette Pointer to receive allocated command palette.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_command_palette_create(
    struct ui_engine *engine, struct md3_command_palette **out_palette);

/**
 * @brief Destroys a Command Palette instance.
 *
 * @param palette Command palette to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_command_palette_destroy(struct md3_command_palette *palette);

/**
 * @brief Registers a command destination or hotkey action.
 *
 * @param palette Command palette instance.
 * @param category Category section header.
 * @param title Action display text.
 * @param shortcut Optional keyboard shortcut accelerator tag.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_command_palette_add_action(
    struct md3_command_palette *palette, const char *category,
    const char *title, const char *shortcut);

/**
 * @brief Opens the modal command palette overlay.
 *
 * @param palette Command palette instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_command_palette_open(struct md3_command_palette *palette);

/**
 * @brief Closes the modal command palette overlay.
 *
 * @param palette Command palette instance.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_command_palette_close(struct md3_command_palette *palette);

/**
 * @brief Checks if command palette is open.
 *
 * @param palette Command palette instance.
 * @param out_is_open Pointer to receive 1 if open, 0 if closed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_command_palette_is_open(
    const struct md3_command_palette *palette, int *out_is_open);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_OVERLAY_MENUS_H */
