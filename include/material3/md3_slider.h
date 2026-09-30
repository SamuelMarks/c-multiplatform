/**
 * @file md3_slider.h
 * @brief Material 3 Continuous and Discrete Slider and Range Slider components.
 */

#ifndef MATERIAL3_MD3_SLIDER_H
#define MATERIAL3_MD3_SLIDER_H

/* clang-format off */
#include "ui_slider_base.h"
#include "ui_range_slider_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_slider_type
 * @brief Material 3 Slider continuous vs discrete step mode.
 */
enum md3_slider_type { MD3_SLIDER_CONTINUOUS = 0, MD3_SLIDER_DISCRETE };

/**
 * @enum md3_slider_orientation
 * @brief Slider layout direction.
 */
enum md3_slider_orientation { MD3_SLIDER_HORIZONTAL = 0, MD3_SLIDER_VERTICAL };

/**
 * @struct md3_slider
 * @brief Material 3 Slider skin wrapping ui_slider_base.
 */
struct md3_slider {
  struct ui_slider_base *base;
  struct ui_control_value_accessor cva;
  enum md3_slider_type type;
  enum md3_slider_orientation orientation;
  float min;
  float max;
  float step;
  int has_value_indicator;
};

/**
 * @struct md3_range_slider
 * @brief Material 3 Range Slider skin wrapping ui_range_slider_base.
 */
struct md3_range_slider {
  struct ui_range_slider_base *base;
  float min;
  float max;
};

/**
 * @brief Creates a Material 3 Slider component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param type Continuous or discrete slider mode.
 * @param min Minimum value.
 * @param max Maximum value.
 * @param out_slider Pointer to receive newly created slider.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_slider_create(
    struct ui_engine *engine, enum md3_slider_type type, float min, float max,
    struct md3_slider **out_slider, struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 Slider.
 *
 * @param slider The slider to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_slider_destroy(struct md3_slider *slider);

/**
 * @brief Sets current slider value.
 *
 * @param slider The slider.
 * @param value New value (clamped between min and max).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_slider_set_value(struct md3_slider *slider, float value);

/**
 * @brief Queries current slider value.
 *
 * @param slider The slider.
 * @param out_value Pointer to receive current value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_slider_get_value(const struct md3_slider *slider, float *out_value);

/**
 * @brief Sets discrete step increment.
 *
 * @param slider The slider.
 * @param step Step amount (e.g. 1.0f).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_slider_set_step(struct md3_slider *slider, float step);

/**
 * @brief Sets disabled state of the slider.
 *
 * @param slider The slider.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_slider_set_disabled(struct md3_slider *slider, int disabled);

/**
 * @brief Sets value change callback.
 *
 * @param slider The slider.
 * @param on_change Change callback.
 * @param user_data User data for callback.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_slider_set_on_change(struct md3_slider *slider,
                         ui_slider_on_change_t on_change, void *user_data);

/**
 * @brief Retrieves underlying ui_slider_base handle.
 *
 * @param slider The slider.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_slider_get_base(
    struct md3_slider *slider, struct ui_slider_base **out_base);

/**
 * @brief Creates a Material 3 Dual-Thumb Range Slider component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param min Minimum possible value.
 * @param max Maximum possible value.
 * @param out_slider Pointer to receive newly created range slider.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_range_slider_create(struct ui_engine *engine, float min, float max,
                        struct md3_range_slider **out_slider);

/**
 * @brief Destroys a Material 3 Dual-Thumb Range Slider.
 *
 * @param slider The range slider to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_range_slider_destroy(struct md3_range_slider *slider);

/**
 * @brief Sets current low and high values for the range slider.
 *
 * @param slider The range slider.
 * @param low Low thumb value.
 * @param high High thumb value.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_range_slider_set_values(
    struct md3_range_slider *slider, float low, float high);

/**
 * @brief Retrieves current low and high values from the range slider.
 *
 * @param slider The range slider.
 * @param out_low Pointer to receive low thumb value.
 * @param out_high Pointer to receive high thumb value.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_range_slider_get_values(
    const struct md3_range_slider *slider, float *out_low, float *out_high);

/**
 * @brief Retrieves underlying ui_range_slider_base handle.
 *
 * @param slider The range slider.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_range_slider_get_base(
    struct md3_range_slider *slider, struct ui_range_slider_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_SLIDER_H */
