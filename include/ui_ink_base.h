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
  float x;
  float y;
  float pressure;
  float tilt_x;
  float tilt_y;
  float azimuth;
  double timestamp;
};

/**
 * @brief Represents an unstyled ink canvas component.
 */
struct ui_ink_base;

/**
 * @brief Creates a new unstyled ink base component.
 *
 * @param out_ink Pointer to output the initialized ink component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_ink_base_create(struct ui_ink_base **out_ink);

/**
 * @brief Destroys an ink base component.
 *
 * @param ink The ink component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_ink_base_destroy(struct ui_ink_base *ink);

/**
 * @brief Retrieves the base component.
 *
 * @param ink The ink component.
 * @return The base component.
 */
enum ui_error ui_ink_base_get_component(struct ui_ink_base *ink,
                                        struct ui_component **out_component);

/**
 * @brief Adds a new stylus event to the current stroke.
 *
 * @param ink The ink component.
 * @param event The stylus event data.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_ink_base_add_event(struct ui_ink_base *ink,
                                    const struct ui_ink_event *event);

/**
 * @brief Finalizes the current stroke.
 *
 * @param ink The ink component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_ink_base_finish_stroke(struct ui_ink_base *ink);

/**
 * @brief Gets the number of smoothed points in the current/last stroke.
 *
 * @param ink The ink component.
 * @param out_count Pointer to receive the count.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_ink_base_get_smoothed_points_count(struct ui_ink_base *ink,
                                                    size_t *out_count);

/**
 * @brief Gets the smoothed point at the specified index.
 *
 * @param ink The ink component.
 * @param index The index.
 * @param out_point Pointer to receive the point.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_ink_base_get_smoothed_point(struct ui_ink_base *ink,
                                             size_t index,
                                             struct ui_ink_event *out_point);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_INK_BASE_H */
