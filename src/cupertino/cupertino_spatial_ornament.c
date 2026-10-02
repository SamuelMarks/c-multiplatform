/**
 * @file cupertino_spatial_ornament.c
 * @brief visionOS & iPadOS/macOS Floating Spatial Glass Ornaments
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_spatial_ornament.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t cupertino_spatial_ornament_create(
    struct ui_engine *engine,
    const struct cupertino_spatial_ornament_descriptor *desc,
    struct cupertino_spatial_ornament **out_ornament) {
  struct cupertino_spatial_ornament *ornament;

  if (!engine || !desc || !out_ornament) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ornament = (struct cupertino_spatial_ornament *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_spatial_ornament));
  if (!ornament) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(ornament, 0, sizeof(*ornament));
  ornament->placement = desc->placement;
  ornament->width = desc->width > 0.0f ? desc->width : 280.0f;
  ornament->height = desc->height > 0.0f ? desc->height : 48.0f;
  ornament->corner_radius =
      desc->corner_radius >= 0.0f ? desc->corner_radius : 24.0f;
  ornament->depth_offset_pt =
      desc->depth_offset_pt >= 0.0f ? desc->depth_offset_pt : 24.0f;
  ornament->ambient_occlusion =
      (desc->ambient_occlusion >= 0.0f && desc->ambient_occlusion <= 1.0f)
          ? desc->ambient_occlusion
          : 0.8f;
  ornament->edge_gap = desc->edge_gap >= 0.0f ? desc->edge_gap : 12.0f;
  ornament->is_visible = 1;

  *out_ornament = ornament;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_destroy(
    struct cupertino_spatial_ornament *ornament) {
  if (!ornament) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(ornament);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_set_placement(
    struct cupertino_spatial_ornament *ornament,
    enum cupertino_ornament_placement placement) {
  if (!ornament) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ornament->placement = placement;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_get_placement(
    const struct cupertino_spatial_ornament *ornament,
    enum cupertino_ornament_placement *out_placement) {
  if (!ornament || !out_placement) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_placement = ornament->placement;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_set_depth(
    struct cupertino_spatial_ornament *ornament, float depth_offset_pt,
    float ambient_occlusion) {
  if (!ornament || depth_offset_pt < 0.0f || ambient_occlusion < 0.0f ||
      ambient_occlusion > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ornament->depth_offset_pt = depth_offset_pt;
  ornament->ambient_occlusion = ambient_occlusion;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_get_depth(
    const struct cupertino_spatial_ornament *ornament, float *out_depth,
    float *out_occlusion) {
  if (!ornament || !out_depth || !out_occlusion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_depth = ornament->depth_offset_pt;
  *out_occlusion = ornament->ambient_occlusion;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_spatial_ornament_set_size(struct cupertino_spatial_ornament *ornament,
                                    float width, float height) {
  if (!ornament || width <= 0.0f || height <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ornament->width = width;
  ornament->height = height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_get_size(
    const struct cupertino_spatial_ornament *ornament, float *out_width,
    float *out_height) {
  if (!ornament || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = ornament->width;
  *out_height = ornament->height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_set_visible(
    struct cupertino_spatial_ornament *ornament, int is_visible) {
  if (!ornament) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ornament->is_visible = is_visible ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_is_visible(
    const struct cupertino_spatial_ornament *ornament, int *out_visible) {
  if (!ornament || !out_visible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_visible = ornament->is_visible;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_compute_bounds(
    const struct cupertino_spatial_ornament *ornament, float window_x,
    float window_y, float window_w, float window_h, float *out_x, float *out_y,
    float *out_w, float *out_h) {
  if (!ornament || !out_x || !out_y || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_w = ornament->width;
  *out_h = ornament->height;

  switch (ornament->placement) {
  case CUPERTINO_ORNAMENT_PLACEMENT_TOP:
    *out_x = window_x + (window_w - ornament->width) * 0.5f;
    *out_y = window_y - ornament->height - ornament->edge_gap;
    break;
  case CUPERTINO_ORNAMENT_PLACEMENT_BOTTOM:
    *out_x = window_x + (window_w - ornament->width) * 0.5f;
    *out_y = window_y + window_h + ornament->edge_gap;
    break;
  case CUPERTINO_ORNAMENT_PLACEMENT_LEADING:
    *out_x = window_x - ornament->width - ornament->edge_gap;
    *out_y = window_y + (window_h - ornament->height) * 0.5f;
    break;
  case CUPERTINO_ORNAMENT_PLACEMENT_TRAILING:
  default:
    *out_x = window_x + window_w + ornament->edge_gap;
    *out_y = window_y + (window_h - ornament->height) * 0.5f;
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_spatial_ornament_compute_shadow(
    const struct cupertino_spatial_ornament *ornament,
    float *out_shadow_offset_y, float *out_shadow_radius,
    float *out_shadow_alpha) {
  if (!ornament || !out_shadow_offset_y || !out_shadow_radius ||
      !out_shadow_alpha) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_shadow_offset_y = ornament->depth_offset_pt * 0.4f;
  *out_shadow_radius = ornament->depth_offset_pt * 0.8f;
  *out_shadow_alpha = ornament->ambient_occlusion * 0.35f;

  return UI_ERROR_NONE;
}
