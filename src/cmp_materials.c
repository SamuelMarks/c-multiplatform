/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_materials {
  int is_initialized;
};

int cmp_materials_create(cmp_materials_t **out_materials) {
  struct cmp_materials *ctx;
  if (!out_materials)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_materials), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  ctx->is_initialized = 1;
  *out_materials = (cmp_materials_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_materials_destroy(cmp_materials_t *materials) {
  if (materials)
    CMP_FREE(materials);
  return CMP_SUCCESS;
}

int cmp_materials_resolve_blur_effect(cmp_materials_t *materials,
                                      cmp_blur_style_t style,
                                      float *out_radius_px,
                                      float *out_saturation_multiplier) {
  struct cmp_materials *c = (struct cmp_materials *)materials;
  if (!c || !out_radius_px || !out_saturation_multiplier)
    return CMP_ERROR_INVALID_ARG;

  /* Simulating Apple HIG blur radii and saturation boosts */
  switch (style) {
  case CMP_BLUR_STYLE_ULTRA_THIN:
    *out_radius_px = 5.0f;
    *out_saturation_multiplier = 1.0f;
    break;
  case CMP_BLUR_STYLE_THIN:
    *out_radius_px = 10.0f;
    *out_saturation_multiplier = 1.2f;
    break;
  case CMP_BLUR_STYLE_REGULAR:
    *out_radius_px = 20.0f;
    *out_saturation_multiplier = 1.5f;
    break;
  case CMP_BLUR_STYLE_THICK:
    *out_radius_px = 30.0f;
    *out_saturation_multiplier = 1.8f;
    break;
  case CMP_BLUR_STYLE_PROMINENT:
    *out_radius_px = 50.0f;
    *out_saturation_multiplier = 2.2f;
    break;
  default:
    *out_radius_px = 20.0f;
    *out_saturation_multiplier = 1.5f;
    break;
  }
  return CMP_SUCCESS;
}

int cmp_materials_resolve_macos_material(cmp_materials_t *materials,
                                         cmp_macos_material_t material,
                                         cmp_blur_style_t *out_mapped_style) {
  struct cmp_materials *c = (struct cmp_materials *)materials;
  if (!c || !out_mapped_style)
    return CMP_ERROR_INVALID_ARG;

  switch (material) {
  case CMP_MACOS_MATERIAL_WINDOW_BACKGROUND:
    *out_mapped_style = CMP_BLUR_STYLE_REGULAR;
    break;
  case CMP_MACOS_MATERIAL_BEHIND_WINDOW:
    *out_mapped_style =
        CMP_BLUR_STYLE_THICK; /* Often used for deeper sidebars */
    break;
  case CMP_MACOS_MATERIAL_UNDER_WINDOW_BACKGROUND:
    *out_mapped_style = CMP_BLUR_STYLE_THIN;
    break;
  default:
    *out_mapped_style = CMP_BLUR_STYLE_REGULAR;
    break;
  }
  return CMP_SUCCESS;
}

int cmp_materials_resolve_vibrancy(cmp_materials_t *materials,
                                   cmp_vibrancy_style_t style,
                                   float *out_opacity,
                                   int *out_requires_color_dodge) {
  struct cmp_materials *c = (struct cmp_materials *)materials;
  if (!c || !out_opacity || !out_requires_color_dodge)
    return CMP_ERROR_INVALID_ARG;

  /* UIVibrancyEffect uses color dodge/burn plus opacity layering */
  switch (style) {
  case CMP_VIBRANCY_STYLE_NONE:
    *out_opacity = 1.0f;
    *out_requires_color_dodge = 0;
    break;
  case CMP_VIBRANCY_STYLE_LABEL:
    *out_opacity = 1.0f;
    *out_requires_color_dodge = 1;
    break;
  case CMP_VIBRANCY_STYLE_SECONDARY_LABEL:
    *out_opacity = 0.6f;
    *out_requires_color_dodge = 1;
    break;
  case CMP_VIBRANCY_STYLE_TERTIARY_LABEL:
    *out_opacity = 0.3f;
    *out_requires_color_dodge = 1;
    break;
  case CMP_VIBRANCY_STYLE_FILL:
    *out_opacity = 0.2f;
    *out_requires_color_dodge = 0; /* Often just alpha layering for fills */
    break;
  case CMP_VIBRANCY_STYLE_SECONDARY_FILL:
    *out_opacity = 0.1f;
    *out_requires_color_dodge = 0;
    break;
  default:
    *out_opacity = 1.0f;
    *out_requires_color_dodge = 0;
    break;
  }
  return CMP_SUCCESS;
}

int cmp_materials_interpolate_blur_transition(cmp_materials_t *materials,
                                              cmp_blur_style_t from_style,
                                              cmp_blur_style_t to_style,
                                              float progress,
                                              float *out_current_radius_px) {
  struct cmp_materials *c = (struct cmp_materials *)materials;
  float from_rad, from_sat, to_rad, to_sat;
  int res;

  if (!c || !out_current_radius_px)
    return CMP_ERROR_INVALID_ARG;

  /* Clamp progress */
  if (progress < 0.0f)
    progress = 0.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  res = cmp_materials_resolve_blur_effect(materials, from_style, &from_rad,
                                          &from_sat);
  if (res != CMP_SUCCESS)
    return res;

  res =
      cmp_materials_resolve_blur_effect(materials, to_style, &to_rad, &to_sat);
  if (res != CMP_SUCCESS)
    return res;

  /* Linear interpolation of the blur radius (crossfade equivalents handle the
   * visual density) */
  *out_current_radius_px = from_rad + ((to_rad - from_rad) * progress);

  return CMP_SUCCESS;
}
