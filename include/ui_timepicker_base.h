/**
 * @file ui_timepicker_base.h
 * @brief Defines the base logic for a timepicker UI component.
 */
#ifndef UI_TIMEPICKER_BASE_H
#define UI_TIMEPICKER_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Time formats for the timepicker.
 */
enum ui_timepicker_format {
  UI_TIMEPICKER_FORMAT_12H, /**< 12-hour format with AM/PM */
  UI_TIMEPICKER_FORMAT_24H  /**< 24-hour format */
};

/**
 * @brief Represents the AM/PM period.
 */
enum ui_timepicker_period { UI_TIMEPICKER_PERIOD_AM, UI_TIMEPICKER_PERIOD_PM };

struct ui_timepicker_base;

/**
 * @brief Creates a new unstyled timepicker base component.
 *
 * @param out_timepicker Pointer to receive the allocated timepicker base.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
ui_error_t ui_timepicker_base_create(struct ui_timepicker_base **out_timepicker,
                                     struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a timepicker base component.
 *
 * @param timepicker The timepicker to destroy.
 */
ui_error_t ui_timepicker_base_destroy(struct ui_timepicker_base *timepicker);

/**
 * @brief Sets the time in hours and minutes.
 *
 * Automatically clamps hours (0-23) and minutes (0-59).
 *
 * @param timepicker The timepicker.
 * @param hour The hour to set (24-hour format, 0-23).
 * @param minute The minute to set (0-59).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_timepicker_base_set_time(struct ui_timepicker_base *timepicker,
                                       int hour, int minute);

/**
 * @brief Gets the time in hours and minutes.
 *
 * @param timepicker The timepicker.
 * @param out_hour Pointer to receive the hour (24-hour format).
 * @param out_minute Pointer to receive the minute.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointers.
 */
ui_error_t
ui_timepicker_base_get_time(const struct ui_timepicker_base *timepicker,
                            int *out_hour, int *out_minute);

/**
 * @brief Gets the time formatted for the current clock format.
 *
 * If the format is 12H, out_hour is converted to 1-12, and out_period is
 * populated. If 24H, out_hour is 0-23, and out_period is ignored (set to AM by
 * default).
 *
 * @param timepicker The timepicker.
 * @param out_hour Pointer to receive the formatted hour.
 * @param out_minute Pointer to receive the minute.
 * @param out_period Pointer to receive the AM/PM period.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointers.
 */
ui_error_t ui_timepicker_base_get_formatted_time(
    const struct ui_timepicker_base *timepicker, int *out_hour, int *out_minute,
    enum ui_timepicker_period *out_period);

/**
 * @brief Sets the time format (12-hour or 24-hour).
 *
 * @param timepicker The timepicker.
 * @param format The format to set.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_timepicker_base_set_format(struct ui_timepicker_base *timepicker,
                                         enum ui_timepicker_format format);

/**
 * @brief Gets the time format (12-hour or 24-hour).
 *
 * @param timepicker The timepicker.
 * @param out_format Pointer to receive the format.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t
ui_timepicker_base_get_format(const struct ui_timepicker_base *timepicker,
                              enum ui_timepicker_format *out_format);

/**
 * @brief Gets the time as a serialized string using the current format.
 *
 * Uses MSVC Safe CRT internally. String is dynamically allocated.
 * The caller is responsible for freeing the string using the system allocator.
 *
 * @param timepicker The timepicker.
 * @param out_string Pointer to receive the serialized time string (e.g. "01:30
 * PM" or "13:30").
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
ui_error_t
ui_timepicker_base_get_time_string(const struct ui_timepicker_base *timepicker,
                                   char **out_string);

#ifdef __cplusplus
}
#endif

#endif /* UI_TIMEPICKER_BASE_H */
