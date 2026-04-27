/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_materials {
  int is_initialized;
};

/**
 * @brief cmp_materials_create
 *
 * @param out_materials Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_materials_create(cmp_materials_t **out_materials) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_materials *ctx = NULL;

  if (!out_materials) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_materials_create: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_materials), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_materials_create: Out of memory\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->is_initialized = 1;
  *out_materials = (cmp_materials_t *)ctx;
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_materials_destroy
 *
 * @param materials Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_materials_destroy(cmp_materials_t *materials) {
  int rc;
  rc = CMP_SUCCESS;

  if (materials) {
    rc = CMP_FREE(materials);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_materials_destroy: CMP_FREE failed\n");
    }
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_materials_resolve_blur_effect
 *
 * @param materials Parameter description.
 * @param style Parameter description.
 * @param out_radius_px Parameter description.
 * @param out_saturation_multiplier Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_materials_resolve_blur_effect(cmp_materials_t *materials,
                                      cmp_blur_style_t style,
                                      float *out_radius_px,
                                      float *out_saturation_multiplier) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_materials *c = (struct cmp_materials *)materials;

  if (!c || !out_radius_px || !out_saturation_multiplier) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_materials_resolve_blur_effect: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

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
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_materials_resolve_macos_material
 *
 * @param materials Parameter description.
 * @param material Parameter description.
 * @param out_mapped_style Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_materials_resolve_macos_material(cmp_materials_t *materials,
                                         cmp_macos_material_t material,
                                         cmp_blur_style_t *out_mapped_style) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_materials *c = (struct cmp_materials *)materials;

  if (!c || !out_mapped_style) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_materials_resolve_macos_material: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

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
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_materials_resolve_vibrancy
 *
 * @param materials Parameter description.
 * @param style Parameter description.
 * @param out_opacity Parameter description.
 * @param out_requires_color_dodge Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_materials_resolve_vibrancy(cmp_materials_t *materials,
                                   cmp_vibrancy_style_t style,
                                   float *out_opacity,
                                   int *out_requires_color_dodge) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_materials *c = (struct cmp_materials *)materials;

  if (!c || !out_opacity || !out_requires_color_dodge) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_materials_resolve_vibrancy: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

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
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_materials_interpolate_blur_transition
 *
 * @param materials Parameter description.
 * @param from_style Parameter description.
 * @param to_style Parameter description.
 * @param progress Parameter description.
 * @param out_current_radius_px Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_materials_interpolate_blur_transition(cmp_materials_t *materials,
                                              cmp_blur_style_t from_style,
                                              cmp_blur_style_t to_style,
                                              float progress,
                                              float *out_current_radius_px) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_materials *c = (struct cmp_materials *)materials;
  float from_rad, from_sat, to_rad, to_sat;

  if (!c || !out_current_radius_px) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_materials_interpolate_blur_transition: Invalid "
              "argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Clamp progress */
  if (progress < 0.0f)
    progress = 0.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  rc = cmp_materials_resolve_blur_effect(materials, from_style, &from_rad,
                                         &from_sat);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_materials_interpolate_blur_transition: "
              "cmp_materials_resolve_blur_effect from failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = cmp_materials_resolve_blur_effect(materials, to_style, &to_rad, &to_sat);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_materials_interpolate_blur_transition: "
              "cmp_materials_resolve_blur_effect to failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Linear interpolation of the blur radius (crossfade equivalents handle the
   * visual density) */
  *out_current_radius_px = from_rad + ((to_rad - from_rad) * progress);

  if (rc != 0) {

    return rc;
  }

  return rc;
}
