/**
 * @file cupertino_dock_tile.h
 * @brief macOS Dock Tile Live Sync Controller conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_DOCK_TILE_H
#define CUPERTINO_CUPERTINO_DOCK_TILE_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_DOCK_TILE_MAX_ACTIONS 16
#define CUPERTINO_DOCK_TILE_ICON_SIZE 128.0f

/**
 * @struct cupertino_dock_tile_action
 * @brief Represents an individual task action in the Dock contextual menu.
 */
struct cupertino_dock_tile_action {
  char title[64];  /**< Displayed action name. */
  int action_id;   /**< Application-defined unique action ID. */
  int is_disabled; /**< 1 if grayed out, 0 if interactive. */
};

/**
 * @struct cupertino_dock_tile_descriptor
 * @brief Initialization descriptor for the application Dock Tile controller.
 */
struct cupertino_dock_tile_descriptor {
  const char *initial_badge_label; /**< Initial badge string (or NULL). */
  float initial_progress;          /**< Initial progress fraction [0.0, 1.0]. */
  int show_progress;               /**< 1 if progress gauge is active. */
};

/**
 * @struct cupertino_dock_tile
 * @brief Controller managing live badge, progress bar overlay, and context
 * menu.
 */
struct cupertino_dock_tile {
  char badge_label[32]; /**< Badge label text (e.g. "5", "99+"). */
  int has_badge;        /**< 1 if badge pill is visible. */
  float progress;       /**< Progress fraction in [0.0, 1.0]. */
  int show_progress;    /**< 1 if progress bar overlay is visible. */
  struct cupertino_dock_tile_action
      actions[CUPERTINO_DOCK_TILE_MAX_ACTIONS]; /**< Dock context actions. */
  size_t action_count; /**< Number of configured actions. */
};

/**
 * @brief Creates a new macOS Dock Tile controller.
 *
 * @param engine Pointer to ui_engine instance.
 * @param desc Configuration descriptor.
 * @param out_dock_tile Pointer to receive allocated dock tile controller.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_create(
    struct ui_engine *engine, const struct cupertino_dock_tile_descriptor *desc,
    struct cupertino_dock_tile **out_dock_tile);

/**
 * @brief Destroys a Dock Tile controller.
 *
 * @param dock_tile Controller to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_dock_tile_destroy(struct cupertino_dock_tile *dock_tile);

/**
 * @brief Sets or clears the dynamic red badge count pill on the Dock tile.
 *
 * @param dock_tile Target controller.
 * @param label Text count (e.g. "3", "99+"), or NULL/empty to clear.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_set_badge_label(
    struct cupertino_dock_tile *dock_tile, const char *label);

/**
 * @brief Gets the current badge label.
 *
 * @param dock_tile Target controller.
 * @param out_label Pointer to receive pointer to badge string.
 * @param out_has_badge Pointer to receive 1 if badge present, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_dock_tile_get_badge_label(const struct cupertino_dock_tile *dock_tile,
                                    const char **out_label, int *out_has_badge);

/**
 * @brief Sets the mini progress bar / gauge overlay drawn onto the Dock tile.
 *
 * @param dock_tile Target controller.
 * @param progress Fraction in [0.0, 1.0].
 * @param is_visible 1 to display progress bar, 0 to hide.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_set_progress(
    struct cupertino_dock_tile *dock_tile, float progress, int is_visible);

/**
 * @brief Gets current progress bar status and fraction.
 *
 * @param dock_tile Target controller.
 * @param out_progress Pointer to receive progress fraction.
 * @param out_is_visible Pointer to receive visibility flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_dock_tile_get_progress(const struct cupertino_dock_tile *dock_tile,
                                 float *out_progress, int *out_is_visible);

/**
 * @brief Adds a task item to the right-click Dock contextual menu.
 *
 * @param dock_tile Target controller.
 * @param title Action label.
 * @param action_id Application-specific identifier.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_add_action(
    struct cupertino_dock_tile *dock_tile, const char *title, int action_id);

/**
 * @brief Gets the number of registered Dock context actions.
 *
 * @param dock_tile Target controller.
 * @param out_count Pointer to receive action count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_get_action_count(
    const struct cupertino_dock_tile *dock_tile, size_t *out_count);

/**
 * @brief Gets action details at the specified index.
 *
 * @param dock_tile Target controller.
 * @param index Action item index.
 * @param out_title Pointer to receive action title.
 * @param out_action_id Pointer to receive action ID.
 * @param out_is_disabled Pointer to receive disabled flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_get_action_at(
    const struct cupertino_dock_tile *dock_tile, size_t index,
    const char **out_title, int *out_action_id, int *out_is_disabled);

/**
 * @brief Triggers execution of a Dock contextual menu action.
 *
 * @param dock_tile Target controller.
 * @param action_id Action ID to invoke.
 * @param out_handled Pointer to receive 1 if matched and handled, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_trigger_action(
    struct cupertino_dock_tile *dock_tile, int action_id, int *out_handled);

/**
 * @brief Computes geometry of the badge pill positioned in the top-right
 * corner.
 *
 * @param dock_tile Target controller.
 * @param out_x Pointer to receive top-left X coordinate in points.
 * @param out_y Pointer to receive top-left Y coordinate in points.
 * @param out_w Pointer to receive capsule width in points.
 * @param out_h Pointer to receive capsule height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_dock_tile_get_badge_bounds(
    const struct cupertino_dock_tile *dock_tile, float *out_x, float *out_y,
    float *out_w, float *out_h);

/**
 * @brief Computes geometry of the progress bar along the bottom of the tile.
 *
 * @param dock_tile Target controller.
 * @param out_x Pointer to receive top-left X coordinate in points.
 * @param out_y Pointer to receive top-left Y coordinate in points.
 * @param out_w Pointer to receive bar width in points.
 * @param out_h Pointer to receive bar height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_dock_tile_get_progress_bounds(
    const struct cupertino_dock_tile *dock_tile, float *out_x, float *out_y,
    float *out_w, float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_DOCK_TILE_H */
