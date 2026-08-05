#ifndef UI_CHART_BASE_H
#define UI_CHART_BASE_H

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
 * @brief Types of coordinate systems.
 */
enum ui_chart_coordinate_system {
  UI_CHART_COORDINATE_CARTESIAN = 0, /* X/Y plane */
  UI_CHART_COORDINATE_POLAR = 1      /* Angle/Radius plane */
};

/**
 * @brief Types of scales used to map abstract values to physical pixels.
 */
enum ui_chart_scale_type {
  UI_CHART_SCALE_LINEAR = 0,
  UI_CHART_SCALE_LOGARITHMIC = 1,
  UI_CHART_SCALE_TIME = 2
};

/**
 * @brief Abstract representation of an axis scale configuration.
 */
struct ui_chart_scale_config {
  enum ui_chart_scale_type type;
  double domain_min; /**< Minimum data value */
  double domain_max; /**< Maximum data value */
  double range_min;  /**< Minimum physical pixel mapping */
  double range_max;  /**< Maximum physical pixel mapping */
};

/**
 * @brief Opaque handle for the chart base component.
 */
struct ui_chart_base;

/**
 * @brief Creates a chart base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param coord_system The type of coordinate system to use.
 * @param out_chart Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_chart_base_create(struct ui_arena *arena,
                                enum ui_chart_coordinate_system coord_system,
                                struct ui_chart_base **out_chart);

/**
 * @brief Destroys a chart base component.
 *
 * @param chart The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_chart_base_destroy(struct ui_chart_base *chart);

/**
 * @brief Configures the primary X (or Angle) axis scale.
 *
 * @param chart The component.
 * @param config The scale configuration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_chart_base_set_x_scale(struct ui_chart_base *chart,
                          const struct ui_chart_scale_config *config);

/**
 * @brief Configures the primary Y (or Radius) axis scale.
 *
 * @param chart The component.
 * @param config The scale configuration.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_chart_base_set_y_scale(struct ui_chart_base *chart,
                          const struct ui_chart_scale_config *config);

/**
 * @brief Translates an abstract data coordinate into a physical pixel location
 * based on the configured scales and coordinate system.
 *
 * @param chart The component.
 * @param data_x The abstract X or Angle value.
 * @param data_y The abstract Y or Radius value.
 * @param out_pixel_point Pointer to receive the physical coordinate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_chart_base_data_to_pixel(const struct ui_chart_base *chart,
                                       double data_x, double data_y,
                                       struct ui_dom_point *out_pixel_point);

/**
 * @brief Performs a hit test, translating a physical pixel location back into
 * the abstract data domain. Useful for tooltips and interactions.
 *
 * @param chart The component.
 * @param pixel_x The physical X coordinate.
 * @param pixel_y The physical Y coordinate.
 * @param out_data_x Pointer to receive the mapped data X or Angle value.
 * @param out_data_y Pointer to receive the mapped data Y or Radius value.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if the pixel is
 * outside the chart bounds.
 */
ui_error_t ui_chart_base_pixel_to_data(const struct ui_chart_base *chart,
                                       double pixel_x, double pixel_y,
                                       double *out_data_x, double *out_data_y);

/**
 * @brief Sets the physical bounding box where the actual chart data is drawn
 * (excluding margins for axes and legends).
 *
 * @param chart The component.
 * @param bounds The drawing area boundaries.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_chart_base_set_draw_bounds(struct ui_chart_base *chart,
                                         const struct ui_dom_rect *bounds);

/**
 * @brief Retrieves the signal emitted when a scale or bound changes,
 * necessitating a chart redraw. Payload is NULL.
 *
 * @param chart The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_chart_base_get_topology_signal(struct ui_chart_base *chart,
                                             ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CHART_BASE_H */
