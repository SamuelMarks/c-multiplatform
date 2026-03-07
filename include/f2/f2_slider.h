#ifndef F2_SLIDER_H
#define F2_SLIDER_H

/**
 * @file f2_slider.h
 * @brief Microsoft Fluent 2 slider widgets.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "f2/f2_color.h"

/** @brief Default slider track height. */
#define F2_SLIDER_DEFAULT_TRACK_HEIGHT 4.0f
/** @brief Default slider track length. */
#define F2_SLIDER_DEFAULT_TRACK_LENGTH 150.0f
/** @brief Default slider thumb radius. */
#define F2_SLIDER_DEFAULT_THUMB_RADIUS 10.0f
/** @brief Default slider thumb inner dot radius. */
#define F2_SLIDER_DEFAULT_THUMB_INNER_RADIUS 6.0f

struct F2Slider;

/**
 * @brief Slider change callback.
 * @param ctx User callback context pointer.
 * @param slider Slider instance.
 * @param value New value.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2SliderOnChange)(void *ctx, struct F2Slider *slider, CMPScalar value);

/**
 * @brief Fluent 2 Slider style descriptor.
 */
typedef struct F2SliderStyle {
  CMPBool is_rtl;             /**< Layout direction. */
  CMPScalar track_height;     /**< Height of the slider track. */
  CMPScalar thumb_radius;     /**< Thumb outer radius. */
  CMPScalar thumb_inner_radius; /**< Thumb inner dot radius. */
  CMPColor track_color;       /**< Unfilled track color. */
  CMPColor active_track_color;/**< Filled track color. */
  CMPColor thumb_color;       /**< Outer thumb color. */
  CMPColor thumb_inner_color; /**< Inner thumb dot color. */
  CMPColor disabled_track_color; /**< Disabled track color. */
  CMPColor disabled_thumb_color; /**< Disabled thumb color. */
} F2SliderStyle;

/**
 * @brief Fluent 2 Slider widget instance.
 */
typedef struct F2Slider {
  CMPWidget widget;         /**< Widget interface. */
  F2SliderStyle style;      /**< Current style. */
  CMPRect bounds;           /**< Layout bounds. */
  CMPScalar min_value;      /**< Minimum value. */
  CMPScalar max_value;      /**< Maximum value. */
  CMPScalar value;          /**< Current value. */
  CMPScalar step;           /**< Step value (0 for continuous). */
  CMPBool disabled;         /**< Disabled state. */
  CMPBool hovered;          /**< Hover state. */
  CMPBool pressed;          /**< Pressed state. */
  F2SliderOnChange on_change; /**< Change callback. */
  void *on_change_ctx;      /**< Change callback context. */
} F2Slider;

/**
 * @brief Initialize a Fluent 2 slider style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_slider_style_init(F2SliderStyle *style);

/**
 * @brief Initialize a Fluent 2 slider widget.
 * @param slider Slider instance.
 * @param style Style descriptor.
 * @param min_value Minimum value.
 * @param max_value Maximum value.
 * @param value Initial value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_slider_init(F2Slider *slider,
                                    const F2SliderStyle *style,
                                    CMPScalar min_value,
                                    CMPScalar max_value,
                                    CMPScalar value);

/**
 * @brief Update the slider value.
 * @param slider Slider instance.
 * @param value New value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_slider_set_value(F2Slider *slider, CMPScalar value);

/**
 * @brief Set the slider step size.
 * @param slider Slider instance.
 * @param step Step size (0 for continuous).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_slider_set_step(F2Slider *slider, CMPScalar step);

/**
 * @brief Update the slider disabled state.
 * @param slider Slider instance.
 * @param disabled New disabled state.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_slider_set_disabled(F2Slider *slider, CMPBool disabled);

/**
 * @brief Assign a callback for when the slider value changes.
 * @param slider Slider instance.
 * @param on_change Callback function.
 * @param ctx Callback context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_slider_set_on_change(F2Slider *slider,
                                             F2SliderOnChange on_change,
                                             void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* F2_SLIDER_H */
