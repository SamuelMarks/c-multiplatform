/**
 * @file ui_popover_base.h
 * @brief Popover component providing anchored, floating contextual content.
 */

#ifndef UI_POPOVER_BASE_H
#define UI_POPOVER_BASE_H
struct ui_computed;
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_geometry_anchor.h"
#include "ui_event.h"
/* clang-format on */

struct ui_popover_base;
struct ui_dom_node;
struct ui_overlay_director;
struct ui_focus_manager;

/**
 * @brief Creates a new popover base component.
 *
 * @param out_popover Pointer to receive the allocated popover base.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_create(struct ui_popover_base **out_popover);

/**
 * @brief Destroys a popover component.
 *
 * @param popover The popover to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_destroy(struct ui_popover_base *popover);

/**
 * @brief Opens the popover, rendering it into the overlay director and pushing
 * a focus trap.
 *
 * @param popover The popover.
 * @param content The DOM node representing the inner popover content.
 * @param director The overlay director handling global positioning.
 * @param focus_mgr The focus manager for trapping focus.
 * @param trigger_layout Target trigger's layout for computing anchor position.
 * @param anchor_config Rules for anchoring the popover to the trigger.
 * @param viewport_width Total width of the screen/viewport for collision math.
 * @param viewport_height Total height of the screen/viewport for collision
 * math.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_open(struct ui_popover_base *popover,
                                struct ui_dom_node *content,
                                struct ui_overlay_director *director,
                                struct ui_focus_manager *focus_mgr,
                                const struct ui_layout_node *trigger_layout,
                                const struct ui_anchor_config *anchor_config,
                                float viewport_width, float viewport_height);

/**
 * @brief Processes an input event to detect if the popover should be dismissed
 * (e.g., click-outside).
 *
 * @param popover The popover.
 * @param event The input event.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_process_event(struct ui_popover_base *popover,
                                         const struct ui_event *event);

/**
 * @brief Closes the popover, unmounting it from the overlay director and
 * popping the focus trap.
 *
 * @param popover The popover.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_close(struct ui_popover_base *popover);

/**
 * @brief Checks if the popover is currently open.
 *
 * @param popover The popover.
 * @return 1 if open, 0 if closed.
 */
ui_error_t ui_popover_base_is_open(const struct ui_popover_base *popover,
                                   int *out_is_open);

/**
 * @brief Binds the open state to a signal.
 *
 * @param widget The widget.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_popover_base_bind_open(struct ui_popover_base *widget,
                                     struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is animating.
 *
 * @param widget The widget.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_popover_base_get_animating_signal(struct ui_popover_base *widget,
                                     struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_POPOVER_BASE_H */
