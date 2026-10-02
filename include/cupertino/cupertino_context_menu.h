/**
 * @file cupertino_context_menu.h
 * @brief Cupertino Context Menu (UIContextMenuInteraction) component conforming
 * to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_CONTEXT_MENU_H
#define CUPERTINO_CUPERTINO_CONTEXT_MENU_H

/* clang-format off */
#include "ui_context_menu_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;
struct ui_overlay_director;

#define CUPERTINO_CONTEXT_MENU_MAX_ACTIONS 16
#define CUPERTINO_CONTEXT_MENU_DEFAULT_WIDTH 250.0f
#define CUPERTINO_CONTEXT_MENU_ITEM_HEIGHT 44.0f

/**
 * @struct cupertino_context_menu_action
 * @brief Action item within a Cupertino context menu.
 */
struct cupertino_context_menu_action {
  char title[64];       /**< Action label text. */
  char symbol_name[64]; /**< SF Symbol glyph name (e.g. "trash", "star"). */
  int is_destructive;   /**< Non-zero for SystemRed destructive text. */
  int is_disabled;      /**< Non-zero if action is disabled. */
  int is_checked;       /**< Non-zero if item displays state checkmark. */
};

/**
 * @struct cupertino_context_menu_descriptor
 * @brief Configuration descriptor for creating a Cupertino context menu.
 */
struct cupertino_context_menu_descriptor {
  const char *title; /**< Optional menu section header title. */
  int is_dark;       /**< Non-zero for dark mode styling. */
};

/**
 * @struct cupertino_context_menu
 * @brief Cupertino Context Menu instance wrapping ui_context_menu_base.
 */
struct cupertino_context_menu {
  struct ui_context_menu_base *base; /**< CDK context menu primitive. */
  char title[128];                   /**< Header title text. */
  int is_dark;                       /**< Dark mode appearance state. */
  int is_open;                       /**< Presentation open status. */
  float target_x;                    /**< Target anchor X coordinate. */
  float target_y;                    /**< Target anchor Y coordinate. */
  size_t action_count;               /**< Number of configured actions. */
  struct cupertino_context_menu_action
      actions[CUPERTINO_CONTEXT_MENU_MAX_ACTIONS]; /**< Action items array. */
  int last_selected_index; /**< Index of last activated action (-1 if none). */
};

/**
 * @brief Creates a new Cupertino Context Menu.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_menu Pointer to receive newly created context menu.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_context_menu_create(
    struct ui_engine *engine,
    const struct cupertino_context_menu_descriptor *desc,
    struct cupertino_context_menu **out_menu);

/**
 * @brief Destroys a Cupertino Context Menu.
 *
 * @param menu Context menu instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_context_menu_destroy(struct cupertino_context_menu *menu);

/**
 * @brief Appends an action item to the context menu.
 *
 * @param menu Target context menu.
 * @param action Pointer to action item definition.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if maximum actions
 * reached, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_context_menu_add_action(
    struct cupertino_context_menu *menu,
    const struct cupertino_context_menu_action *action);

/**
 * @brief Gets number of configured actions.
 *
 * @param menu Target context menu.
 * @param out_count Pointer to receive action count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_context_menu_get_action_count(
    const struct cupertino_context_menu *menu, size_t *out_count);

/**
 * @brief Gets action item at specified index.
 *
 * @param menu Target context menu.
 * @param index 0-based index of the action.
 * @param out_action Pointer to receive action item copy.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if out of range,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_context_menu_get_action(
    const struct cupertino_context_menu *menu, size_t index,
    struct cupertino_context_menu_action *out_action);

/**
 * @brief Opens context menu anchored at specified screen coordinates.
 *
 * @param menu Target context menu.
 * @param director Overlay director to mount into (or NULL for headless mode).
 * @param x Anchor target screen X coordinate.
 * @param y Anchor target screen Y coordinate.
 * @param viewport_w Screen/window viewport width in points.
 * @param viewport_h Screen/window viewport height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_context_menu_open_at(
    struct cupertino_context_menu *menu, struct ui_overlay_director *director,
    float x, float y, float viewport_w, float viewport_h);

/**
 * @brief Closes the context menu.
 *
 * @param menu Target context menu.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_context_menu_close(struct cupertino_context_menu *menu);

/**
 * @brief Checks if the context menu is currently open.
 *
 * @param menu Target context menu.
 * @param out_is_open Pointer to receive open flag (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_context_menu_is_open(
    const struct cupertino_context_menu *menu, int *out_is_open);

/**
 * @brief Simulates selecting an action item by index, recording selection and
 * closing menu.
 *
 * @param menu Target context menu.
 * @param index 0-based index of the action.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index invalid,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_context_menu_select_action(
    struct cupertino_context_menu *menu, size_t index);

/**
 * @brief Gets index of last activated action item.
 *
 * @param menu Target context menu.
 * @param out_index Pointer to receive index (-1 if none selected).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_context_menu_get_last_selected_index(
    const struct cupertino_context_menu *menu, int *out_index);

/**
 * @brief Retrieves underlying CDK context menu base primitive.
 *
 * @param menu Target context menu.
 * @param out_base Pointer to receive ui_context_menu_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_context_menu_get_base(struct cupertino_context_menu *menu,
                                struct ui_context_menu_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_CONTEXT_MENU_H */
