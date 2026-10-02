/**
 * @file cupertino_tab_view.h
 * @brief Cupertino Tab View (CupertinoTabView) independent per-tab navigation
 * stack coordinator conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_TAB_VIEW_H
#define CUPERTINO_CUPERTINO_TAB_VIEW_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_TAB_VIEW_MAX_STACK_DEPTH 16

/**
 * @struct cupertino_tab_view_descriptor
 * @brief Configuration descriptor for initializing a Cupertino Tab View.
 */
struct cupertino_tab_view_descriptor {
  struct ui_component *root_view; /**< Root view component for this tab. */
  const char *default_title;      /**< Title of root view. */
};

/**
 * @struct cupertino_tab_view
 * @brief Independent parallel navigation coordinator instance for a tab.
 */
struct cupertino_tab_view {
  struct ui_component
      *stack[CUPERTINO_TAB_VIEW_MAX_STACK_DEPTH]; /**< View stack. */
  float
      scroll_offsets[CUPERTINO_TAB_VIEW_MAX_STACK_DEPTH]; /**< Preserved scroll
                                                             offsets. */
  char titles[CUPERTINO_TAB_VIEW_MAX_STACK_DEPTH]
             [64];    /**< Titles for each level. */
  size_t stack_depth; /**< Current depth (>= 1). */
};

/**
 * @brief Creates a new Cupertino Tab View with an initial root view.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_view Pointer to receive newly created tab view coordinator.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_create(
    struct ui_engine *engine, const struct cupertino_tab_view_descriptor *desc,
    struct cupertino_tab_view **out_view);

/**
 * @brief Destroys a Cupertino Tab View coordinator.
 *
 * @param view Coordinator instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_view_destroy(struct cupertino_tab_view *view);

/**
 * @brief Pushes a view onto the tab's navigation stack.
 *
 * @param view Target tab view coordinator.
 * @param component View component to push.
 * @param title Optional title string for this navigation level.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if maximum stack
 * depth reached, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_view_push(struct cupertino_tab_view *view,
                        struct ui_component *component, const char *title);

/**
 * @brief Pops the top view from the navigation stack.
 *
 * @param view Target tab view coordinator.
 * @param out_popped Pointer to receive popped view component (optional, can be
 * NULL).
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if already at root,
 * or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_pop(
    struct cupertino_tab_view *view, struct ui_component **out_popped);

/**
 * @brief Pops all views down to the root view.
 *
 * @param view Target tab view coordinator.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_tab_view_pop_to_root(struct cupertino_tab_view *view);

/**
 * @brief Gets current stack depth.
 *
 * @param view Target tab view coordinator.
 * @param out_depth Pointer to receive stack depth.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_get_stack_depth(
    const struct cupertino_tab_view *view, size_t *out_depth);

/**
 * @brief Gets currently visible top view on the stack.
 *
 * @param view Target tab view coordinator.
 * @param out_component Pointer to receive view component pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_get_current_view(
    const struct cupertino_tab_view *view, struct ui_component **out_component);

/**
 * @brief Gets title of currently visible top view on the stack.
 *
 * @param view Target tab view coordinator.
 * @param out_title Pointer to receive const char* pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_get_current_title(
    const struct cupertino_tab_view *view, const char **out_title);

/**
 * @brief Preserves vertical scroll offset for currently visible view.
 *
 * @param view Target tab view coordinator.
 * @param offset Vertical scroll offset in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_set_scroll_offset(
    struct cupertino_tab_view *view, float offset);

/**
 * @brief Gets preserved vertical scroll offset for currently visible view.
 *
 * @param view Target tab view coordinator.
 * @param out_offset Pointer to receive scroll offset.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_get_scroll_offset(
    const struct cupertino_tab_view *view, float *out_offset);

/**
 * @brief Handles tapping the currently active tab item:
 * - If stack_depth > 1: pops stack back to root view.
 * - If stack_depth == 1: resets preserved scroll offset to 0.0f (scroll to
 * top).
 *
 * @param view Target tab view coordinator.
 * @param out_did_scroll_to_top Pointer to receive 1 if scroll-to-top was
 * performed, 0 otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_tab_view_handle_tab_tap(
    struct cupertino_tab_view *view, int *out_did_scroll_to_top);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TAB_VIEW_H */
