/**
 * @file ui_layout_observer.h
 * @brief Layout breakpoint and resize observer.
 */

#ifndef UI_LAYOUT_OBSERVER_H
#define UI_LAYOUT_OBSERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

struct ui_layout_observer;

/**
 * @brief Callback invoked when a layout breakpoint's state changes.
 *
 * @param observer The layout observer.
 * @param breakpoint_id The ID of the breakpoint that changed.
 * @param is_active 1 if the breakpoint just became active, 0 if it became
 * inactive.
 * @param user_data Opaque user data.
 */
typedef ui_error_t (*ui_layout_observer_cb_t)(
    struct ui_layout_observer *observer, int breakpoint_id, int is_active,
    void *user_data);

/**
 * @brief Creates a new layout breakpoint observer.
 *
 * @param out_observer Pointer to receive the created observer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_layout_observer_create(struct ui_layout_observer **out_observer);

/**
 * @brief Destroys a layout breakpoint observer.
 *
 * @param observer The observer to destroy.
 */
ui_error_t ui_layout_observer_destroy(struct ui_layout_observer *observer);

/**
 * @brief Adds a new breakpoint constraint to the observer.
 * Use -1 for bounds you do not want to constrain (e.g., max_width = -1 means
 * "no maximum width").
 *
 * @param observer The observer.
 * @param min_width Minimum width (inclusive), or -1.
 * @param max_width Maximum width (inclusive), or -1.
 * @param min_height Minimum height (inclusive), or -1.
 * @param max_height Maximum height (inclusive), or -1.
 * @param out_breakpoint_id Pointer to receive the generated breakpoint ID.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_layout_observer_add_breakpoint(struct ui_layout_observer *observer,
                                  int min_width, int max_width, int min_height,
                                  int max_height, int *out_breakpoint_id);

/**
 * @brief Subscribes a callback to be notified of breakpoint state changes.
 *
 * @param observer The observer.
 * @param callback The function to call when a breakpoint state changes.
 * @param user_data Opaque data to pass to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_layout_observer_subscribe(struct ui_layout_observer *observer,
                                        ui_layout_observer_cb_t callback,
                                        void *user_data);

/**
 * @brief Notifies the observer of a window resize event, evaluating all
 * breakpoints.
 *
 * @param observer The observer.
 * @param width The new window width.
 * @param height The new window height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_layout_observer_notify_resize(struct ui_layout_observer *observer,
                                            int width, int height);

/**
 * @brief Checks if a specific breakpoint is currently active based on the last
 * known size.
 *
 * @param observer The observer.
 * @param breakpoint_id The breakpoint ID to query.
 * @param out_is_active Pointer to receive 1 if active, 0 if inactive or invalid
 * ID.
 * @return ui_error_t `UI_ERROR_NONE` on success.
 */
ui_error_t
ui_layout_observer_is_active(const struct ui_layout_observer *observer,
                             int breakpoint_id, int *out_is_active);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_LAYOUT_OBSERVER_H */
