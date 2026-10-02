/**
 * @file cupertino_text_selection_toolbar.h
 * @brief Cupertino iOS Text Selection Toolbar and Grabber Handles conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_TEXT_SELECTION_TOOLBAR_H
#define CUPERTINO_CUPERTINO_TEXT_SELECTION_TOOLBAR_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_TEXT_TOOLBAR_ITEM_WIDTH 54.0f
#define CUPERTINO_TEXT_TOOLBAR_HEIGHT 44.0f
#define CUPERTINO_TEXT_TOOLBAR_ARROW_HEIGHT 8.0f
#define CUPERTINO_TEXT_TOOLBAR_PAGE_SIZE 4
#define CUPERTINO_TEXT_HANDLE_RADIUS 5.0f
#define CUPERTINO_TEXT_HANDLE_BAR_HEIGHT 22.0f

/**
 * @enum cupertino_text_action
 * @brief Text editing toolbar action identifiers.
 */
enum cupertino_text_action {
  CUPERTINO_TEXT_ACTION_CUT = 0,    /**< Cut selected text. */
  CUPERTINO_TEXT_ACTION_COPY,       /**< Copy selected text. */
  CUPERTINO_TEXT_ACTION_PASTE,      /**< Paste clipboard text. */
  CUPERTINO_TEXT_ACTION_SELECT_ALL, /**< Select all text in field. */
  CUPERTINO_TEXT_ACTION_SHARE,      /**< Share selected text. */
  CUPERTINO_TEXT_ACTION_LOOK_UP,    /**< Dictionary / Web lookup. */
  CUPERTINO_TEXT_ACTION_COUNT
};

/**
 * @struct cupertino_text_selection_toolbar_descriptor
 * @brief Configuration descriptor for text selection toolbar.
 */
struct cupertino_text_selection_toolbar_descriptor {
  int allowed_actions_mask; /**< Bitmask of 1 << cupertino_text_action (0 for
                               all). */
};

/**
 * @struct cupertino_text_selection_toolbar
 * @brief Text selection toolbar instance.
 */
struct cupertino_text_selection_toolbar {
  int is_visible;
  int current_page;
  int total_pages;
  float anchor_x;
  float anchor_y;
  float selection_w;
  float selection_h;
  float bubble_x;
  float bubble_y;
  float bubble_w;
  float bubble_h;
  int arrow_on_bottom; /**< 1 if arrow points down towards text, 0 if up. */
  int action_count;
  enum cupertino_text_action actions[CUPERTINO_TEXT_ACTION_COUNT];
};

/**
 * @brief Creates a new Cupertino text selection toolbar.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_toolbar Pointer to receive newly created toolbar instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_create(
    struct ui_engine *engine,
    const struct cupertino_text_selection_toolbar_descriptor *desc,
    struct cupertino_text_selection_toolbar **out_toolbar);

/**
 * @brief Destroys a Cupertino text selection toolbar.
 *
 * @param toolbar Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_destroy(
    struct cupertino_text_selection_toolbar *toolbar);

/**
 * @brief Shows toolbar anchored to a text selection box.
 *
 * @param toolbar Target toolbar.
 * @param anchor_x Top-left X coordinate of selection.
 * @param anchor_y Top-left Y coordinate of selection.
 * @param selection_w Width of selected text bounding box.
 * @param selection_h Height of selected text bounding box.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_selection_toolbar_show(
    struct cupertino_text_selection_toolbar *toolbar, float anchor_x,
    float anchor_y, float selection_w, float selection_h);

/**
 * @brief Hides the toolbar.
 *
 * @param toolbar Target toolbar.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_text_selection_toolbar_hide(
    struct cupertino_text_selection_toolbar *toolbar);

/**
 * @brief Checks if toolbar is currently visible.
 *
 * @param toolbar Target toolbar.
 * @param out_visible Pointer to receive visibility flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_is_visible(
    const struct cupertino_text_selection_toolbar *toolbar, int *out_visible);

/**
 * @brief Gets current pagination state.
 *
 * @param toolbar Target toolbar.
 * @param out_page Pointer to receive active page index (0-based).
 * @param out_total_pages Pointer to receive total pages.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_get_page(
    const struct cupertino_text_selection_toolbar *toolbar, int *out_page,
    int *out_total_pages);

/**
 * @brief Navigates between pagination pages.
 *
 * @param toolbar Target toolbar.
 * @param forward 1 to advance page, 0 to go back.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_paginate(
    struct cupertino_text_selection_toolbar *toolbar, int forward);

/**
 * @brief Triggers an action from the toolbar.
 *
 * @param toolbar Target toolbar.
 * @param action Action identifier.
 * @param out_handled Pointer to receive 1 if action supported, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_trigger_action(
    struct cupertino_text_selection_toolbar *toolbar,
    enum cupertino_text_action action, int *out_handled);

/**
 * @brief Gets bounding coordinates of the toolbar bubble.
 *
 * @param toolbar Target toolbar.
 * @param out_x Pointer to receive top-left X coordinate.
 * @param out_y Pointer to receive top-left Y coordinate.
 * @param out_w Pointer to receive bounding width.
 * @param out_h Pointer to receive bounding height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_get_bounds(
    const struct cupertino_text_selection_toolbar *toolbar, float *out_x,
    float *out_y, float *out_w, float *out_h);

/**
 * @brief Computes geometry of selection teardrop handles.
 *
 * @param toolbar Target toolbar.
 * @param is_start_handle 1 for start handle, 0 for end handle.
 * @param out_x Pointer to receive handle circle center X coordinate.
 * @param out_y Pointer to receive handle circle center Y coordinate.
 * @param out_w Pointer to receive handle width.
 * @param out_h Pointer to receive handle height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_text_selection_toolbar_get_handle_bounds(
    const struct cupertino_text_selection_toolbar *toolbar, int is_start_handle,
    float *out_x, float *out_y, float *out_w, float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TEXT_SELECTION_TOOLBAR_H */
