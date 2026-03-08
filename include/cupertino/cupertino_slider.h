#ifndef CUPERTINO_SLIDER_H
#define CUPERTINO_SLIDER_H

/**
 * @file cupertino_slider.h
 * @brief Apple Cupertino style Slider component.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_anim.h"
#include "cupertino/cupertino_color.h"

/** @brief Cupertino Slider Widget */
typedef struct CupertinoSlider {
    CMPWidget widget;             /**< Base widget interface. */
    CMPScalar value;              /**< Current value. */
    CMPScalar min_value;          /**< Minimum value. */
    CMPScalar max_value;          /**< Maximum value. */
    CMPColor min_track_color;     /**< Color for the track before the thumb. */
    CMPColor max_track_color;     /**< Color for the track after the thumb. */
    CMPColor thumb_color;         /**< Color for the thumb. */
    CMPRect bounds;               /**< Layout bounds. */
    CMPBool is_disabled;          /**< Disabled state. */
    CMPBool is_dark_mode;         /**< Dark mode styling. */
    CMPBool is_pressed;           /**< Is being interacted with. */
} CupertinoSlider;

/**
 * @brief Initializes a Cupertino Slider.
 * @param slider Pointer to the slider instance to initialize.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_slider_init(CupertinoSlider *slider);

/**
 * @brief Sets the value of the slider.
 * @param slider Pointer to the slider instance.
 * @param value The new value.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_slider_set_value(CupertinoSlider *slider, CMPScalar value);

/**
 * @brief Renders the slider.
 * @param slider Pointer to the slider instance.
 * @param ctx Pointer to the renderer used for drawing.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cupertino_slider_paint(const CupertinoSlider *slider, CMPPaintContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_SLIDER_H */
