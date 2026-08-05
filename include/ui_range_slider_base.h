#ifndef UI_RANGE_SLIDER_BASE_H
#define UI_RANGE_SLIDER_BASE_H

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

struct ui_range_slider_base;

/**
 * @brief Which thumb is active or being interacted with.
 */
enum ui_range_slider_thumb {
  UI_RANGE_SLIDER_THUMB_NONE,
  UI_RANGE_SLIDER_THUMB_LOW,
  UI_RANGE_SLIDER_THUMB_HIGH
};

/**
 * @brief Callback invoked when the range slider values change.
 */
typedef ui_error_t (*ui_range_slider_on_change_t)(
    struct ui_range_slider_base *slider, float low_value, float high_value,
    void *user_data);

/**
 * @brief Creates a new unstyled range slider base component.
 *
 * @param out_slider Pointer to receive the allocated range slider base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_create(struct ui_range_slider_base **out_slider);

/**
 * @brief Destroys a range slider base component.
 *
 * @param slider The range slider to destroy.
 */
ui_error_t ui_range_slider_base_destroy(struct ui_range_slider_base *slider);

/**
 * @brief Sets the minimum value of the range slider.
 */
ui_error_t ui_range_slider_base_set_min(struct ui_range_slider_base *slider,
                                        float min);

/**
 * @brief Sets the maximum value of the range slider.
 */
ui_error_t ui_range_slider_base_set_max(struct ui_range_slider_base *slider,
                                        float max);

/**
 * @brief Sets the current values of the range slider.
 */
ui_error_t ui_range_slider_base_set_values(struct ui_range_slider_base *slider,
                                           float low_value, float high_value);

/**
 * @brief Gets the current values of the range slider.
 */
ui_error_t
ui_range_slider_base_get_values(const struct ui_range_slider_base *slider,
                                float *out_low, float *out_high);

/**
 * @brief Sets the step increment. If 0.0, the slider is continuous.
 */
ui_error_t ui_range_slider_base_set_step(struct ui_range_slider_base *slider,
                                         float step);

/**
 * @brief Sets the disabled state of the range slider.
 */
ui_error_t
ui_range_slider_base_set_disabled(struct ui_range_slider_base *slider,
                                  int disabled);

/**
 * @brief Sets the change handler for the range slider.
 */
ui_error_t
ui_range_slider_base_set_on_change(struct ui_range_slider_base *slider,
                                   ui_range_slider_on_change_t on_change,
                                   void *user_data);

/**
 * @brief Processes an incoming input event to trigger slider interactions.
 */
ui_error_t
ui_range_slider_base_set_normalized_value(struct ui_range_slider_base *slider,
                                          enum ui_range_slider_thumb thumb,
                                          float normalized_position);

/**
 * @brief Processes an incoming input event (e.g., keyboard interactions like
 * Arrow Keys).
 */
ui_error_t ui_range_slider_base_process_event(
    struct ui_range_slider_base *slider, const struct ui_event *event,
    enum ui_range_slider_thumb active_thumb, double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 */
ui_error_t
ui_range_slider_base_get_component(struct ui_range_slider_base *slider,
                                   struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RANGE_SLIDER_BASE_H */
