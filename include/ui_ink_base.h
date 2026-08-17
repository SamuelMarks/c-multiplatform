/**
 * @file ui_ink_base.h
 * @brief Stylus and ink canvas component abstractions.
 *
 * This header defines structures and functions to manage stylus inputs,
 * storing points, and finalizing ink strokes for a canvas.
 */

#ifndef UI_INK_BASE_H
#define UI_INK_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_signal.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a single high-frequency stylus/touch event.
 */
struct ui_ink_event {
  float x;          /**< X coordinate of the stylus point. */
  float y;          /**< Y coordinate of the stylus point. */
  float pressure;   /**< Applied pressure. */
  float tilt_x;     /**< Stylus tilt along the X axis. */
  float tilt_y;     /**< Stylus tilt along the Y axis. */
  float azimuth;    /**< Stylus azimuth angle. */
  double timestamp; /**< Timestamp of the event in milliseconds. */
};

/**
 * @brief Represents an unstyled ink canvas component.
 */
struct ui_ink_base;

/**
 * @brief Creates a new unstyled ink base component.
 *
 * @param out_ink Pointer to output the initialized ink component structure.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_ink_base_create(struct ui_ink_base **out_ink);

/**
 * @brief Destroys an ink base component.
 *
 * @param ink Pointer to the ink component to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_ink_base_destroy(struct ui_ink_base *ink);

/**
 * @brief Retrieves the underlying base component from an ink structure.
 *
 * @param ink Pointer to the ink component.
 * @param out_component Pointer to receive the underlying `ui_component`.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_ink_base_get_component(struct ui_ink_base *ink,
                                     struct ui_component **out_component);

/**
 * @brief Adds a new stylus event to the current stroke.
 *
 * @param ink Pointer to the ink component.
 * @param event Pointer to the stylus event data.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_ink_base_add_event(struct ui_ink_base *ink,
                                 const struct ui_ink_event *event);

/**
 * @brief Finalizes the current stroke.
 *
 * @param ink Pointer to the ink component.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_ink_base_finish_stroke(struct ui_ink_base *ink);

/**
 * @brief Gets the number of smoothed points in the current/last stroke.
 *
 * @param ink Pointer to the ink component.
 * @param out_count Pointer to receive the point count.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_ink_base_get_smoothed_points_count(struct ui_ink_base *ink,
                                                 size_t *out_count);

/**
 * @brief Gets the smoothed point at the specified index.
 *
 * @param ink Pointer to the ink component.
 * @param index The index of the point.
 * @param out_point Pointer to receive the smoothed point data.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_ink_base_get_smoothed_point(struct ui_ink_base *ink, size_t index,
                                          struct ui_ink_event *out_point);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_INK_BASE_H */
