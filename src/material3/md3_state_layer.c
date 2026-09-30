/**
 * @file md3_state_layer.c
 * @brief Implementation of Material 3 state layer opacity values and color
 * blending.
 */

/* clang-format off */
#include "material3/md3_state_layer.h"
#include <math.h>
#include <string.h>
/* clang-format on */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Retrieves the standard state layer opacity for a given interaction
 * state.
 *
 * @param type Interaction state type.
 * @param out_opacity Pointer to receive the opacity fraction.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * type/pointer.
 */
ui_error_t md3_state_layer_get_opacity(enum md3_state_layer_type type,
                                       float *out_opacity) {
  if (!out_opacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (type) {
  case MD3_STATE_LAYER_HOVER:
    *out_opacity = 0.08f;
    return UI_ERROR_NONE;
  case MD3_STATE_LAYER_FOCUS:
  case MD3_STATE_LAYER_PRESSED:
    *out_opacity = 0.10f;
    return UI_ERROR_NONE;
  case MD3_STATE_LAYER_DRAGGED:
    *out_opacity = 0.16f;
    return UI_ERROR_NONE;
  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }
}

/**
 * @brief Blends a content state layer over a base color.
 *
 * @param base_color The underlying container or surface color.
 * @param content_color The state content color.
 * @param type Interaction state type.
 * @param out_color Pointer to store the blended result color.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_state_layer_blend(ui_color_t base_color,
                                 ui_color_t content_color,
                                 enum md3_state_layer_type type,
                                 ui_color_t *out_color) {
  float opacity;
  float inv_opacity;
  ui_uint8 a, r, g, b;
  ui_error_t rc;

  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = md3_state_layer_get_opacity(type, &opacity);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  inv_opacity = 1.0f - opacity;
  a = UI_COLOR_ALPHA(base_color);
  r = (ui_uint8)((float)UI_COLOR_RED(base_color) * inv_opacity +
                 (float)UI_COLOR_RED(content_color) * opacity + 0.5f);
  g = (ui_uint8)((float)UI_COLOR_GREEN(base_color) * inv_opacity +
                 (float)UI_COLOR_GREEN(content_color) * opacity + 0.5f);
  b = (ui_uint8)((float)UI_COLOR_BLUE(base_color) * inv_opacity +
                 (float)UI_COLOR_BLUE(content_color) * opacity + 0.5f);

  *out_color = UI_COLOR_ARGB(a, r, g, b);
  return UI_ERROR_NONE;
}

/**
 * @brief Evaluates an animated ripple frame.
 *
 * @param params Input ripple parameters (progress, style, radius).
 * @param out_frame Pointer to receive the evaluated frame metrics.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad inputs.
 */
ui_error_t md3_ripple_evaluate(const struct md3_ripple_params *params,
                               struct md3_ripple_frame *out_frame) {
  int i;
  float p;

  if (!params || !out_frame || params->progress < 0.0f ||
      params->progress > 1.0f || params->max_radius < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  p = params->progress;
  memset(out_frame, 0, sizeof(struct md3_ripple_frame));

  switch (params->style) {
  case MD3_RIPPLE_STYLE_FLUID_WAVE:
    out_frame->current_radius =
        params->max_radius * (float)sin((double)p * M_PI * 0.5);
    out_frame->current_opacity = 1.0f - (p * p);
    out_frame->wave_distortion =
        (float)sin((double)p * M_PI * 3.0) * (1.0f - p) * 0.25f;
    out_frame->sparkle_count = 0;
    break;

  case MD3_RIPPLE_STYLE_SPARKLE:
    out_frame->current_radius = params->max_radius * (0.5f + 0.5f * p);
    out_frame->current_opacity = 1.0f - p;
    out_frame->wave_distortion = 0.0f;
    out_frame->sparkle_count = 8;
    for (i = 0; i < 8; ++i) {
      out_frame->sparkle_scales[i] =
          (float)(0.2 + 0.8 * sin((double)(p * 4.0f + (float)i * 0.785f)));
      if (out_frame->sparkle_scales[i] < 0.0f) {
        out_frame->sparkle_scales[i] = 0.0f;
      }
    }
    break;

  case MD3_RIPPLE_STYLE_STANDARD:
    out_frame->current_radius = params->max_radius * p;
    out_frame->current_opacity = 1.0f - p;
    out_frame->wave_distortion = 0.0f;
    out_frame->sparkle_count = 0;
    break;

  default:
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}
