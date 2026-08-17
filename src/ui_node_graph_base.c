/**
 * \file ui_node_graph_base.c
 * \brief Implementation of the UI node graph component for visual programming
 * or flow charts.
 */
/* clang-format off */
#include "ui_node_graph_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * \def UI_NODE_GRAPH_MAX_CONNECTIONS
 * \brief Maximum allowed number of connections in the graph.
 */
#define UI_NODE_GRAPH_MAX_CONNECTIONS 256

/**
 * \struct ui_node_graph_base
 * \brief Core state and camera context for a node graph widget.
 */
struct ui_node_graph_base {
  struct ui_arena *arena;
  struct ui_node_graph_camera_config camera_config;

  float zoom;
  float pan_x;
  float pan_y;
  struct ui_dom_matrix camera_matrix;

  struct ui_node_graph_connection connections[UI_NODE_GRAPH_MAX_CONNECTIONS];
  int num_connections;

  struct ui_dom_rect current_marquee;
  ui_bool_t has_marquee;

  ui_signal_t *camera_signal;
  ui_signal_t *topology_signal;
};

/**
 * \brief Equality check for pointer payloads.
 * \param[in] a First payload.
 * \param[in] b Second payload.
 * \param[out] out_equal True if equal.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t pointer_equality(union ui_signal_payload a,
                                   union ui_signal_payload b,
                                   ui_bool_t *out_equal) {
  *out_equal = (a.ptr_val == b.ptr_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * \brief Equality check that always returns false (for triggering signals
 * unconditionally).
 * \param[in] a First payload.
 * \param[in] b Second payload.
 * \param[out] out_equal Always set to UI_FALSE.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t void_equality(union ui_signal_payload a,
                                union ui_signal_payload b,
                                ui_bool_t *out_equal) {
  (void)a;
  (void)b;
  *out_equal = UI_FALSE;
  return UI_ERROR_NONE;
}

/**
 * \brief Updates the internal camera matrix based on pan and zoom and triggers
 * its signal.
 * \param[in,out] graph The node graph widget.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t update_camera_matrix(struct ui_node_graph_base *graph) {
  union ui_signal_payload payload;
  ui_error_t rc;

  /* Construct simple 2D transform matrix (scale + translate) */
  (void)ui_dom_matrix_init_identity(&graph->camera_matrix);

  graph->camera_matrix.m11 = graph->zoom;
  graph->camera_matrix.m22 = graph->zoom;
  graph->camera_matrix.m41 = graph->pan_x;
  graph->camera_matrix.m42 = graph->pan_y;

  payload.ptr_val = &graph->camera_matrix;
  /* Ignore error internally as this is synchronous state update */
  rc = ui_signal_set(graph->camera_signal, payload);
  return rc;
}

/** \brief ui_node_graph_base_create */
ui_error_t ui_node_graph_base_create(
    struct ui_arena *arena,
    const struct ui_node_graph_camera_config *camera_config,
    struct ui_node_graph_base **out_graph) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !camera_config || !out_graph) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_node_graph_base), 8, &ptr);
  if (err != UI_ERROR_NONE)
    return err;

  *out_graph = (struct ui_node_graph_base *)ptr;
  (*out_graph)->arena = arena;
  (*out_graph)->camera_config = *camera_config;
  (*out_graph)->zoom = 1.0f;
  (*out_graph)->pan_x = 0.0f;
  (*out_graph)->pan_y = 0.0f;
  (*out_graph)->num_connections = 0;
  (*out_graph)->has_marquee = UI_FALSE;
  (*out_graph)->camera_signal = NULL;
  (*out_graph)->topology_signal = NULL;

  (void)ui_dom_matrix_init_identity(&(*out_graph)->camera_matrix);

  initial_payload.ptr_val = &(*out_graph)->camera_matrix;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER,
                         pointer_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_graph)->camera_signal);
  if (err != UI_ERROR_NONE) {
    /* If signal creation fails, we don't have a specific free for arena blocks,
       but the caller will likely destroy the arena. We just return the error.
     */
    return err;
  }

  initial_payload.ptr_val = NULL;
  err = ui_signal_create(arena, initial_payload, UI_SIGNAL_TYPE_POINTER,
                         void_equality, NULL, UI_SIGNAL_MODE_SINGLE_THREADED,
                         &(*out_graph)->topology_signal);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a node graph widget.
 * \param[in,out] graph The node graph widget to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_node_graph_base_destroy(struct ui_node_graph_base *graph) {
  if (!graph)
    return UI_ERROR_INVALID_ARGUMENT;
  if (graph->camera_signal)
    (void)ui_signal_destroy(graph->camera_signal);
  if (graph->topology_signal)
    (void)ui_signal_destroy(graph->topology_signal);
  return UI_ERROR_NONE;
}

/**
 * \brief Pans the camera view.
 * \param[in,out] graph The node graph widget.
 * \param[in] delta_x X pan delta.
 * \param[in] delta_y Y pan delta.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_node_graph_base_pan(struct ui_node_graph_base *graph,
                                  float delta_x, float delta_y) {
  ui_error_t rc;
  if (!graph)
    return UI_ERROR_INVALID_ARGUMENT;

  graph->pan_x += delta_x;
  graph->pan_y += delta_y;

  /* Apply bounding box constraints if specified */
  if (graph->camera_config.bounds.width > 0 &&
      graph->camera_config.bounds.height > 0) {
    if (graph->pan_x < graph->camera_config.bounds.left)
      graph->pan_x = graph->camera_config.bounds.left;
    if (graph->pan_y < graph->camera_config.bounds.top)
      graph->pan_y = graph->camera_config.bounds.top;
    if (graph->pan_x > graph->camera_config.bounds.right)
      graph->pan_x = graph->camera_config.bounds.right;
    if (graph->pan_y > graph->camera_config.bounds.bottom)
      graph->pan_y = graph->camera_config.bounds.bottom;
  }

  rc = update_camera_matrix(graph);
  { (void)rc; }
  return UI_ERROR_NONE;
}

/**
 * \brief Zooms the camera view, optionally towards a specific focal point.
 * \param[in,out] graph The node graph widget.
 * \param[in] zoom The new zoom level.
 * \param[in] focal_point Optional focal point for the zoom operation.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_node_graph_base_zoom(struct ui_node_graph_base *graph, float zoom,
                                   const struct ui_dom_point *focal_point) {
  float old_zoom;
  float scale_factor;
  ui_error_t rc;

  if (!graph)
    return UI_ERROR_INVALID_ARGUMENT;

  old_zoom = graph->zoom;

  /* Clamp */
  if (zoom < graph->camera_config.min_zoom)
    zoom = graph->camera_config.min_zoom;
  if (zoom > graph->camera_config.max_zoom)
    zoom = graph->camera_config.max_zoom;

  graph->zoom = zoom;

  if (focal_point && old_zoom != 0.0f) {
    /* Offset pan to zoom towards focal point */
    scale_factor = zoom / old_zoom;
    graph->pan_x =
        focal_point->x - (focal_point->x - graph->pan_x) * scale_factor;
    graph->pan_y =
        focal_point->y - (focal_point->y - graph->pan_y) * scale_factor;
  }

  rc = update_camera_matrix(graph);
  { (void)rc; }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_node_graph_base_get_camera_signal(struct ui_node_graph_base *graph,
                                     ui_signal_t **out_signal) {
  if (!graph || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = graph->camera_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_node_graph_base_screen_to_graph(const struct ui_node_graph_base *graph,
                                   const struct ui_dom_point *screen_point,
                                   struct ui_dom_point *out_graph_point) {
  if (!graph || !screen_point || !out_graph_point)
    return UI_ERROR_INVALID_ARGUMENT;

  if (graph->zoom == 0.0f)
    return UI_ERROR_UNKNOWN;

  /* Inverse translation then inverse scale */
  out_graph_point->x = (screen_point->x - graph->pan_x) / graph->zoom;
  out_graph_point->y = (screen_point->y - graph->pan_y) / graph->zoom;
  out_graph_point->z = screen_point->z;
  out_graph_point->w = screen_point->w;

  return UI_ERROR_NONE;
}

/** \brief ui_node_graph_base_add_connection */
ui_error_t ui_node_graph_base_add_connection(
    struct ui_node_graph_base *graph,
    const struct ui_node_graph_connection *connection) {
  union ui_signal_payload payload;

  if (!graph || !connection)
    return UI_ERROR_INVALID_ARGUMENT;

  if (graph->num_connections >= UI_NODE_GRAPH_MAX_CONNECTIONS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  graph->connections[graph->num_connections++] = *connection;

  payload.ptr_val = NULL;
  return ui_signal_set(graph->topology_signal, payload);
}

/** \brief ui_node_graph_base_set_marquee_selection */
ui_error_t ui_node_graph_base_set_marquee_selection(
    struct ui_node_graph_base *graph,
    const struct ui_dom_rect *selection_rect) {
  union ui_signal_payload payload;

  if (!graph)
    return UI_ERROR_INVALID_ARGUMENT;

  if (selection_rect) {
    graph->current_marquee = *selection_rect;
    graph->has_marquee = UI_TRUE;
  } else {
    graph->has_marquee = UI_FALSE;
  }

  payload.ptr_val = NULL;
  return ui_signal_set(graph->topology_signal, payload);
}

/** \brief ui_error */
ui_error_t
ui_node_graph_base_get_topology_signal(struct ui_node_graph_base *graph,
                                       ui_signal_t **out_signal) {
  if (!graph || !out_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_signal = graph->topology_signal;
  return UI_ERROR_NONE;
}
