/**
 * @file design_system_renderer.h
 * @brief Extensible architecture for multi-design-system screenshot generation.
 * Supports Material 3, Cupertino (iOS HIG), Fluent 2 (Microsoft), and future
 * systems.
 */

#ifndef DESIGN_SYSTEM_RENDERER_H
#define DESIGN_SYSTEM_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "rasterizer.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @struct design_system_options
 * @brief Options passed to each design system screenshot generator.
 */
struct design_system_options {
  const char *output_dir; /**< Absolute or relative directory for outputs */
  int is_dark;            /**< 1 for dark theme, 0 for light theme */
};

/**
 * @struct design_system_driver
 * @brief Pluggable driver interface for a design system screenshot generator.
 */
struct design_system_driver {
  const char *
      name; /**< Unique identifier (e.g. "material3", "cupertino", "fluent2") */
  const char
      *title; /**< Human-readable title (e.g. "Material 3 Design System") */
  const char *description; /**< Brief description of the design language */

  /**
   * @brief Renders the entire suite of component screenshots for this design
   * system.
   * @param opts Pointer to generation options.
   * @return UI_ERROR_NONE on success, or an error code.
   */
  ui_error_t (*render_all)(const struct design_system_options *opts);
};

/**
 * @brief Registers a design system driver with the screenshot engine.
 * @param driver Pointer to the driver structure.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
ui_error_t ds_registry_register(const struct design_system_driver *driver);

/**
 * @brief Finds a registered design system driver by name.
 * @param name Unique identifier name.
 * @param out_driver Pointer to receive driver.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_NOT_FOUND.
 */
ui_error_t ds_registry_find(const char *name,
                            const struct design_system_driver **out_driver);

/**
 * @brief Retrieves an array of all registered design system drivers.
 * @param out_drivers Pointer to receive the driver pointer array.
 * @param out_count Pointer to receive the driver count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t
ds_registry_get_all(const struct design_system_driver *const **out_drivers,
                    size_t *out_count);

/**
 * @brief Resets the driver registry.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ds_registry_reset(void);

/* Driver forward declarations */
extern const struct design_system_driver g_driver_material3;
extern const struct design_system_driver g_driver_cupertino;
extern const struct design_system_driver g_driver_fluent2;

#ifdef __cplusplus
}
#endif

#endif /* DESIGN_SYSTEM_RENDERER_H */
