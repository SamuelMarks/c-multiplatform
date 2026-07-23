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
 * @brief Enum defining different vibrancy material types.
 */
enum ui_vibrancy_material {
  UI_VIBRANCY_MATERIAL_NONE,
  UI_VIBRANCY_MATERIAL_ACRYLIC,
  UI_VIBRANCY_MATERIAL_MICA,
  UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT,
  UI_VIBRANCY_MATERIAL_CUPERTINO_DARK
};

/**
 * @brief Parameters defining a vibrancy effect.
 */
struct ui_vibrancy_params {
  enum ui_vibrancy_material material;
  float blur_radius;
  float noise_opacity;
  float tint_color_rgba[4];
};

/**
 * @brief Initializes vibrancy parameters based on a known material preset.
 *
 * @param params Pointer to parameters to initialize.
 * @param material The preset material to apply.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_vibrancy_params_init(struct ui_vibrancy_params *params,
                                      enum ui_vibrancy_material material);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_VIBRANCY_H */
