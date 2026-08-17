#ifndef UI_SURFACE_BASE_H
#define UI_SURFACE_BASE_H

struct ui_computed;

struct ui_signal;

/* clang-format off */
#include "ui_component.h"
#include "ui_elevation.h"
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_surface_base
 * @brief Represents an unstyled generic surface/paper container.
 */
struct ui_surface_base {
  /** @brief The base component. */
  struct ui_component base;
  /** @brief The material elevation level. */
  enum ui_elevation_level elevation;
  /** @brief The signal controlling the surface data. */
  struct ui_signal *data_signal;
};

/**
 * @brief Creates a new unstyled surface base component.
 *
 * @param out_surface Pointer to output the initialized surface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_surface_base_create(struct ui_surface_base **out_surface);

/**
 * @brief Sets the elevation level of the surface.
 *
 * @param surface The surface component.
 * @param level The new elevation level.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_surface_base_set_elevation(struct ui_surface_base *surface,
                                         enum ui_elevation_level level);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_surface_base_bind_data(struct ui_surface_base *widget,
                                     struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SURFACE_BASE_H */
