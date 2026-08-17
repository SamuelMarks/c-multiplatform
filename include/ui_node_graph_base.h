#ifndef UI_NODE_GRAPH_BASE_H
#define UI_NODE_GRAPH_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
#include "ui_geometry.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_node_graph_base
 * @brief Opaque handle for the node graph component.
 */
struct ui_node_graph_base;

/**
 * @struct ui_node_graph_port
 * @brief Represents a port on a node where connections can be made.
 */
struct ui_node_graph_port {
  /** @brief The ID of the node owning this port. */
  const char *node_id;
  /** @brief The unique ID of the port. */
  const char *port_id;
  /** @brief Non-zero if this is an input port, 0 if output. */
  ui_bool_t is_input;
  /** @brief Absolute position in graph space. */
  struct ui_dom_point center;
};

/**
 * @struct ui_node_graph_connection
 * @brief Represents a connection (spline) between two ports.
 */
struct ui_node_graph_connection {
  /** @brief The unique ID of this connection. */
  const char *connection_id;
  /** @brief The source port of the connection. */
  struct ui_node_graph_port source;
  /** @brief The target port of the connection. */
  struct ui_node_graph_port target;
};

/**
 * @struct ui_node_graph_camera_config
 * @brief Configuration for the 2D Camera constraints.
 */
struct ui_node_graph_camera_config {
  /** @brief The minimum allowed zoom level. */
  float min_zoom;
  /** @brief The maximum allowed zoom level. */
  float max_zoom;
  /** @brief Bounding box constraints for panning. Use 0 width/height for
   * infinite. */
  struct ui_dom_rect bounds;
};

/**
 * @brief Creates a node graph base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param camera_config Constraints for the 2D camera.
 * @param out_graph Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_node_graph_base_create(
    struct ui_arena *arena,
    const struct ui_node_graph_camera_config *camera_config,
    struct ui_node_graph_base **out_graph);

/**
 * @brief Destroys a node graph base component.
 *
 * @param graph The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_node_graph_base_destroy(struct ui_node_graph_base *graph);

/**
 * @brief Applies a pan offset to the 2D camera.
 *
 * @param graph The component.
 * @param delta_x The horizontal pan delta in pixels.
 * @param delta_y The vertical pan delta in pixels.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_node_graph_base_pan(struct ui_node_graph_base *graph,
                                  float delta_x, float delta_y);

/**
 * @brief Sets the zoom level of the 2D camera.
 *
 * @param graph The component.
 * @param zoom The new zoom level. Will be clamped to camera config constraints.
 * @param focal_point Optional. The point in canvas space to zoom towards. If
 * NULL, zooms to center.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_node_graph_base_zoom(struct ui_node_graph_base *graph, float zoom,
                                   const struct ui_dom_point *focal_point);

/**
 * @brief Retrieves the signal emitted when the camera matrix changes.
 * Payload is a pointer to the ui_dom_matrix.
 *
 * @param graph The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_node_graph_base_get_camera_signal(struct ui_node_graph_base *graph,
                                     ui_signal_t **out_signal);

/**
 * @brief Transforms a point from screen space (viewport) into graph space
 * (canvas).
 *
 * @param graph The component.
 * @param screen_point The point in screen space.
 * @param out_graph_point Pointer to receive the point in graph space.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_node_graph_base_screen_to_graph(const struct ui_node_graph_base *graph,
                                   const struct ui_dom_point *screen_point,
                                   struct ui_dom_point *out_graph_point);

/**
 * @brief Adds a new connection between two nodes.
 *
 * @param graph The component.
 * @param connection The connection details.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_node_graph_base_add_connection(
    struct ui_node_graph_base *graph,
    const struct ui_node_graph_connection *connection);

/**
 * @brief Sets the current marquee selection rectangle.
 * All nodes falling within this rectangle in graph space should be considered
 * selected.
 *
 * @param graph The component.
 * @param selection_rect The selection rectangle in graph space. Can be NULL to
 * clear.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_node_graph_base_set_marquee_selection(
    struct ui_node_graph_base *graph, const struct ui_dom_rect *selection_rect);

/**
 * @brief Retrieves the signal emitted when the marquee selection or node
 * connections change. Payload is NULL.
 *
 * @param graph The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_node_graph_base_get_topology_signal(struct ui_node_graph_base *graph,
                                       ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_NODE_GRAPH_BASE_H */
