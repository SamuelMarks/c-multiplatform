#ifndef UI_COLOR_PICKER_BASE_H
#define UI_COLOR_PICKER_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents an RGB color.
 * Values are in the range [0, 255].
 */
struct ui_color_rgb {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

/**
 * @brief Represents an HSV color.
 * h is in the range [0.0, 360.0).
 * s is in the range [0.0, 1.0].
 * v is in the range [0.0, 1.0].
 */
struct ui_color_hsv {
  double h;
  double s;
  double v;
};

/**
 * @brief Represents the manager for a headless color picker.
 */
struct ui_color_picker_base;

/**
 * @brief Creates a new color picker manager.
 *
 * @param out_picker Pointer to receive the allocated color picker.
 * @param out_cva Optional pointer to receive the control value accessor.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_color_picker_base_create(struct ui_color_picker_base **out_picker,
                            struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a color picker manager.
 *
 * @param picker The color picker manager to destroy.
 */
enum ui_error ui_color_picker_base_destroy(struct ui_color_picker_base *picker);

/**
 * @brief Converts an HSV color to RGB.
 *
 * @param hsv The input HSV color.
 * @param out_rgb Pointer to receive the output RGB color.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_color_picker_hsv_to_rgb(const struct ui_color_hsv *hsv,
                                         struct ui_color_rgb *out_rgb);

/**
 * @brief Converts an RGB color to HSV.
 *
 * @param rgb The input RGB color.
 * @param out_hsv Pointer to receive the output HSV color.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_color_picker_rgb_to_hsv(const struct ui_color_rgb *rgb,
                                         struct ui_color_hsv *out_hsv);

/**
 * @brief Converts an RGB color to a HEX string (e.g., "#FF0000").
 *
 * @param rgb The input RGB color.
 * @param out_hex Buffer to receive the HEX string (must be at least 8 bytes).
 * @param hex_size Size of the out_hex buffer.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_color_picker_rgb_to_hex(const struct ui_color_rgb *rgb,
                                         char *out_hex, size_t hex_size);

/**
 * @brief Converts a HEX string (e.g., "#FF0000" or "FF0000") to an RGB color.
 *
 * @param hex The input HEX string.
 * @param out_rgb Pointer to receive the output RGB color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_PARSE_FAILED if the string is
 * invalid.
 */
enum ui_error ui_color_picker_hex_to_rgb(const char *hex,
                                         struct ui_color_rgb *out_rgb);

/**
 * @brief Calculates HSV values from a 2D coordinate on a saturation/value
 * picking plane.
 *
 * @param hue The constant hue for the plane [0.0, 360.0).
 * @param x The X coordinate representing saturation.
 * @param y The Y coordinate representing value (usually inverted in UI, where
 * y=0 is v=1).
 * @param width The total width of the picking plane.
 * @param height The total height of the picking plane.
 * @param out_hsv Pointer to receive the calculated HSV color.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_color_picker_calc_hsv_from_2d(double hue, double x, double y,
                                               double width, double height,
                                               struct ui_color_hsv *out_hsv);

/**
 * @brief Gets the current selected RGB color.
 *
 * @param picker The color picker manager.
 * @param out_rgb Pointer to receive the current RGB color.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_color_picker_base_get_rgb(const struct ui_color_picker_base *picker,
                             struct ui_color_rgb *out_rgb);

/**
 * @brief Sets the current selected RGB color.
 *
 * @param picker The color picker manager.
 * @param rgb The new RGB color to set.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_color_picker_base_set_rgb(struct ui_color_picker_base *picker,
                                           const struct ui_color_rgb *rgb);

/**
 * @brief Sets the current selected HSV color.
 *
 * @param picker The color picker manager.
 * @param hsv The new HSV color to set.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_color_picker_base_set_hsv(struct ui_color_picker_base *picker,
                                           const struct ui_color_hsv *hsv);

#ifdef __cplusplus
}
#endif

#endif /* UI_COLOR_PICKER_BASE_H */
