#ifdef _MSC_VER
#pragma warning(disable : 4716)
#pragma warning(disable : 4702)
#endif

/* clang-format off */
#include "ui_map_view_base.h"
#include "ui_internal_mem.h"
#include "ui_dom_node.h"
#include <string.h>
#include <math.h>
/* clang-format on */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/** \brief ui_map_marker_entry */
struct ui_map_marker_entry {
  size_t id;
  struct ui_map_marker data;
};

/** \brief ui_map_view_base */
struct ui_map_view_base {
  struct ui_component *component;
  struct ui_signal *center_signal;
  struct ui_signal *zoom_signal;
  struct ui_signal *rotation_signal;

  /* State */
  struct ui_map_coordinate center;
  double zoom;
  double rotation;

  /* Tile provider */
  ui_map_tile_request_cb tile_cb;
  void *tile_user_data;

  /* Markers */
  struct ui_map_marker_entry *markers;
  size_t marker_capacity;
  size_t marker_count;
  size_t next_marker_id;
};

enum ui_error ui_map_view_base_create(struct ui_map_view_base **out_map) {
  struct ui_map_view_base *map;
  struct ui_dom_node *root_node = NULL;
  enum ui_error rc;

  if (!out_map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map = (struct ui_map_view_base *)UI_MALLOC(sizeof(struct ui_map_view_base));
  if (!map) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(map, 0, sizeof(struct ui_map_view_base));

  rc = ui_component_create(&map->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(map);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(map->component);
    UI_FREE(map);
    return rc;
  }

  ui_dom_node_set_tag_name(root_node, "div");
  ui_dom_node_set_attribute(root_node, "role", "application");
  map->component->shadow_root = root_node;

  map->zoom = 1.0;
  map->next_marker_id = 1;

  *out_map = map;
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_destroy(struct ui_map_view_base *map) {
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (map->component) {
    ui_component_destroy(map->component);
  }
  if (map->markers) {
    UI_FREE(map->markers);
  }
  UI_FREE(map);
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_bind_center(struct ui_map_view_base *map,
                                           struct ui_signal *signal) {
  if (!map || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->center_signal = signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_bind_zoom(struct ui_map_view_base *map,
                                         struct ui_signal *signal) {
  if (!map || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->zoom_signal = signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_bind_rotation(struct ui_map_view_base *map,
                                             struct ui_signal *signal) {
  if (!map || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->rotation_signal = signal;
  return UI_ERROR_NONE;
}

/* Helper functions for signal emission */
static enum ui_error emit_center(struct ui_map_view_base *map) {
  if (map->center_signal) {
    union ui_signal_payload payload;
    enum ui_error rc;
    memset(&payload, 0, sizeof(payload));
    payload.ptr_val = &map->center;
    rc = ui_signal_set(map->center_signal, payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

static enum ui_error emit_zoom(struct ui_map_view_base *map) {
  if (map->zoom_signal) {
    union ui_signal_payload payload;
    enum ui_error rc;
    memset(&payload, 0, sizeof(payload));
    payload.float_val = (float)map->zoom;
    rc = ui_signal_set(map->zoom_signal, payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

static enum ui_error emit_rotation(struct ui_map_view_base *map) {
  if (map->rotation_signal) {
    union ui_signal_payload payload;
    enum ui_error rc;
    memset(&payload, 0, sizeof(payload));
    payload.float_val = (float)map->rotation;
    rc = ui_signal_set(map->rotation_signal, payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

/* Note: Simple Web Mercator implementation */
static enum ui_error mercator_lon_to_x(double lon, double *out_x) {
  *out_x = (lon + 180.0) / 360.0;
  return UI_ERROR_NONE;
}

static enum ui_error mercator_lat_to_y(double lat, double *out_y) {
  double rad, y;
  rad = lat * M_PI / 180.0;
  y = log(tan(M_PI / 4.0 + rad / 2.0));
  *out_y = 0.5 - (y / (2.0 * M_PI));
  return UI_ERROR_NONE;
}

static enum ui_error mercator_x_to_lon(double x, double *out_lon) {
  *out_lon = x * 360.0 - 180.0;
  return UI_ERROR_NONE;
}

static enum ui_error mercator_y_to_lat(double y, double *out_lat) {
  double rad;
  rad = atan(exp(M_PI * (1.0 - 2.0 * y))) * 2.0 - M_PI / 2.0;
  *out_lat = rad * 180.0 / M_PI;
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_handle_pan(struct ui_map_view_base *map,
                                          double delta_x, double delta_y) {
  double cx, cy, cos_r, sin_r, rdx, rdy, map_size;
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map_size = 256.0 * pow(2.0, map->zoom);

  cos_r = cos(map->rotation);
  sin_r = sin(map->rotation);

  rdx = delta_x * cos_r - delta_y * sin_r;
  rdy = delta_x * sin_r + delta_y * cos_r;

  mercator_lon_to_x(map->center.longitude, &cx);
  mercator_lat_to_y(map->center.latitude, &cy);

  cx -= rdx / map_size;
  cy -= rdy / map_size;

  (void)mercator_x_to_lon(cx, &map->center.longitude);
  (void)mercator_y_to_lat(cy, &map->center.latitude);

  (void)emit_center(map);
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_handle_pinch(struct ui_map_view_base *map,
                                            double scale, double focal_x,
                                            double focal_y) {
  (void)focal_x;
  (void)focal_y;
  if (!map || scale <= 0.0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map->zoom += log(scale) / 0.6931471805599453; /* log(2) */
  if (map->zoom < 0.0)
    map->zoom = 0.0;

  (void)emit_zoom(map);
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_handle_rotate(struct ui_map_view_base *map,
                                             double angle, double focal_x,
                                             double focal_y) {
  (void)focal_x;
  (void)focal_y;
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  map->rotation += angle;
  (void)emit_rotation(map);
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_project(struct ui_map_view_base *map,
                                       const struct ui_map_coordinate *coord,
                                       double *out_x, double *out_y) {
  double mx, my, cx, cy, map_size, dx, dy, cos_r, sin_r;

  if (!map || !coord || !out_x || !out_y) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mercator_lon_to_x(coord->longitude, &mx);
  mercator_lat_to_y(coord->latitude, &my);
  mercator_lon_to_x(map->center.longitude, &cx);
  mercator_lat_to_y(map->center.latitude, &cy);

  map_size = 256.0 * pow(2.0, map->zoom);

  dx = (mx - cx) * map_size;
  dy = (my - cy) * map_size;

  cos_r = cos(map->rotation);
  sin_r = sin(map->rotation);

  *out_x = dx * cos_r + dy * sin_r;
  *out_y = -dx * sin_r + dy * cos_r;

  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_unproject(struct ui_map_view_base *map, double x,
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

  mercator_lon_to_x(map->center.longitude, &cx);
  mercator_lat_to_y(map->center.latitude, &cy);

  mx = cx + (dx / map_size);
  my = cy + (dy / map_size);

  mercator_x_to_lon(mx, &out_coord->longitude);
  mercator_y_to_lat(my, &out_coord->latitude);

  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_set_tile_provider(struct ui_map_view_base *map,
                                                 ui_map_tile_request_cb cb,
                                                 void *user_data) {
  if (!map) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  map->tile_cb = cb;
  map->tile_user_data = user_data;
  return UI_ERROR_NONE;
}

enum ui_error ui_map_view_base_add_marker(struct ui_map_view_base *map,
                                          const struct ui_map_marker *marker,
                                          size_t *out_id) {
  size_t new_cap;
  struct ui_map_marker_entry *new_arr;
  if (!map || !marker || !out_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (map->marker_count >= map->marker_capacity) {
    new_cap = map->marker_capacity == 0 ? 8 : map->marker_capacity * 2;
    new_arr = (struct ui_map_marker_entry *)UI_REALLOC(
        map->markers, new_cap * sizeof(struct ui_map_marker_entry));
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

enum ui_error ui_map_view_base_remove_marker(struct ui_map_view_base *map,
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

enum ui_error ui_map_view_base_get_marker_position(struct ui_map_view_base *map,
                                                   size_t id, double *out_x,
                                                   double *out_y) {
  size_t i;
  enum ui_error rc;
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
