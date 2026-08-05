#ifndef UI_TOOLTIP_BASE_H
#define UI_TOOLTIP_BASE_H
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

struct ui_tooltip_base;
struct ui_dom_node;
struct ui_overlay_director;

/**
 * @brief Configuration for a tooltip interaction state machine.
 */
struct ui_tooltip_config {
  double hover_delay_secs;
  double focus_delay_secs;
  double touch_hold_delay_secs;
  double hide_delay_secs;
};

/**
 * @brief Creates a new tooltip base component.
 *
 * @param out_tooltip Pointer to receive the allocated tooltip base.
 * @param config Configuration for the interaction delays.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tooltip_base_create(struct ui_tooltip_base **out_tooltip,
                                  const struct ui_tooltip_config *config);

/**
 * @brief Destroys a tooltip component.
 *
 * @param tooltip The tooltip to destroy.
 */
ui_error_t ui_tooltip_base_destroy(struct ui_tooltip_base *tooltip);

/**
 * @brief Sets the text content for the tooltip.
 *
 * @param tooltip The tooltip.
 * @param text The text content.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tooltip_base_set_text(struct ui_tooltip_base *tooltip,
                                    const char *text);

/**
 * @brief Handles input events to drive the tooltip state machine.
 *
 * @param tooltip The tooltip.
 * @param event The input event (mouse hover, touch start/end, focus, etc.).
 * @param current_time_secs Current monotonic time in seconds, used for
 * evaluating delays.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tooltip_base_handle_event(struct ui_tooltip_base *tooltip,
                                        const struct ui_event *event,
                                        double current_time_secs);

/**
 * @brief Performs periodic checks for state transitions (e.g., triggering
 * show/hide after delay).
 *
 * @param tooltip The tooltip.
 * @param current_time_secs Current monotonic time in seconds.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tooltip_base_tick(struct ui_tooltip_base *tooltip,
                                double current_time_secs);

/**
 * @brief Checks if the tooltip is currently logically visible (after delays
 * have passed).
 *
 * @param tooltip The tooltip.
 * @return 1 if visible, 0 if hidden.
 */
ui_error_t ui_tooltip_base_is_visible(const struct ui_tooltip_base *tooltip,
                                      int *out_is_visible);

/**
 * @brief Programmatically forces the tooltip to hide, overriding delays.
 *
 * @param tooltip The tooltip.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tooltip_base_hide(struct ui_tooltip_base *tooltip);

/**
 * @brief Mounts the tooltip visually inside the overlay director if it is
 * logically visible.
 *
 * Should be called after tick() evaluates state transitions.
 *
 * @param tooltip The tooltip.
 * @param director The overlay director handling global positioning.
 * @param trigger_layout Target trigger's layout for computing anchor position.
 * @param anchor_config Rules for anchoring the tooltip to the trigger.
 * @param viewport_width Total width of the screen/viewport for collision math.
 * @param viewport_height Total height of the screen/viewport for collision
 * math.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tooltip_base_render(struct ui_tooltip_base *tooltip,
                                  struct ui_overlay_director *director,
                                  const struct ui_layout_node *trigger_layout,
                                  const struct ui_anchor_config *anchor_config,
                                  float viewport_width, float viewport_height);

/**
 * @brief Binds the open state to a signal.
 *
 * @param widget The widget.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_tooltip_base_bind_open(struct ui_tooltip_base *widget,
                                     struct ui_signal *open_signal);

/**
 * @brief Retrieves the computed signal indicating if the widget is animating.
 *
 * @param widget The widget.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_tooltip_base_get_animating_signal(struct ui_tooltip_base *widget,
                                     struct ui_computed **out_animating);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TOOLTIP_BASE_H */
