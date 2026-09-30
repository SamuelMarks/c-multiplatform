/**
 * @file md3_state_layer.h
 * @brief Material 3 state layer opacity values and color blending.
 */

#ifndef MATERIAL3_MD3_STATE_LAYER_H
#define MATERIAL3_MD3_STATE_LAYER_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md3_state_layer_type
 * @brief Visual interaction state types for Material 3 state layers.
 */
enum md3_state_layer_type {
  MD3_STATE_LAYER_HOVER = 0, /**< Hover state: 8% opacity */
  MD3_STATE_LAYER_FOCUS,     /**< Focus state: 10% opacity */
  MD3_STATE_LAYER_PRESSED,   /**< Pressed state: 10% opacity */
  MD3_STATE_LAYER_DRAGGED    /**< Dragged state: 16% opacity */
};

/**
 * @brief Retrieves the standard state layer opacity for a given interaction
 * state.
 *
 * @param type Interaction state type.
 * @param out_opacity Pointer to receive the opacity fraction.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * type/pointer.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_state_layer_get_opacity(enum md3_state_layer_type type, float *out_opacity);

/**
 * @brief Blends a content state layer over a base color.
 *
 * @param base_color The underlying container or surface color.
 * @param content_color The state content color (typically on-* color or
 * primary).
 * @param type Interaction state type.
 * @param out_color Pointer to store the blended result color.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_state_layer_blend(ui_color_t base_color, ui_color_t content_color,
                      enum md3_state_layer_type type, ui_color_t *out_color);

/**
 * @enum md3_ripple_style
 * @brief Ripple animation styles including baseline and expressive variants.
 */
enum md3_ripple_style {
  MD3_RIPPLE_STYLE_STANDARD = 0, /**< Standard radial circle ripple */
  MD3_RIPPLE_STYLE_FLUID_WAVE,   /**< Fluid wave dissipation */
  MD3_RIPPLE_STYLE_SPARKLE       /**< Starburst / sparkle expressive ripple */
};

/**
 * @struct md3_ripple_params
 * @brief Dynamic parameters of an active ripple.
 */
struct md3_ripple_params {
  enum md3_ripple_style style; /**< Ripple animation style */
  float origin_x;              /**< Center X coordinate */
  float origin_y;              /**< Center Y coordinate */
  float max_radius;            /**< Target max expansion radius */
  float progress;              /**< Animation progress [0.0, 1.0] */
};

/**
 * @struct md3_ripple_frame
 * @brief Evaluated render frame for a ripple.
 */
struct md3_ripple_frame {
  float current_radius;    /**< Current expansion radius */
  float current_opacity;   /**< Effective alpha opacity [0.0, 1.0] */
  float wave_distortion;   /**< Wave surface distortion fraction */
  int sparkle_count;       /**< Count of active sparkle points */
  float sparkle_scales[8]; /**< Radii scales of sparkles */
};

/**
 * @brief Evaluates an animated ripple frame.
 *
 * @param params Input ripple parameters (progress, style, radius).
 * @param out_frame Pointer to receive the evaluated frame metrics.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_ripple_evaluate(
    const struct md3_ripple_params *params, struct md3_ripple_frame *out_frame);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_STATE_LAYER_H */
