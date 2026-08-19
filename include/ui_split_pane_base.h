/**
 * @file ui_split_pane_base.h
 * @brief Split pane base declarations.
 *
 * @defgroup SplitPaneBase Split Pane Base
 * @brief Base implementation for resizable split pane components.
 * @{
 */

#ifndef UI_SPLIT_PANE_BASE_H
#define UI_SPLIT_PANE_BASE_H

/**
 * @brief Opaque structure for a computed signal.
 */
struct ui_computed;

/**
 * @brief Opaque structure for a signal.
 */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
/* clang-format on */

/**
 * @brief Split pane orientations.
 */
enum ui_split_pane_orientation {
  UI_SPLIT_PANE_ORIENTATION_HORIZONTAL, /**< Children placed left and right.
                                           Resizer moves horizontally. */
  UI_SPLIT_PANE_ORIENTATION_VERTICAL    /**< Children placed top and bottom.
                                           Resizer moves vertically. */
};

/**
 * @brief Opaque structure representing the split pane base.
 */
struct ui_split_pane_base;

/**
 * @brief Creates a new unstyled split pane base component.
 *
 * @param out_split_pane Pointer to receive the allocated split pane base.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
ui_error_t
ui_split_pane_base_create(struct ui_split_pane_base **out_split_pane);

/**
 * @brief Destroys a split pane base component.
 *
 * @param split_pane The split pane to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_pane_base_destroy(struct ui_split_pane_base *split_pane);

/**
 * @brief Sets the orientation of the split pane.
 *
 * @param split_pane The split pane.
 * @param orientation The orientation to set.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_split_pane_base_set_orientation(struct ui_split_pane_base *split_pane,
                                   enum ui_split_pane_orientation orientation);

/**
 * @brief Gets the orientation of the split pane.
 *
 * @param split_pane The split pane.
 * @param out_orientation Pointer to receive the orientation.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_split_pane_base_get_orientation(
    const struct ui_split_pane_base *split_pane,
    enum ui_split_pane_orientation *out_orientation);

/**
 * @brief Sets the split position (e.g., width of the first pane if horizontal).
 *
 * @param split_pane The split pane.
 * @param position The position in pixels.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_split_pane_base_set_position(struct ui_split_pane_base *split_pane,
                                int position);

/**
 * @brief Gets the split position.
 *
 * @param split_pane The split pane.
 * @param out_position Pointer to receive the position.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_split_pane_base_get_position(const struct ui_split_pane_base *split_pane,
                                int *out_position);

/**
 * @brief Sets the minimum and maximum boundaries for the resizer.
 *
 * @param split_pane The split pane.
 * @param min_position The minimum allowed position.
 * @param max_position The maximum allowed position.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_split_pane_base_set_bounds(struct ui_split_pane_base *split_pane,
                                         int min_position, int max_position);

/**
 * @brief Processes an input event for dragging the resizer.
 *
 * Calculates boundary constraints and updates the position if dragging.
 *
 * @param split_pane The split pane.
 * @param event The input event.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointers.
 */
ui_error_t
ui_split_pane_base_process_event(struct ui_split_pane_base *split_pane,
                                 const struct ui_event *event);

/**
 * @brief Binds the data property.
 *
 * @param widget The split pane widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_split_pane_base_bind_data(struct ui_split_pane_base *widget,
                                        struct ui_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_SPLIT_PANE_BASE_H */

/** @} */
