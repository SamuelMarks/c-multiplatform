#ifndef UI_METER_BASE_H
#define UI_METER_BASE_H

struct ui_computed;

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an unstyled meter component (gauge).
 */
struct ui_meter_base {
  struct ui_component base;
  float value;
  float min_val;
  float max_val;
  float low_val;
  float high_val;
  float optimum_val;
  struct ui_signal *value_signal;
};

/**
 * @brief Creates a new base meter component.
 *
 * @param out_meter Pointer to output the initialized meter.
 * @return UI_ERROR_NONE on success, or an error code.
 */
enum ui_error ui_meter_base_create(struct ui_meter_base **out_meter);

/**
 * @brief Sets the current value of the meter.
 *
 * @param meter The meter component.
 * @param value The value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_meter_base_set_value(struct ui_meter_base *meter, float value);

/**
 * @brief Sets the minimum and maximum boundaries of the meter.
 *
 * @param meter The meter component.
 * @param min_val The minimum value.
 * @param max_val The maximum value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_meter_base_set_bounds(struct ui_meter_base *meter,
                                       float min_val, float max_val);

/**
 * @brief Sets the gauge thresholds for low, high, and optimum ranges.
 *
 * @param meter The meter component.
 * @param low_val The low threshold.
 * @param high_val The high threshold.
 * @param optimum_val The optimum value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_meter_base_set_thresholds(struct ui_meter_base *meter,
                                           float low_val, float high_val,
                                           float optimum_val);

/**
 * @brief Binds the value property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_meter_base_bind_value(struct ui_meter_base *widget,
                                       struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_METER_BASE_H */
