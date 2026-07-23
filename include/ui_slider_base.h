#ifndef UI_SLIDER_BASE_H
#define UI_SLIDER_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_gesture.h"
#include "ui_event.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

struct ui_slider_base;

/**
 * @brief Callback invoked when the slider value changes.
 */
typedef enum ui_error (*ui_slider_on_change_t)(struct ui_slider_base *slider,
                                               float value, void *user_data);

/**
 * @brief Creates a new unstyled slider base component.
 *
 * CSS Custom Properties (Variable Hooks):
 * - `--slider-track-bg`: Background color of the slider track.
 * - `--slider-track-height`: Height/thickness of the track.
 * - `--slider-thumb-bg`: Background color of the draggable thumb.
 * - `--slider-thumb-size`: Width/height of the thumb.
 * - `--slider-thumb-radius`: Border radius of the thumb.
 * - `--slider-fill-bg`: Background color of the filled portion of the track.
 * - `--slider-disabled-opacity`: Opacity of the slider when disabled.
 *
 * @param out_slider Pointer to receive the allocated slider base.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_slider_base_create(struct ui_slider_base **out_slider,
                                    struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a slider base component.
 *
 * @param slider The slider to destroy.
 */
void ui_slider_base_destroy(struct ui_slider_base *slider);

/**
 * @brief Sets the minimum value of the slider.
 *
 * @param slider The slider.
 * @param min The minimum value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_set_min(struct ui_slider_base *slider, float min);

/**
 * @brief Sets the maximum value of the slider.
 *
 * @param slider The slider.
 * @param max The maximum value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_set_max(struct ui_slider_base *slider, float max);

/**
 * @brief Sets the current value of the slider.
 *
 * @param slider The slider.
 * @param value The current value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_set_value(struct ui_slider_base *slider,
                                       float value);

/**
 * @brief Gets the current value of the slider.
 *
 * @param slider The slider.
 * @return The current value.
 */
enum ui_error ui_slider_base_get_value(const struct ui_slider_base *slider,
                                       float *out_value);

/**
 * @brief Sets the step increment. If 0.0, the slider is continuous.
 *
 * @param slider The slider.
 * @param step The step increment.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_set_step(struct ui_slider_base *slider,
                                      float step);

/**
 * @brief Sets the disabled state of the slider.
 * Updates ARIA attributes and prevents state changes.
 *
 * @param slider The slider.
 * @param disabled 1 to disable, 0 to enable.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_set_disabled(struct ui_slider_base *slider,
                                          int disabled);

/**
 * @brief Sets the change handler for the slider.
 *
 * @param slider The slider.
 * @param on_change The callback to invoke when the value changes.
 * @param user_data Opaque user data passed to the callback.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_set_on_change(struct ui_slider_base *slider,
                                           ui_slider_on_change_t on_change,
                                           void *user_data);

/**
 * @brief Processes an incoming input event to trigger slider interactions.
 * Note: To correctly process dragging, the slider needs layout information to
 * map coordinates. For this base structural component, we expose a direct
 * normalization function instead, or allow an event to set the normalized value
 * directly (0.0 to 1.0).
 *
 * @param slider The slider.
 * @param normalized_position A float from 0.0 to 1.0 representing the pointer's
 * position on the track.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_set_normalized_value(struct ui_slider_base *slider,
                                                  float normalized_position);

/**
 * @brief Processes an incoming input event (e.g., keyboard interactions like
 * Arrow Keys).
 *
 * @param slider The slider.
 * @param event The input event.
 * @param timestamp_ms Current time in milliseconds.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_slider_base_process_event(struct ui_slider_base *slider,
                                           const struct ui_event *event,
                                           double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param slider The slider.
 * @return The underlying component.
 */
enum ui_error ui_slider_base_get_component(struct ui_slider_base *slider,
                                           struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SLIDER_BASE_H */
