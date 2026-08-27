/* clang-format off */
#include "ui_vibrancy.h"
/* clang-format on */

/*
 * @brief ui_vibrancy_params_init.
 * @param params Parameter params.
 * @param material Parameter material.
 * @return Return value.
 */
ui_error_t ui_vibrancy_params_init(struct ui_vibrancy_params *params,
                                   enum ui_vibrancy_material material) {
  if (!params) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  params->material = material;

  switch (material) {
  case UI_VIBRANCY_MATERIAL_ACRYLIC:
    params->blur_radius = 30.0f;
    params->noise_opacity = 0.02f;
    params->tint_color_rgba[0] = 1.0f;
    params->tint_color_rgba[1] = 1.0f;
    params->tint_color_rgba[2] = 1.0f;
    params->tint_color_rgba[3] = 0.6f;
    break;
  case UI_VIBRANCY_MATERIAL_MICA:
    params->blur_radius =
        0.0f; /* Mica relies on desktop wallpaper sampling, simplified here */
    params->noise_opacity = 0.0f;
    params->tint_color_rgba[0] = 0.95f;
    params->tint_color_rgba[1] = 0.95f;
    params->tint_color_rgba[2] = 0.95f;
    params->tint_color_rgba[3] = 0.8f;
    break;
  case UI_VIBRANCY_MATERIAL_CUPERTINO_LIGHT:
    params->blur_radius = 20.0f;
    params->noise_opacity = 0.0f;
    params->tint_color_rgba[0] = 1.0f;
    params->tint_color_rgba[1] = 1.0f;
    params->tint_color_rgba[2] = 1.0f;
    params->tint_color_rgba[3] = 0.7f;
    break;
  case UI_VIBRANCY_MATERIAL_CUPERTINO_DARK:
    params->blur_radius = 20.0f;
    params->noise_opacity = 0.0f;
    params->tint_color_rgba[0] = 0.1f;
    params->tint_color_rgba[1] = 0.1f;
    params->tint_color_rgba[2] = 0.1f;
    params->tint_color_rgba[3] = 0.7f;
    break;
  case UI_VIBRANCY_MATERIAL_NONE:
  default:
    params->blur_radius = 0.0f;
    params->noise_opacity = 0.0f;
    params->tint_color_rgba[0] = 0.0f;
    params->tint_color_rgba[1] = 0.0f;
    params->tint_color_rgba[2] = 0.0f;
    params->tint_color_rgba[3] = 0.0f;
    break;
  }

  return UI_ERROR_NONE;
}
