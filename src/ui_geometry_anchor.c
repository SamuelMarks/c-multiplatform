/* clang-format off */
#include "ui_geometry_anchor.h"
#include <stddef.h>
/* clang-format on */

ui_error_t ui_geometry_anchor_compute(const struct ui_layout_node *target,
                                      const struct ui_layout_node *overlay,
                                      const struct ui_anchor_config *config,
                                      float viewport_width,
                                      float viewport_height, float *out_x,
                                      float *out_y) {
  float target_point_x;
  float target_point_y;
  float overlay_offset_x;
  float overlay_offset_y;
  float final_x;
  float final_y;

  if (!target || !overlay || !config || !out_x || !out_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* 1. Find the absolute coordinate of the target's anchor point */
  if (config->target_x == UI_ANCHOR_EDGE_START) {
    target_point_x = target->x;
  } else if (config->target_x == UI_ANCHOR_EDGE_CENTER) {
    target_point_x = target->x + (target->width / 2.0f);
  } else {
    target_point_x = target->x + target->width;
  }

  if (config->target_y == UI_ANCHOR_EDGE_START) {
    target_point_y = target->y;
  } else if (config->target_y == UI_ANCHOR_EDGE_CENTER) {
    target_point_y = target->y + (target->height / 2.0f);
  } else {
    target_point_y = target->y + target->height;
  }

  /* 2. Find the relative offset within the overlay's box that should align to
   * the target point */
  if (config->overlay_x == UI_ANCHOR_EDGE_START) {
    overlay_offset_x = 0.0f;
  } else if (config->overlay_x == UI_ANCHOR_EDGE_CENTER) {
    overlay_offset_x = overlay->width / 2.0f;
  } else {
    overlay_offset_x = overlay->width;
  }

  if (config->overlay_y == UI_ANCHOR_EDGE_START) {
    overlay_offset_y = 0.0f;
  } else if (config->overlay_y == UI_ANCHOR_EDGE_CENTER) {
    overlay_offset_y = overlay->height / 2.0f;
  } else {
    overlay_offset_y = overlay->height;
  }

  /* 3. Compute initial final coordinates (matching the two points and adding
   * config offsets) */
  final_x = target_point_x - overlay_offset_x + config->offset_x;
  final_y = target_point_y - overlay_offset_y + config->offset_y;

  /* 4. Perform simple viewport collision adjustments if viewport dimensions are
   * provided */
  if (viewport_width > 0.0f) {
    if (final_x < 0.0f) {
      final_x = 0.0f;
    } else if (final_x + overlay->width > viewport_width) {
      final_x = viewport_width - overlay->width;
    }
  }

  if (viewport_height > 0.0f) {
    if (final_y < 0.0f) {
      final_y = 0.0f;
    } else if (final_y + overlay->height > viewport_height) {
      final_y = viewport_height - overlay->height;
    }
  }

  *out_x = final_x;
  *out_y = final_y;

  return UI_ERROR_NONE;
}
