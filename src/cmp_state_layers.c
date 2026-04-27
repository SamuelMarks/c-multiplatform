/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cmp_log.h"
/* clang-format on */

/**
 * @brief Update a state layer.
 *
 * @param layer Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_state_layer_update(cmp_state_layer_t *layer, float dt_ms) {
  if (layer == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (layer->is_active) {
    if (layer->current_radius < layer->max_radius) {
      layer->current_radius += (dt_ms / 1000.0f) * 300.0f;
    }
    if (layer->current_radius > layer->max_radius) {
      layer->current_radius = layer->max_radius;
    }
  } else {
    if (layer->current_opacity > 0.0f) {
      layer->current_opacity -= (dt_ms / 1000.0f) * 2.0f;
    }
    if (layer->current_opacity < 0.0f) {
      layer->current_opacity = 0.0f;
    }
  }

  return CMP_SUCCESS;
}

/**
 * @brief Trigger a ripple effect.
 *
 * @param layer Parameter description.
 * @param start_x Parameter description.
 * @param start_y Parameter description.
 * @param max_radius Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_state_layer_trigger_ripple(cmp_state_layer_t *layer,
                                           float start_x, float start_y,
                                           float max_radius) {
  if (layer == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  layer->origin_x = start_x;
  layer->origin_y = start_y;
  layer->max_radius = max_radius;
  layer->current_radius = 0.0f;
  layer->current_opacity = 0.12f;
  layer->is_active = 1;

  return CMP_SUCCESS;
}

/**
 * @brief Trigger a fluent reveal effect.
 *
 * @param layer Parameter description.
 * @param pointer_x Parameter description.
 * @param pointer_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_state_layer_trigger_fluent_reveal(cmp_state_layer_t *layer,
                                                  float pointer_x,
                                                  float pointer_y) {
  if (layer == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  layer->origin_x = pointer_x;
  layer->origin_y = pointer_y;
  layer->current_opacity = 1.0f;

  return CMP_SUCCESS;
}

/**
 * @brief Apply a vibrancy mask.
 *
 * @param layer Parameter description.
 * @param style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_state_layer_apply_vibrancy_mask(cmp_state_layer_t *layer,
                                                cmp_vibrancy_style_t style) {
  if (layer == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  switch (style) {
  case CMP_VIBRANCY_STYLE_LABEL:
    layer->current_opacity = 1.0f;
    break;
  case CMP_VIBRANCY_STYLE_SECONDARY_LABEL:
    layer->current_opacity = 0.6f;
    break;
  case CMP_VIBRANCY_STYLE_TERTIARY_LABEL:
    layer->current_opacity = 0.3f;
    break;
  case CMP_VIBRANCY_STYLE_FILL:
    layer->current_opacity = 0.2f;
    break;
  case CMP_VIBRANCY_STYLE_SECONDARY_FILL:
    layer->current_opacity = 0.1f;
    break;
  default:
    layer->current_opacity = 1.0f;
    break;
  }

  return CMP_SUCCESS;
}
