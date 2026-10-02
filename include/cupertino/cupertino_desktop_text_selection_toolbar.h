/**
 * @file cupertino_desktop_text_selection_toolbar.h
 * @brief macOS Desktop Text Selection Context Menu Toolbar conforming to Apple
 * HIG.
 */

#ifndef CUPERTINO_CUPERTINO_DESKTOP_TEXT_SELECTION_TOOLBAR_H
#define CUPERTINO_CUPERTINO_DESKTOP_TEXT_SELECTION_TOOLBAR_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_DESKTOP_TEXT_TOOLBAR_MAX_ITEMS 8
#define CUPERTINO_DESKTOP_TEXT_TOOLBAR_WIDTH 190.0f
#define CUPERTINO_DESKTOP_TEXT_TOOLBAR_ROW_HEIGHT 22.0f

/**
 * @struct cupertino_desktop_text_toolbar_item
 * @brief Represents an action row in the desktop text selection context menu.
 */
struct cupertino_desktop_text_toolbar_item {
  char title[32];          /**< Menu item label. */
  char shortcut[32];       /**< Keyboard accelerator (e.g. "Cmd+C"). */
  int action_id;           /**< Action identifier. */
  int has_separator_below; /**< 1 if hairline separator follows item. */
  int is_disabled;         /**< 1 if disabled, 0 if active. */
};

/**
 * @struct cupertino_desktop_text_selection_toolbar_descriptor
 * @brief Initialization descriptor for the desktop text selection toolbar.
 */
struct cupertino_desktop_text_selection_toolbar_descriptor {
  int can_cut;        /**< 1 if Cut is enabled. */
  int can_copy;       /**< 1 if Copy is enabled. */
  int can_paste;      /**< 1 if Paste is enabled. */
  int can_select_all; /**< 1 if Select All is enabled. */
};

/**
 * @struct cupertino_desktop_text_selection_toolbar
 * @brief Instance managing the macOS desktop right-click text selection menu.
 */
struct cupertino_desktop_text_selection_toolbar {
  int is_visible;    /**< Visibility state flag. */
  float x;           /**< Top-left X anchor coordinate. */
  float y;           /**< Top-left Y anchor coordinate. */
  float width;       /**< Bounding menu width in points. */
  float height;      /**< Bounding menu height in points. */
  int hovered_index; /**< Currently hovered row index (-1 for none). */
  struct cupertino_desktop_text_toolbar_item
      items[CUPERTINO_DESKTOP_TEXT_TOOLBAR_MAX_ITEMS]; /**< Item rows. */
  size_t item_count; /**< Number of items in menu. */
};

/**
 * @brief Creates a new macOS Desktop Text Selection Toolbar instance.
 *
 * @param engine Pointer to ui_engine instance.
 * @param desc Configuration descriptor.
 * @param out_toolbar Pointer to receive allocated toolbar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_create(
    struct ui_engine *engine,
    const struct cupertino_desktop_text_selection_toolbar_descriptor *desc,
    struct cupertino_desktop_text_selection_toolbar **out_toolbar);

/**
 * @brief Destroys a desktop text selection toolbar instance.
 *
 * @param toolbar Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_destroy(
    struct cupertino_desktop_text_selection_toolbar *toolbar);

/**
 * @brief Displays the toolbar anchored at specified screen coordinates.
 *
 * @param toolbar Target toolbar.
 * @param x Anchor X position in points.
 * @param y Anchor Y position in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_show(
    struct cupertino_desktop_text_selection_toolbar *toolbar, float x, float y);

/**
 * @brief Hides the toolbar.
 *
 * @param toolbar Target toolbar.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_hide(
    struct cupertino_desktop_text_selection_toolbar *toolbar);

/**
 * @brief Checks if the toolbar is currently visible.
 *
 * @param toolbar Target toolbar.
 * @param out_visible Pointer to receive visibility flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_is_visible(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    int *out_visible);

/**
 * @brief Sets the hover highlight index for keyboard or mouse navigation.
 *
 * @param toolbar Target toolbar.
 * @param index Row index to highlight (-1 to clear hover).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_set_hover_index(
    struct cupertino_desktop_text_selection_toolbar *toolbar, int index);

/**
 * @brief Gets the currently hovered item index.
 *
 * @param toolbar Target toolbar.
 * @param out_index Pointer to receive hovered index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_get_hover_index(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    int *out_index);

/**
 * @brief Triggers execution of a toolbar action.
 *
 * @param toolbar Target toolbar.
 * @param action_id Action identifier.
 * @param out_handled Pointer to receive 1 if handled, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_trigger_action(
    struct cupertino_desktop_text_selection_toolbar *toolbar, int action_id,
    int *out_handled);

/**
 * @brief Gets total item count in the menu.
 *
 * @param toolbar Target toolbar.
 * @param out_count Pointer to receive count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_get_item_count(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    size_t *out_count);

/**
 * @brief Gets item details by index.
 *
 * @param toolbar Target toolbar.
 * @param index Item row index.
 * @param out_title Pointer to receive title.
 * @param out_shortcut Pointer to receive shortcut string.
 * @param out_action_id Pointer to receive action ID.
 * @param out_disabled Pointer to receive disabled state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_get_item_at(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    size_t index, const char **out_title, const char **out_shortcut,
    int *out_action_id, int *out_disabled);

/**
 * @brief Gets the bounding box coordinates of the context menu.
 *
 * @param toolbar Target toolbar.
 * @param out_x Pointer to receive top-left X in points.
 * @param out_y Pointer to receive top-left Y in points.
 * @param out_w Pointer to receive menu width in points.
 * @param out_h Pointer to receive menu height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_desktop_text_selection_toolbar_get_bounds(
    const struct cupertino_desktop_text_selection_toolbar *toolbar,
    float *out_x, float *out_y, float *out_w, float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_DESKTOP_TEXT_SELECTION_TOOLBAR_H */
