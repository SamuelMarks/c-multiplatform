#ifndef UI_BACKDROP_H
#define UI_BACKDROP_H

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque backdrop context.
 */
struct ui_backdrop;

/**
 * @brief Creates a new backdrop manager.
 * @param out_backdrop Pointer to receive the allocated backdrop.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_backdrop_create(struct ui_backdrop **out_backdrop);

/**
 * @brief Destroys a backdrop manager.
 * @param backdrop The backdrop to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_backdrop_destroy(struct ui_backdrop *backdrop);

/**
 * @brief Sets whether the backdrop is active.
 * @param backdrop The backdrop manager.
 * @param is_active 1 if active, 0 if inactive.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_backdrop_set_active(struct ui_backdrop *backdrop, int is_active);

/**
 * @brief Processes an input event to detect if the backdrop should dismiss the
 * overlay. Dismissal occurs if the user presses Escape, or clicks/taps outside
 * the provided content bounding box.
 * @param backdrop The backdrop manager.
 * @param event The event to process.
 * @param content_x The X coordinate of the overlay content bounds.
 * @param content_y The Y coordinate of the overlay content bounds.
 * @param content_width The width of the overlay content.
 * @param content_height The height of the overlay content.
 * @param out_should_dismiss Pointer to a boolean set to 1 if dismissal is
 * triggered, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_backdrop_process_event(struct ui_backdrop *backdrop,
                                     const struct ui_event *event,
                                     float content_x, float content_y,
                                     float content_width, float content_height,
                                     int *out_should_dismiss);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BACKDROP_H */
