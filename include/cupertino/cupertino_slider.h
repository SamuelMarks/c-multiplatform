/**
 * @file cupertino_slider.h
 * @brief Cupertino Slider component wrapping ui_slider_base with Apple HIG
 * styling.
 */

#ifndef CUPERTINO_CUPERTINO_SLIDER_H
#define CUPERTINO_CUPERTINO_SLIDER_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_slider_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Standard Apple HIG slider track height.
 */
#define CUPERTINO_SLIDER_TRACK_HEIGHT 4.0f
/**
 * @brief Standard Apple HIG slider thumb diameter.
 */
#define CUPERTINO_SLIDER_THUMB_DIAMETER 28.0f

/**
 * @struct cupertino_slider
 * @brief Cupertino Slider skin wrapping ui_slider_base.
 */
struct cupertino_slider {
  struct ui_slider_base *base;          /**< Wrapped CDK slider primitive. */
  struct ui_control_value_accessor cva; /**< Control Value Accessor. */
  float min;                            /**< Minimum value. */
  float max;                            /**< Maximum value. */
  float step;                           /**< Step increment (0 = continuous). */
  ui_color_t active_color; /**< Active leading track fill (SystemBlue). */
  ui_color_t track_color;  /**< Inactive trailing track fill (SystemGray5). */
  ui_color_t thumb_color;  /**< Circular thumb fill (White). */
};

/**
 * @brief Creates a new Cupertino Slider component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param min Minimum possible value.
 * @param max Maximum possible value.
 * @param out_slider Pointer to receive newly created slider.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_slider_create(struct ui_engine *engine, float min, float max,
                        struct cupertino_slider **out_slider,
                        struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino slider and its underlying base.
 *
 * @param slider The slider to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_slider_destroy(struct cupertino_slider *slider);

/**
 * @brief Sets current slider value.
 *
 * @param slider The slider.
 * @param value New value (clamped between min and max).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_slider_set_value(struct cupertino_slider *slider, float value);

/**
 * @brief Queries current slider value.
 *
 * @param slider The slider.
 * @param out_value Pointer to receive current value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_slider_get_value(
    const struct cupertino_slider *slider, float *out_value);

/**
 * @brief Sets discrete step increment with magnetic snapping.
 *
 * @param slider The slider.
 * @param step Step amount (e.g. 1.0f, or 0.0f for continuous).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_slider_set_step(struct cupertino_slider *slider, float step);

/**
 * @brief Sets disabled state of the slider.
 *
 * @param slider The slider.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_slider_set_disabled(struct cupertino_slider *slider, int disabled);

/**
 * @brief Retrieves underlying ui_slider_base handle.
 *
 * @param slider The slider.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_slider_get_base(
    struct cupertino_slider *slider, struct ui_slider_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SLIDER_H */
