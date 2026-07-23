#ifndef UI_FOCUS_MANAGER_H
#define UI_FOCUS_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ui_types.h"

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_layout.h"
/* clang-format on */

/**
 * @brief Direction for spatial navigation (D-pad/arrow keys).
 */
enum ui_focus_direction {
  UI_FOCUS_DIRECTION_UP = 0,
  UI_FOCUS_DIRECTION_DOWN,
  UI_FOCUS_DIRECTION_LEFT,
  UI_FOCUS_DIRECTION_RIGHT
};

struct ui_focus_manager;

/**
 * @brief Creates a focus manager instance.
 *
 * @param out_manager Pointer to receive the new focus manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_focus_manager_create(struct ui_focus_manager **out_manager);

/**
 * @brief Destroys a focus manager instance.
 *
 * @param manager The focus manager.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_focus_manager_destroy(struct ui_focus_manager *manager);

/**
 * @brief Requests focus for a specific DOM node.
 *
 * @param manager The focus manager.
 * @param node The DOM node to focus.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_focus_manager_request_focus(struct ui_focus_manager *manager,
                                             struct ui_dom_node *node);

/**
 * @brief Gets the currently focused DOM node.
 *
 * @param manager The focus manager.
 * @param out_node Pointer to receive the focused DOM node.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_focus_manager_get_focused_node(const struct ui_focus_manager *manager,
                                  struct ui_dom_node **out_node);

/**
 * @brief Advances focus to the next/previous focusable node (Tab indexing).
 *
 * @param manager The focus manager.
 * @param root The root DOM node to search within.
 * @param forward 1 for forward (Tab), 0 for backward (Shift+Tab).
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_focus_manager_advance(struct ui_focus_manager *manager,
                                       struct ui_dom_node *root, int forward);

/**
 * @brief Moves focus spatially based on geometry (D-pad or arrow keys).
 *
 * @param manager The focus manager.
 * @param layout_root The layout tree root containing geometry.
 * @param direction The direction to navigate.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_focus_manager_navigate(struct ui_focus_manager *manager,
                                        struct ui_layout_node *layout_root,
                                        enum ui_focus_direction direction);

/**
 * @brief Pushes a focus trap (e.g., a Dialog). All focus movement is
 * constrained to this subtree.
 *
 * @param manager The focus manager.
 * @param trap_root The root DOM node of the trap.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_focus_manager_push_trap(struct ui_focus_manager *manager,
                                         struct ui_dom_node *trap_root);

/**
 * @brief Pops the top-most focus trap and restores focus to the previously
 * focused node.
 *
 * @param manager The focus manager.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_focus_manager_pop_trap(struct ui_focus_manager *manager);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FOCUS_MANAGER_H */
