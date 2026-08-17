/**
 * @file ui_geometry_anchor.h
 * @brief Logic for anchoring overlays and popovers.
 *
 * This header defines structures and functions to compute the exact absolute
 * coordinates of an overlay (like a tooltip or popover) relative to a target
 * node, accounting for user-specified edges and offset rules.
 */

#ifndef UI_GEOMETRY_ANCHOR_H
#define UI_GEOMETRY_ANCHOR_H

/* clang-format off */
#include "ui_error.h"
#include "ui_layout.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents an attachment edge or point on an element's axis.
 */
enum ui_anchor_edge {
  UI_ANCHOR_EDGE_START =
      0, /**< Start edge (Left for X axis, Top for Y axis). */
  UI_ANCHOR_EDGE_CENTER = 1, /**< Center point. */
  UI_ANCHOR_EDGE_END = 2 /**< End edge (Right for X axis, Bottom for Y axis). */
};

/**
 * @brief Configuration for anchoring an overlay to a target node.
 */
struct ui_anchor_config {
  enum ui_anchor_edge target_x; /**< X-axis anchor point on the target node. */
  enum ui_anchor_edge target_y; /**< Y-axis anchor point on the target node. */
  enum ui_anchor_edge
      overlay_x; /**< X-axis anchor point on the overlay node. */
  enum ui_anchor_edge
      overlay_y;  /**< Y-axis anchor point on the overlay node. */
  float offset_x; /**< Horizontal offset applied after anchoring. */
  float offset_y; /**< Vertical offset applied after anchoring. */
};

/**
 * @brief Computes the absolute coordinates for an overlay to attach to a target
 * node.
 *
 * Computes the position by matching the overlay's specified anchor point to the
 * target's specified anchor point, then adding the requested offset.
 *
 * @param target Pointer to the target layout node (the trigger).
 * @param overlay Pointer to the overlay layout node to position (the
 * tooltip/popover).
 * @param config Pointer to the anchor configuration detailing how to connect
 * the two boxes.
 * @param viewport_width The width of the viewport, for collision fallback
 * (optional, set to 0.0f if unused).
 * @param viewport_height The height of the viewport, for collision fallback
 * (optional, set to 0.0f if unused).
 * @param out_x Pointer to receive the computed absolute X position.
 * @param out_y Pointer to receive the computed absolute Y position.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_geometry_anchor_compute(const struct ui_layout_node *target,
                                      const struct ui_layout_node *overlay,
                                      const struct ui_anchor_config *config,
                                      float viewport_width,
                                      float viewport_height, float *out_x,
                                      float *out_y);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_GEOMETRY_ANCHOR_H */
