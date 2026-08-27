/**
 * @file ui_range_slider_base.h
 */
#ifndef UI_RANGE_SLIDER_BASE_H
#define UI_RANGE_SLIDER_BASE_H

/**
 * \file ui_range_slider_base.h
 * \brief UI Range Slider Base component.
 *
 * This file contains definitions for a range slider component
 * allowing selection of a sub-range (low and high values) within bounds.
 */

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

/** \brief Opaque handle representing a range slider base component. */
struct ui_range_slider_base;

/**
 * @brief Which thumb is active or being interacted with.
 */
enum ui_range_slider_thumb {
  UI_RANGE_SLIDER_THUMB_NONE = 0, /**< No thumb */
  UI_RANGE_SLIDER_THUMB_LOW = 1,  /**< The low value thumb */
  UI_RANGE_SLIDER_THUMB_HIGH = 2  /**< The high value thumb */
};

/**
 * @brief Callback invoked when the range slider values change.
 *
 * @param slider The range slider component.
 * @param low_value The new low value.
 * @param high_value The new high value.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
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
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_destroy(struct ui_range_slider_base *slider);

/**
 * @brief Sets the minimum possible value of the range slider.
 *
 * @param slider The range slider component.
 * @param min The new minimum value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_set_min(struct ui_range_slider_base *slider,
                                        float min);

/**
 * @brief Sets the maximum possible value of the range slider.
 *
 * @param slider The range slider component.
 * @param max The new maximum value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_set_max(struct ui_range_slider_base *slider,
                                        float max);

/**
 * @brief Sets the current values of the range slider.
 *
 * @param slider The range slider component.
 * @param low_value The new low value (clamped to bounds).
 * @param high_value The new high value (clamped to bounds).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_set_values(struct ui_range_slider_base *slider,
                                           float low_value, float high_value);

/**
 * @brief Gets the current values of the range slider.
 *
 * @param slider The range slider component.
 * @param out_low Pointer to receive the low value.
 * @param out_high Pointer to receive the high value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_get_values(const struct ui_range_slider_base *slider,
                                float *out_low, float *out_high);

/**
 * @brief Sets the step increment. If 0.0, the slider is continuous.
 *
 * @param slider The range slider component.
 * @param step The step increment value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_set_step(struct ui_range_slider_base *slider,
                                         float step);

/**
 * @brief Sets the disabled state of the range slider.
 *
 * @param slider The range slider component.
 * @param disabled Non-zero to disable, 0 to enable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_set_disabled(struct ui_range_slider_base *slider,
                                  int disabled);

/**
 * @brief Sets the change handler for the range slider.
 *
 * @param slider The range slider component.
 * @param on_change The callback invoked on value change.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_set_on_change(struct ui_range_slider_base *slider,
                                   ui_range_slider_on_change_t on_change,
                                   void *user_data);

/**
 * @brief Processes an incoming input event to trigger slider interactions based
 * on normalized pointer position.
 *
 * @param slider The range slider component.
 * @param thumb Which thumb is active.
 * @param normalized_position The normalized position along the track (0.0
 * to 1.0).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_set_normalized_value(struct ui_range_slider_base *slider,
                                          enum ui_range_slider_thumb thumb,
                                          float normalized_position);

/**
 * @brief Processes an incoming input event (e.g., keyboard interactions like
 * Arrow Keys).
 *
 * @param slider The range slider component.
 * @param event The input event.
 * @param active_thumb Which thumb is active for the keyboard event.
 * @param timestamp_ms Event timestamp in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_range_slider_base_process_event(
    struct ui_range_slider_base *slider, const struct ui_event *event,
    enum ui_range_slider_thumb active_thumb, double timestamp_ms);

/**
 * @brief Gets the underlying component instance for style injection and DOM
 * mounting.
 *
 * @param slider The range slider component.
 * @param out_component Pointer to receive the underlying component.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_range_slider_base_get_component(struct ui_range_slider_base *slider,
                                   struct ui_component **out_component);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RANGE_SLIDER_BASE_H */
