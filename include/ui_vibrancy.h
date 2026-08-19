/**
 * @file ui_vibrancy.h
 * @brief Types and initialization for platform UI vibrancy effects.
 */

#ifndef UI_VIBRANCY_H
#define UI_VIBRANCY_H

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ui_vibrancy Vibrancy
 * @brief Platform-agnostic definitions for background vibrancy/acrylic effects.
 * @{
 */

/**
 * @enum ui_vibrancy_material
 * @brief Enum defining different vibrancy material types.
 */
enum ui_vibrancy_material {
  UI_VIBRANCY_MATERIAL_NONE,            /**< No vibrancy effect. */
  UI_VIBRANCY_MATERIAL_ACRYLIC,         /**< Standard acrylic effect. */
  UI_VIBRANCY_MATERIAL_MICA,            /**< Standard mica effect. */
  UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT, /**< Light cupertino/glass effect. */
  UI_VIBRANCY_MATERIAL_CUPERTINO_DARK   /**< Dark cupertino/glass effect. */
};

/**
 * @struct ui_vibrancy_params
 * @brief Parameters defining a vibrancy effect.
 */
struct ui_vibrancy_params {
  enum ui_vibrancy_material material; /**< The base material to use. */
  float blur_radius;                  /**< The blur radius in pixels. */
  float noise_opacity; /**< The opacity of the noise overlay (0.0 to 1.0). */
  float tint_color_rgba[4]; /**< RGBA tint color to overlay. */
};

/**
 * @brief Initializes vibrancy parameters based on a known material preset.
 *
 * @param params Pointer to parameters to initialize.
 * @param material The preset material to apply.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_vibrancy_params_init(struct ui_vibrancy_params *params,
                                   enum ui_vibrancy_material material);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIBRANCY_H */
