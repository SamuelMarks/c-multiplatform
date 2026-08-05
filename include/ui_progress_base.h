#ifndef UI_PROGRESS_BASE_H
#define UI_PROGRESS_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
/* clang-format on */

struct ui_progress_base;

/**
 * @brief Creates a new progress base component.
 *
 * @param out_progress Pointer to receive the allocated progress component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_progress_base_create(struct ui_progress_base **out_progress);

/**
 * @brief Destroys a progress component.
 *
 * @param progress The progress component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_progress_base_destroy(struct ui_progress_base *progress);

/**
 * @brief Sets the component to determinate mode and updates the value.
 *
 * @param progress The progress component.
 * @param value The progress value (will be clamped between min and max).
 * @param min The minimum possible value (e.g., 0.0f).
 * @param max The maximum possible value (e.g., 100.0f).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_progress_base_set_determinate(struct ui_progress_base *progress,
                                            float value, float min, float max);

/**
 * @brief Sets the component to indeterminate mode.
 *
 * @param progress The progress component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_progress_base_set_indeterminate(struct ui_progress_base *progress);

/**
 * @brief Retrieves the underlying UI component.
 *
 * @param progress The progress component.
 * @return The underlying component, or NULL if invalid.
 */
ui_error_t ui_progress_base_get_component(struct ui_progress_base *progress,
                                          struct ui_component **out_component);

/**
 * @brief Gets the current normalized percentage [0.0, 1.0].
 * If the progress is indeterminate, this returns 0.0.
 *
 * @param progress The progress component.
 * @return The normalized percentage.
 */
ui_error_t ui_progress_base_get_normalized_percentage(
    const struct ui_progress_base *progress, float *out_percentage);

/**
 * @brief Checks if the progress is currently indeterminate.
 *
 * @param progress The progress component.
 * @return 1 if indeterminate, 0 if determinate.
 */
ui_error_t
ui_progress_base_is_indeterminate(const struct ui_progress_base *progress,
                                  int *out_is_indeterminate);

/**
 * @brief Binds the value property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_progress_base_bind_value(struct ui_progress_base *widget,
                                       struct ui_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_PROGRESS_BASE_H */
