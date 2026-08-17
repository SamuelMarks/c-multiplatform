#ifndef UI_VIBRANCY_H
#define UI_VIBRANCY_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum ui_vibrancy_material
 * @brief Enum defining different vibrancy material types.
 */
enum ui_vibrancy_material {
  /** @brief No vibrancy effect. */
  UI_VIBRANCY_MATERIAL_NONE,
  /** @brief Standard acrylic effect. */
  UI_VIBRANCY_MATERIAL_ACRYLIC,
  /** @brief Standard mica effect. */
  UI_VIBRANCY_MATERIAL_MICA,
  /** @brief Light cupertino/glass effect. */
  UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT,
  /** @brief Dark cupertino/glass effect. */
  UI_VIBRANCY_MATERIAL_CUPERTINO_DARK
};

/**
 * @struct ui_vibrancy_params
 * @brief Parameters defining a vibrancy effect.
 */
struct ui_vibrancy_params {
  /** @brief The base material to use. */
  enum ui_vibrancy_material material;
  /** @brief The blur radius in pixels. */
  float blur_radius;
  /** @brief The opacity of the noise overlay (0.0 to 1.0). */
  float noise_opacity;
  /** @brief RGBA tint color to overlay. */
  float tint_color_rgba[4];
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIBRANCY_H */
