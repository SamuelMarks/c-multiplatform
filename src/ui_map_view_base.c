/**
 * @file ui_map_view_base.c
 * @brief Implementation of the UI map view base component.
 * @details Provides the core map view functionality.
 */
#ifdef _MSC_VER
#endif

/* clang-format off */
#include "ui_map_view_base.h"
#include "ui_internal_mem.h"
#include "ui_dom_node.h"
#include <string.h>
#include <math.h>
/* clang-format on */

#ifndef M_PI
/** @brief internal Pi value */
#define M_PI 3.14159265358979323846
#endif

/**
 * @struct ui_map_marker_entry
 * @brief Map marker entry data structure.
 * @details Represents an entry for a marker in the map view.
 */
struct ui_map_marker_entry {
  size_t id;                 /**< Marker ID */
  struct ui_map_marker data; /**< Marker data */
};

/**
 * @struct ui_map_view_base
 * @brief Map view base structure.
 * @details Holds the internal state and DOM mapping for the map view.
 */
struct ui_map_view_base {
  struct ui_component *component;    /**< UI component */
  struct ui_signal *center_signal;   /**< Center coordinate signal */
  struct ui_signal *zoom_signal;     /**< Zoom level signal */
  struct ui_signal *rotation_signal; /**< Rotation angle signal */

  /* State */
  struct ui_map_coordinate center; /**< Center coordinate */
  double zoom;                     /**< Zoom level */
  double rotation;                 /**< Rotation angle */

  /* Tile provider */
  ui_map_tile_request_cb tile_cb; /**< Tile provider callback */
  void *tile_user_data;           /**< Tile user data */

  /* Markers */
  struct ui_map_marker_entry *markers; /**< Array of markers */
  size_t marker_capacity;              /**< Marker array capacity */
  size_t marker_count;                 /**< Current marker count */
  size_t next_marker_id;               /**< Next marker ID to issue */
};

/**
 * @brief Creates a new map view base widget.
 * @param[out] out_map Pointer to store the created map view.
 * @return UI_ERROR_NONE on success, or a corresponding error code.
 */
ui_error_t ui_map_view_base_create(struct ui_map_view_base **out_map) {
  struct ui_map_view_base *map;
  struct ui_dom_node *root_node = NULL;
  ui_error_t rc;

  if (!out_map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map = (struct ui_map_view_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_map_view_base));
  if (!map) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(map, 0, sizeof(struct ui_map_view_base));

  rc = ui_component_create(&map->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(map);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(map->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(map);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_component_destroy(map->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(map);
    return rc;
  }
  rc = ui_dom_node_set_attribute(root_node, "role", "application");
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_component_destroy(map->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(map);
    return rc;
  }
  map->component->shadow_root = root_node;

  map->zoom = 1.0;
  map->next_marker_id = 1;

  *out_map = map;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a map view base widget.
 * @param[in,out] map The map view to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_destroy(struct ui_map_view_base *map) {
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(map->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (map->markers) {
    C_MULTIPLATFORM_FREE(map->markers);
  }
  C_MULTIPLATFORM_FREE(map);
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the center coordinate to a reactive signal.
 * @param[in,out] map The map view widget.
 * @param[in,out] signal The signal representing the center coordinate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_bind_center(struct ui_map_view_base *map,
                                        struct ui_signal *signal) {
  if (!map || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->center_signal = signal;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the zoom level to a reactive signal.
 * @param[in,out] map The map view widget.
 * @param[in,out] signal The signal representing the zoom level.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_bind_zoom(struct ui_map_view_base *map,
                                      struct ui_signal *signal) {
  if (!map || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->zoom_signal = signal;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the rotation angle to a reactive signal.
 * @param[in,out] map The map view widget.
 * @param[in,out] signal The signal representing the rotation angle.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_bind_rotation(struct ui_map_view_base *map,
                                          struct ui_signal *signal) {
  if (!map || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->rotation_signal = signal;
  return UI_ERROR_NONE;
}

/**
 * @brief Emits the current center coordinate to the bound signal.
 * @param[in,out] map The map view widget.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t emit_center(struct ui_map_view_base *map) {
  if (map->center_signal) {
    union ui_signal_payload payload;
    ui_error_t rc;
    memset(&payload, 0, sizeof(payload));
    payload.ptr_val = &map->center;
    rc = ui_signal_set(map->center_signal, payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Emits the current zoom level to the bound signal.
 * @param[in,out] map The map view widget.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t emit_zoom(struct ui_map_view_base *map) {
  if (map->zoom_signal) {
    union ui_signal_payload payload;
    ui_error_t rc;
    memset(&payload, 0, sizeof(payload));
    payload.float_val = (float)map->zoom;
    rc = ui_signal_set(map->zoom_signal, payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Emits the current rotation angle to the bound signal.
 * @param[in,out] map The map view widget.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t emit_rotation(struct ui_map_view_base *map) {
  if (map->rotation_signal) {
    union ui_signal_payload payload;
    ui_error_t rc;
    memset(&payload, 0, sizeof(payload));
    payload.float_val = (float)map->rotation;
    rc = ui_signal_set(map->rotation_signal, payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Converts longitude to Web Mercator X coordinate.
 * @param[in] lon Longitude in degrees.
 * @return The X coordinate (0.0 to 1.0).
 */
static double mercator_lon_to_x(double lon) { return (lon + 180.0) / 360.0; }

/**
 * @brief Converts latitude to Web Mercator Y coordinate.
 * @param[in] lat Latitude in degrees.
 * @return The Y coordinate (0.0 to 1.0).
 */
static double mercator_lat_to_y(double lat) {
  double rad = lat * M_PI / 180.0;
  double y = log(tan(M_PI / 4.0 + rad / 2.0));
  return 0.5 - (y / (2.0 * M_PI));
}

/**
 * @brief Converts Web Mercator X coordinate back to longitude.
 * @param[in] x The X coordinate (0.0 to 1.0).
 * @return Longitude in degrees.
 */
static double mercator_x_to_lon(double x) { return x * 360.0 - 180.0; }

/**
 * @brief Converts Web Mercator Y coordinate back to latitude.
 * @param[in] y The Y coordinate (0.0 to 1.0).
 * @return Latitude in degrees.
 */
static double mercator_y_to_lat(double y) {
  double n = M_PI - 2.0 * M_PI * y;
  return (180.0 / M_PI) * atan(0.5 * (exp(n) - exp(-n)));
}

/**
 * @brief Handles a panning interaction, updating the map center.
 * @param[in,out] map The map view widget.
 * @param[in] delta_x Pan delta on the X axis in pixels.
 * @param[in] delta_y Pan delta on the Y axis in pixels.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_handle_pan(struct ui_map_view_base *map,
                                       double delta_x, double delta_y) {
  double cx, cy, cos_r, sin_r, rdx, rdy, map_size;
  ui_error_t rc;
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map_size = 256.0 * pow(2.0, map->zoom);

  cos_r = cos(map->rotation);
  sin_r = sin(map->rotation);

  rdx = delta_x * cos_r - delta_y * sin_r;
  rdy = delta_x * sin_r + delta_y * cos_r;

  cx = mercator_lon_to_x(map->center.longitude);
  cy = mercator_lat_to_y(map->center.latitude);

  cx -= rdx / map_size;
  cy -= rdy / map_size;

  map->center.longitude = mercator_x_to_lon(cx);
  map->center.latitude = mercator_y_to_lat(cy);

  rc = emit_center(map);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

/**
 * @brief Handles a pinch interaction, updating the zoom level.
 * @param[in,out] map The map view widget.
 * @param[in] scale The pinch scale multiplier.
 * @param[in] focal_x The focal point X coordinate.
 * @param[in] focal_y The focal point Y coordinate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_handle_pinch(struct ui_map_view_base *map,
                                         double scale, double focal_x,
                                         double focal_y) {
  ui_error_t rc;
  (void)focal_x;
  (void)focal_y;
  if (!map || scale <= 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map->zoom += log(scale) / 0.6931471805599453; /* log(2) */
  if (map->zoom < 0.0)
    map->zoom = 0.0;

  rc = emit_zoom(map);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

/**
 * @brief Handles a rotation interaction.
 * @param[in,out] map The map view widget.
 * @param[in] angle The rotation angle in radians.
 * @param[in] focal_x The focal point X coordinate.
 * @param[in] focal_y The focal point Y coordinate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_handle_rotate(struct ui_map_view_base *map,
                                          double angle, double focal_x,
                                          double focal_y) {
  ui_error_t rc;
  (void)focal_x;
  (void)focal_y;
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map->rotation += angle;
  rc = emit_rotation(map);
  if (rc != UI_ERROR_NONE)
    return rc;
  return UI_ERROR_NONE;
}

/**
 * @brief Projects a geographic coordinate to pixel coordinates relative to the
 * map center.
 * @param[in] map The map view widget.
 * @param[in] coord The geographic coordinate.
 * @param[out] out_x Pointer to store the X pixel coordinate.
 * @param[out] out_y Pointer to store the Y pixel coordinate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_project(struct ui_map_view_base *map,
                                    const struct ui_map_coordinate *coord,
                                    double *out_x, double *out_y) {
  double mx, my, cx, cy, map_size, dx, dy, cos_r, sin_r;

  if (!map || !coord || !out_x || !out_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mx = mercator_lon_to_x(coord->longitude);
  my = mercator_lat_to_y(coord->latitude);
  cx = mercator_lon_to_x(map->center.longitude);
  cy = mercator_lat_to_y(map->center.latitude);

  map_size = 256.0 * pow(2.0, map->zoom);

  dx = (mx - cx) * map_size;
  dy = (my - cy) * map_size;

  cos_r = cos(map->rotation);
  sin_r = sin(map->rotation);

  *out_x = dx * cos_r + dy * sin_r;
  *out_y = -dx * sin_r + dy * cos_r;

  return UI_ERROR_NONE;
}

/**
 * @brief Unprojects pixel coordinates back to a geographic coordinate.
 * @param[in] map The map view widget.
 * @param[in] x The X pixel coordinate relative to the map center.
 * @param[in] y The Y pixel coordinate relative to the map center.
 * @param[out] out_coord Pointer to store the geographic coordinate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_unproject(struct ui_map_view_base *map, double x,
                                      double y,
                                      struct ui_map_coordinate *out_coord) {
  double dx, dy, cos_r, sin_r, cx, cy, mx, my, map_size;

  if (!map || !out_coord) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cos_r = cos(-map->rotation);
  sin_r = sin(-map->rotation);

  dx = x * cos_r + y * sin_r;
  dy = -x * sin_r + y * cos_r;

  map_size = 256.0 * pow(2.0, map->zoom);

  cx = mercator_lon_to_x(map->center.longitude);
  cy = mercator_lat_to_y(map->center.latitude);

  mx = cx + (dx / map_size);
  my = cy + (dy / map_size);

  out_coord->longitude = mercator_x_to_lon(mx);
  out_coord->latitude = mercator_y_to_lat(my);

  return UI_ERROR_NONE;
}

/**
 * @brief Sets the callback for requesting map tiles.
 * @param[in,out] map The map view widget.
 * @param[in] cb The tile request callback.
 * @param[in] user_data User data for the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_set_tile_provider(struct ui_map_view_base *map,
                                              ui_map_tile_request_cb cb,
                                              void *user_data) {
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->tile_cb = cb;
  map->tile_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief Adds a geographic marker to the map.
 * @param[in,out] map The map view widget.
 * @param[in] marker The marker data.
 * @param[out] out_id Pointer to store the internal ID of the new marker.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_map_view_base_add_marker(struct ui_map_view_base *map,
                                       const struct ui_map_marker *marker,
                                       size_t *out_id) {
  size_t new_cap;
  struct ui_map_marker_entry *new_arr;
  if (!map || !marker || !out_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (map->marker_count >= map->marker_capacity) {
    new_cap = map->marker_capacity == 0 ? 8 : map->marker_capacity * 2;
    new_arr = (struct ui_map_marker_entry *)C_MULTIPLATFORM_REALLOC(
        map->markers, (size_t)new_cap * sizeof(struct ui_map_marker_entry));
    if (!new_arr) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    map->markers = new_arr;
    map->marker_capacity = new_cap;
  }

  map->markers[map->marker_count].id = map->next_marker_id;
  map->markers[map->marker_count].data = *marker;
  *out_id = map->next_marker_id;

  map->marker_count++;
  map->next_marker_id++;

  return UI_ERROR_NONE;
}

/**
 * @brief Removes a geographic marker from the map by its ID.
 * @param[in,out] map The map view widget.
 * @param[in] id The internal ID of the marker to remove.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t ui_map_view_base_remove_marker(struct ui_map_view_base *map,
                                          size_t id) {
  size_t i, j;
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < map->marker_count; i++) {
    if (map->markers[i].id == id) {
      for (j = i; j < map->marker_count - 1; j++) {
        map->markers[j] = map->markers[j + 1];
      }
      map->marker_count--;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Gets the current projected pixel position of a marker.
 * @param[in] map The map view widget.
 * @param[in] id The internal ID of the marker.
 * @param[out] out_x Pointer to store the X pixel coordinate.
 * @param[out] out_y Pointer to store the Y pixel coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t ui_map_view_base_get_marker_position(struct ui_map_view_base *map,
                                                size_t id, double *out_x,
                                                double *out_y) {
  size_t i;
  ui_error_t rc;
  if (!map || !out_x || !out_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < map->marker_count; i++) {
    if (map->markers[i].id == id) {
      rc = ui_map_view_base_project(map, &map->markers[i].data.coordinate,
                                    out_x, out_y);
      return rc;
    }
  }

  return UI_ERROR_NOT_FOUND;
}
