/**
 * @file ui_floating_toolbar_base.h
 * @brief Base unstyled floating toolbar component providing layout, anchoring,
 * and state machine.
 */

#ifndef UI_FLOATING_TOOLBAR_BASE_H
#define UI_FLOATING_TOOLBAR_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum ui_floating_toolbar_orientation
 * @brief Orientation alignment of the floating toolbar.
 */
enum ui_floating_toolbar_orientation {
  /** @brief Horizontal floating toolbar layout. */
  UI_FLOATING_TOOLBAR_HORIZONTAL = 0,
  /** @brief Vertical floating toolbar layout. */
  UI_FLOATING_TOOLBAR_VERTICAL = 1
};

/**
 * @enum ui_floating_toolbar_state
 * @brief Expansion state of the floating toolbar.
 */
enum ui_floating_toolbar_state {
  /** @brief Collapsed compact state (typically icon or single FAB only). */
  UI_FLOATING_TOOLBAR_COLLAPSED = 0,
  /** @brief Fully expanded toolbar with all action slots visible. */
  UI_FLOATING_TOOLBAR_EXPANDED = 1
};

/**
 * @brief Opaque handle representing the base floating toolbar component.
 */
struct ui_floating_toolbar_base;

/**
 * @brief Creates a new unstyled floating toolbar base component.
 *
 * @param out_toolbar Pointer receiving the allocated floating toolbar instance.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails,
 *         or UI_ERROR_INVALID_ARGUMENT if out_toolbar is null.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_create(struct ui_floating_toolbar_base **out_toolbar);

/**
 * @brief Destroys a floating toolbar base instance and frees all internal
 * allocations.
 *
 * @param toolbar The floating toolbar instance to destroy. If null, does
 * nothing.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_destroy(struct ui_floating_toolbar_base *toolbar);

/**
 * @brief Retrieves the underlying ui_component for DOM mounting and styling.
 *
 * @param toolbar Floating toolbar instance.
 * @param out_component Pointer receiving the underlying ui_component handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_get_component(struct ui_floating_toolbar_base *toolbar,
                                       struct ui_component **out_component);

/**
 * @brief Sets the layout orientation of the floating toolbar.
 *
 * @param toolbar Floating toolbar instance.
 * @param orientation Horizontal or vertical orientation.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_set_orientation(
    struct ui_floating_toolbar_base *toolbar,
    enum ui_floating_toolbar_orientation orientation);

/**
 * @brief Retrieves the current layout orientation of the floating toolbar.
 *
 * @param toolbar Floating toolbar instance.
 * @param out_orientation Pointer receiving the current orientation.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_get_orientation(
    const struct ui_floating_toolbar_base *toolbar,
    enum ui_floating_toolbar_orientation *out_orientation);

/**
 * @brief Sets the expansion state of the floating toolbar.
 *
 * @param toolbar Floating toolbar instance.
 * @param state Collapsed or expanded state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_set_state(struct ui_floating_toolbar_base *toolbar,
                                   enum ui_floating_toolbar_state state);

/**
 * @brief Retrieves the current expansion state of the floating toolbar.
 *
 * @param toolbar Floating toolbar instance.
 * @param out_state Pointer receiving the current state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_floating_toolbar_base_get_state(
    const struct ui_floating_toolbar_base *toolbar,
    enum ui_floating_toolbar_state *out_state);

/**
 * @brief Sets or removes the primary floating action button (FAB) component
 * slot.
 *
 * @param toolbar Floating toolbar instance.
 * @param fab_component Component to mount into the primary FAB slot (or NULL to
 * clear).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_set_fab_slot(struct ui_floating_toolbar_base *toolbar,
                                      struct ui_component *fab_component);

/**
 * @brief Appends a secondary action component item into the floating toolbar.
 *
 * @param toolbar Floating toolbar instance.
 * @param action_component Action item component to append.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_append_action(struct ui_floating_toolbar_base *toolbar,
                                       struct ui_component *action_component);

/**
 * @brief Retrieves the total number of action components currently attached.
 *
 * @param toolbar Floating toolbar instance.
 * @param out_count Pointer receiving total action count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_floating_toolbar_base_get_action_count(
    const struct ui_floating_toolbar_base *toolbar, size_t *out_count);

#ifdef __cplusplus
}
#endif

#endif /* UI_FLOATING_TOOLBAR_BASE_H */
