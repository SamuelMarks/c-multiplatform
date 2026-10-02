/**
 * @file cupertino_control_center.h
 * @brief iOS 18+ Control Center Modular System Controls conforming to Apple
 * HIG.
 */

#ifndef CUPERTINO_CUPERTINO_CONTROL_CENTER_H
#define CUPERTINO_CUPERTINO_CONTROL_CENTER_H

/* clang-format off */
#include "ui_button_base.h"
#include "ui_slider_base.h"
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_CONTROL_CENTER_MAX_GRID_MODULES 32

/**
 * @enum cupertino_control_center_layout
 * @brief Modular layout geometries defined by iOS 18 Control Center HIG.
 */
enum cupertino_control_center_layout {
  CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_CIRCULAR =
      0, /**< 1x1 fully circular button (60x60pt, 30pt radius). */
  CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_SQUIRCLE, /**< 1x1 squircle button
                                                   (60x60pt, 18pt radius). */
  CUPERTINO_CONTROL_CENTER_LAYOUT_2X1_PILL,     /**< 2x1 pill button (128x60pt,
                                                   18pt radius). */
  CUPERTINO_CONTROL_CENTER_LAYOUT_2X2_MACRO     /**< 2x2 macro square card
                                                   (128x128pt, 20pt radius). */
};

/**
 * @struct cupertino_control_center_module_descriptor
 * @brief Initialization descriptor for a Control Center modular control.
 */
struct cupertino_control_center_module_descriptor {
  enum cupertino_control_center_layout layout; /**< Control geometry type. */
  const char *title;            /**< Primary title label (or NULL). */
  const char *subtitle;         /**< Subtitle status label (or NULL). */
  const char *symbol_name;      /**< Initial SF Symbol name. */
  int is_active;                /**< 1 if toggled ON initially, 0 if OFF. */
  int has_slider;               /**< 1 if module supports slider expansion. */
  float initial_slider_value;   /**< Initial slider fraction [0.0, 1.0]. */
  ui_color_t active_tint_color; /**< Custom active tint color (or 0 for system
                                   blue). */
};

/**
 * @struct cupertino_control_center_module
 * @brief Represents an individual Control Center modular widget instance.
 */
struct cupertino_control_center_module {
  enum cupertino_control_center_layout layout; /**< Geometry mode. */
  char title[64];                              /**< Title string. */
  char subtitle[64];                           /**< Subtitle string. */
  char symbol_name[64];         /**< Current SF Symbol identifier. */
  char symbol_morph_target[64]; /**< Target SF Symbol for vector morph. */
  float symbol_morph_progress;  /**< Interpolation progress [0.0, 1.0]. */
  int is_active;                /**< Active state toggle flag. */
  int has_slider;               /**< Flag indicating slider capability. */
  int is_expanded;              /**< 1 if currently expanded via long-press. */
  float slider_value;           /**< Current slider value [0.0, 1.0]. */
  float width;                  /**< Rendered width in points. */
  float height;                 /**< Rendered height in points. */
  float corner_radius;          /**< Corner radius in points. */
  ui_color_t active_tint;       /**< Active tint color. */
  ui_color_t background_color;  /**< Background surface color. */
  struct ui_button_base *button_base; /**< Wrapped button primitive. */
  struct ui_slider_base *slider_base; /**< Wrapped slider primitive if enabled.
                                       */
};

/**
 * @struct cupertino_control_center_grid_item
 * @brief Internal position slot within the Control Center modular grid.
 */
struct cupertino_control_center_grid_item {
  struct cupertino_control_center_module *module; /**< Hosted module. */
  int col;                                        /**< Grid column index. */
  int row;                                        /**< Grid row index. */
};

/**
 * @struct cupertino_control_center_grid
 * @brief Container grid managing a collection of Control Center modules.
 */
struct cupertino_control_center_grid {
  struct cupertino_control_center_grid_item
      items[CUPERTINO_CONTROL_CENTER_MAX_GRID_MODULES]; /**< Hosted items. */
  size_t item_count; /**< Number of placed modules. */
  float padding;     /**< Outer edge padding in points (standard 16pt). */
  float spacing;     /**< Gap between modules in points (standard 14pt). */
};

/**
 * @brief Creates a new Control Center modular control.
 *
 * @param engine Pointer to ui_engine instance.
 * @param desc Configuration descriptor.
 * @param out_module Pointer to receive allocated module.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_control_center_module_create(
    struct ui_engine *engine,
    const struct cupertino_control_center_module_descriptor *desc,
    struct cupertino_control_center_module **out_module);

/**
 * @brief Destroys a Control Center modular control and frees resources.
 *
 * @param module Module to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_destroy(
    struct cupertino_control_center_module *module);

/**
 * @brief Sets the active/toggled state of the module.
 * Emits a haptic impact pattern on state flip.
 *
 * @param module Target module.
 * @param is_active 1 for active ON, 0 for inactive OFF.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_set_active(
    struct cupertino_control_center_module *module, int is_active);

/**
 * @brief Gets whether the module is currently active.
 *
 * @param module Target module.
 * @param out_is_active Pointer to receive active flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_is_active(
    const struct cupertino_control_center_module *module, int *out_is_active);

/**
 * @brief Toggles the module active state, firing haptic feedback.
 *
 * @param module Target module.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_control_center_module_toggle(
    struct cupertino_control_center_module *module);

/**
 * @brief Sets whether the module is expanded into its long-press state.
 *
 * @param module Target module.
 * @param is_expanded 1 if expanded, 0 if normal.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_set_expanded(
    struct cupertino_control_center_module *module, int is_expanded);

/**
 * @brief Gets whether the module is expanded.
 *
 * @param module Target module.
 * @param out_is_expanded Pointer to receive expansion flag.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_is_expanded(
    const struct cupertino_control_center_module *module, int *out_is_expanded);

/**
 * @brief Sets the slider fraction for expandable modules.
 *
 * @param module Target module.
 * @param value Fraction in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_set_slider_value(
    struct cupertino_control_center_module *module, float value);

/**
 * @brief Gets the slider value.
 *
 * @param module Target module.
 * @param out_value Pointer to receive slider fraction.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_get_slider_value(
    const struct cupertino_control_center_module *module, float *out_value);

/**
 * @brief Sets the animated SF Symbol vector morph target and progress.
 *
 * @param module Target module.
 * @param symbol_from Initial symbol name.
 * @param symbol_to Destination symbol name.
 * @param progress Morph interpolation progress in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_set_symbol_morph(
    struct cupertino_control_center_module *module, const char *symbol_from,
    const char *symbol_to, float progress);

/**
 * @brief Gets the active SF Symbol morph configuration.
 *
 * @param module Target module.
 * @param out_from Pointer to receive pointer to starting symbol.
 * @param out_to Pointer to receive pointer to target symbol.
 * @param out_progress Pointer to receive interpolation progress.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_get_symbol_morph(
    const struct cupertino_control_center_module *module, const char **out_from,
    const char **out_to, float *out_progress);

/**
 * @brief Gets the bounding box and continuous corner radius of the module.
 *
 * @param module Target module.
 * @param out_w Pointer to receive width in points.
 * @param out_h Pointer to receive height in points.
 * @param out_radius Pointer to receive continuous corner radius in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_get_bounds(
    const struct cupertino_control_center_module *module, float *out_w,
    float *out_h, float *out_radius);

/**
 * @brief Gets the wrapped CDK button base primitive.
 *
 * @param module Target module.
 * @param out_button Pointer to receive wrapped button.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_get_button_base(
    const struct cupertino_control_center_module *module,
    struct ui_button_base **out_button);

/**
 * @brief Gets the wrapped CDK slider base primitive if available.
 *
 * @param module Target module.
 * @param out_slider Pointer to receive wrapped slider.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_module_get_slider_base(
    const struct cupertino_control_center_module *module,
    struct ui_slider_base **out_slider);

/**
 * @brief Creates a Control Center container grid.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_grid Pointer to receive allocated grid.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_control_center_grid_create(
    struct ui_engine *engine, struct cupertino_control_center_grid **out_grid);

/**
 * @brief Destroys a Control Center grid.
 * Note: hosted modules are not destroyed automatically.
 *
 * @param grid Grid to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_control_center_grid_destroy(
    struct cupertino_control_center_grid *grid);

/**
 * @brief Adds a module to the grid at specified column and row.
 *
 * @param grid Target grid.
 * @param module Module to add.
 * @param col Zero-based column index.
 * @param row Zero-based row index.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_grid_add_module(
    struct cupertino_control_center_grid *grid,
    struct cupertino_control_center_module *module, int col, int row);

/**
 * @brief Gets the number of modules registered in the grid.
 *
 * @param grid Target grid.
 * @param out_count Pointer to receive module count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_grid_get_module_count(
    const struct cupertino_control_center_grid *grid, size_t *out_count);

/**
 * @brief Gets the module at the specified index.
 *
 * @param grid Target grid.
 * @param index Zero-based item index.
 * @param out_module Pointer to receive module.
 * @param out_col Pointer to receive column (or NULL).
 * @param out_row Pointer to receive row (or NULL).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_grid_get_module_at(
    const struct cupertino_control_center_grid *grid, size_t index,
    struct cupertino_control_center_module **out_module, int *out_col,
    int *out_row);

/**
 * @brief Computes total grid bounds based on placed modules and spacing.
 *
 * @param grid Target grid.
 * @param out_w Pointer to receive computed width in points.
 * @param out_h Pointer to receive computed height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_control_center_grid_get_bounds(
    const struct cupertino_control_center_grid *grid, float *out_w,
    float *out_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_CONTROL_CENTER_H */
