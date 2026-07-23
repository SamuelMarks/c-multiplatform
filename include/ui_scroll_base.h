#ifndef UI_SCROLL_BASE_H
#define UI_SCROLL_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_gesture.h"
#include "ui_event.h"
/* clang-format on */

struct ui_scroll_base;

/**
 * @brief Callback invoked when the scroll position changes.
 */
typedef enum ui_error (*ui_scroll_on_change_t)(struct ui_scroll_base *scroll,
                                               float scroll_x, float scroll_y,
                                               void *user_data);

/**
 * @brief Creates a new unstyled scroll area base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--scroll-bg`: Background color of the scroll container.
 * - `--scroll-thumb-bg`: Background color of the scrollbar thumb.
 * - `--scroll-thumb-radius`: Border radius of the scrollbar thumb.
 * - `--scroll-track-bg`: Background color of the scrollbar track.
 * - `--scroll-padding`: Padding inside the scroll area.
 *
 * @param out_scroll Pointer to receive the allocated scroll base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_scroll_base_create(struct ui_scroll_base **out_scroll);

/**
 * @brief Destroys a scroll area component.
 *
 * @param scroll The scroll area to destroy.
 */
void ui_scroll_base_destroy(struct ui_scroll_base *scroll);

/**
 * @brief Sets the scroll position.
 *
 * @param scroll The scroll area.
 * @param x The horizontal scroll position.
 * @param y The vertical scroll position.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_scroll_base_set_scroll_pos(struct ui_scroll_base *scroll,
                                            float x, float y);

/**
 * @brief Gets the horizontal scroll position.
 *
 * @param scroll The scroll area.
 * @return The horizontal scroll position.
 */
enum ui_error ui_scroll_base_get_scroll_x(const struct ui_scroll_base *scroll,
                                          float *out_x);

/**
 * @brief Gets the vertical scroll position.
 *
 * @param scroll The scroll area.
 * @return The vertical scroll position.
 */
enum ui_error ui_scroll_base_get_scroll_y(const struct ui_scroll_base *scroll,
                                          float *out_y);

/**
 * @brief Sets the content dimensions for overflow calculations.
 *
 * @param scroll The scroll area.
 * @param width The content width.
 * @param height The content height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_scroll_base_set_content_size(struct ui_scroll_base *scroll,
                                              float width, float height);

/**
 * @brief Sets the viewport dimensions for overflow calculations.
 *
 * @param scroll The scroll area.
 * @param width The viewport width.
 * @param height The viewport height.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_scroll_base_set_viewport_size(struct ui_scroll_base *scroll,
                                               float width, float height);

/**
 * @brief Sets the change handler for the scroll area.
 *
 * @param scroll The scroll area.
 * @param on_change The callback to invoke when scrolling occurs.
 * @param user_data Opaque user data passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_scroll_base_set_on_change(struct ui_scroll_base *scroll,
                                           ui_scroll_on_change_t on_change,
                                           void *user_data);

/**
 * @brief Processes an incoming input event to trigger scrolling (e.g., mouse
 * wheel, touch drag).
 *
 * @param scroll The scroll area.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_scroll_base_process_event(struct ui_scroll_base *scroll,
                                           const struct ui_event *event,
                                           double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param scroll The scroll area.
 * @return The underlying component.
 */
enum ui_error ui_scroll_base_get_component(struct ui_scroll_base *scroll,
                                           struct ui_component **out_component);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_scroll_base_bind_data(struct ui_scroll_base *widget,
                                       struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SCROLL_BASE_H */
