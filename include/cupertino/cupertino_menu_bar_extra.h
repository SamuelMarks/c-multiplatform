/**
 * @file cupertino_menu_bar_extra.h
 * @brief macOS Menu Bar Extras / System Status Items conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_MENU_BAR_EXTRA_H
#define CUPERTINO_CUPERTINO_MENU_BAR_EXTRA_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_MENU_BAR_EXTRA_STANDARD_HEIGHT 24.0f

/**
 * @struct cupertino_menu_bar_extra_descriptor
 * @brief Initialization descriptor for a macOS Menu Bar Extra / Status Item.
 */
struct cupertino_menu_bar_extra_descriptor {
  const char *title;       /**< Optional text readout (e.g. clock, percentage).
                            */
  const char *icon_symbol; /**< SF Symbol icon identifier. */
  int is_animating;        /**< 1 if icon animation is running. */
  float animation_phase;   /**< Current cycle phase [0.0, 1.0]. */
  float fixed_width; /**< Explicit fixed width (or 0 for dynamic sizing). */
};

/**
 * @struct cupertino_menu_bar_extra
 * @brief Instance managing a status item in the macOS Menu Bar Extra tray.
 */
struct cupertino_menu_bar_extra {
  char title[64];        /**< Dynamic text string. */
  char icon_symbol[64];  /**< Active SF Symbol. */
  int is_animating;      /**< Animation state flag. */
  float animation_phase; /**< Phase progress [0.0, 1.0]. */
  float fixed_width;     /**< User fixed width override. */
  float computed_width;  /**< Final computed width. */
  float computed_height; /**< Standard status bar item height. */
  int is_menu_open;      /**< 1 if anchored dropdown popover menu is open. */
};

/**
 * @brief Creates a new macOS Menu Bar Extra status item.
 *
 * @param engine Pointer to ui_engine instance.
 * @param desc Configuration descriptor.
 * @param out_extra Pointer to receive allocated extra item.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_create(
    struct ui_engine *engine,
    const struct cupertino_menu_bar_extra_descriptor *desc,
    struct cupertino_menu_bar_extra **out_extra);

/**
 * @brief Recomputes the computed width and height based on title and icon.
 *
 * @param extra Target item.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_bar_extra_recompute_bounds(
    struct cupertino_menu_bar_extra *extra);

/**
 * @brief Destroys a Menu Bar Extra item.
 *
 * @param extra Item to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_bar_extra_destroy(struct cupertino_menu_bar_extra *extra);

/**
 * @brief Updates the text readout displayed in the status item.
 *
 * @param extra Target item.
 * @param title New text readout (or NULL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_set_title(
    struct cupertino_menu_bar_extra *extra, const char *title);

/**
 * @brief Gets the current text readout.
 *
 * @param extra Target item.
 * @param out_title Pointer to receive pointer to title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_get_title(
    const struct cupertino_menu_bar_extra *extra, const char **out_title);

/**
 * @brief Updates the icon symbol displayed in the status item.
 *
 * @param extra Target item.
 * @param icon_symbol New SF Symbol name (or NULL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_set_icon(
    struct cupertino_menu_bar_extra *extra, const char *icon_symbol);

/**
 * @brief Gets the current icon symbol name.
 *
 * @param extra Target item.
 * @param out_symbol Pointer to receive pointer to symbol name.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_get_icon(
    const struct cupertino_menu_bar_extra *extra, const char **out_symbol);

/**
 * @brief Controls status animation state and phase progression.
 *
 * @param extra Target item.
 * @param is_animating 1 to start animation, 0 to stop.
 * @param animation_phase Cycle progress [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_bar_extra_set_animating(struct cupertino_menu_bar_extra *extra,
                                       int is_animating, float animation_phase);

/**
 * @brief Queries current animation status and phase.
 *
 * @param extra Target item.
 * @param out_is_animating Pointer to receive animation flag.
 * @param out_phase Pointer to receive phase (or NULL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_is_animating(
    const struct cupertino_menu_bar_extra *extra, int *out_is_animating,
    float *out_phase);

/**
 * @brief Sets whether the anchored popover menu is currently open.
 *
 * @param extra Target item.
 * @param is_open 1 if open, 0 if closed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_set_menu_open(
    struct cupertino_menu_bar_extra *extra, int is_open);

/**
 * @brief Gets whether the anchored popover menu is open.
 *
 * @param extra Target item.
 * @param out_is_open Pointer to receive menu open state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_is_menu_open(
    const struct cupertino_menu_bar_extra *extra, int *out_is_open);

/**
 * @brief Simulates clicking the status item, toggling its open state.
 *
 * @param extra Target item.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_menu_bar_extra_click(struct cupertino_menu_bar_extra *extra);

/**
 * @brief Gets the layout bounds of the status item.
 *
 * @param extra Target item.
 * @param out_w Pointer to receive computed width in points.
 * @param out_h Pointer to receive computed height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_menu_bar_extra_get_bounds(
    const struct cupertino_menu_bar_extra *extra, float *out_w, float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_MENU_BAR_EXTRA_H */
