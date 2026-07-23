#ifndef UI_MAP_VIEW_BASE_H
#define UI_MAP_VIEW_BASE_H

/* clang-format off */
#include "ui_component.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_geometry.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a geographic coordinate.
 */
struct ui_map_coordinate {
  double latitude;
  double longitude;
};

/**
 * @brief Represents a marker on the map.
 */
struct ui_map_marker {
  struct ui_map_coordinate coordinate;
  void *user_data;
};

struct ui_map_view_base;

/**
 * @brief Creates a new map view base instance.
 *
 * @param out_map Pointer to receive the map view base.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_create(struct ui_map_view_base **out_map);

/**
 * @brief Destroys a map view base instance.
 *
 * @param map The map view base.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_destroy(struct ui_map_view_base *map);

/**
 * @brief Binds a signal to the map's center coordinate.
 *
 * @param map The map view base.
 * @param signal The signal (must contain pointer to struct ui_map_coordinate).
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_bind_center(struct ui_map_view_base *map,
                                           struct ui_signal *signal);

/**
 * @brief Binds a signal to the map's zoom level.
 *
 * @param map The map view base.
 * @param signal The signal (must contain float32).
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_bind_zoom(struct ui_map_view_base *map,
                                         struct ui_signal *signal);

/**
 * @brief Binds a signal to the map's rotation (in radians).
 *
 * @param map The map view base.
 * @param signal The signal (must contain float32).
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_bind_rotation(struct ui_map_view_base *map,
                                             struct ui_signal *signal);

/**
 * @brief Handles a pan gesture, logically translating the map center.
 *
 * @param map The map view base.
 * @param delta_x X-axis delta in logical pixels.
 * @param delta_y Y-axis delta in logical pixels.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_handle_pan(struct ui_map_view_base *map,
                                          double delta_x, double delta_y);

/**
 * @brief Handles a pinch-to-zoom gesture.
 *
 * @param map The map view base.
 * @param scale The scale multiplier.
 * @param focal_x X-axis focal point.
 * @param focal_y Y-axis focal point.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_handle_pinch(struct ui_map_view_base *map,
                                            double scale, double focal_x,
                                            double focal_y);

/**
 * @brief Handles a rotate gesture.
 *
 * @param map The map view base.
 * @param angle The rotation angle delta in radians.
 * @param focal_x X-axis focal point.
 * @param focal_y Y-axis focal point.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_handle_rotate(struct ui_map_view_base *map,
                                             double angle, double focal_x,
                                             double focal_y);

/**
 * @brief Projects a geographic coordinate into view-space pixels using Web
 * Mercator.
 *
 * @param map The map view base.
 * @param coord The geographic coordinate.
 * @param out_x Pointer to receive X view-space coordinate.
 * @param out_y Pointer to receive Y view-space coordinate.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_project(struct ui_map_view_base *map,
                                       const struct ui_map_coordinate *coord,
                                       double *out_x, double *out_y);

/**
 * @brief Unprojects view-space pixels into a geographic coordinate.
 *
 * @param map The map view base.
 * @param x The X view-space coordinate.
 * @param y The Y view-space coordinate.
 * @param out_coord Pointer to receive the geographic coordinate.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_unproject(struct ui_map_view_base *map, double x,
                                         double y,
                                         struct ui_map_coordinate *out_coord);

/**
 * @brief Tile request callback definition.
 */
typedef enum ui_error (*ui_map_tile_request_cb)(struct ui_map_view_base *map,
                                                int x, int y, int z,
                                                void *user_data);

/**
 * @brief Sets the tile provider callback to stream and cache map tiles.
 *
 * @param map The map view base.
 * @param cb The callback.
 * @param user_data User data.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_set_tile_provider(struct ui_map_view_base *map,
                                                 ui_map_tile_request_cb cb,
                                                 void *user_data);

/**
 * @brief Adds an unstyled overlay marker to track coordinates.
 *
 * @param map The map view base.
 * @param marker The marker data.
 * @param out_id Pointer to receive the generated marker ID.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_add_marker(struct ui_map_view_base *map,
                                          const struct ui_map_marker *marker,
                                          size_t *out_id);

/**
 * @brief Removes an existing overlay marker.
 *
 * @param map The map view base.
 * @param id The marker ID.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_remove_marker(struct ui_map_view_base *map,
                                             size_t id);

/**
 * @brief Retrieves the projected view-space position of a marker.
 *
 * @param map The map view base.
 * @param id The marker ID.
 * @param out_x Pointer to receive X coordinate.
 * @param out_y Pointer to receive Y coordinate.
 * @return enum ui_error
 */
enum ui_error ui_map_view_base_get_marker_position(struct ui_map_view_base *map,
                                                   size_t id, double *out_x,
                                                   double *out_y);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_MAP_VIEW_BASE_H */
