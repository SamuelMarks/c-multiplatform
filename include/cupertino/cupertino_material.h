/**
 * @file cupertino_material.h
 * @brief Cupertino Material Vibrancy and Liquid Glass Meta-Material APIs.
 */

#ifndef CUPERTINO_CUPERTINO_MATERIAL_H
#define CUPERTINO_CUPERTINO_MATERIAL_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
#include "ui_types.h"
#include "ui_vibrancy.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Apple glass index of refraction (typical acrylic/glass n ~ 1.52).
 */
#define CUPERTINO_LIQUID_GLASS_INDEX_OF_REFRACTION 1.52f

/**
 * @enum cupertino_material_style
 * @brief Apple HIG background blur material hierarchy.
 */
enum cupertino_material_style {
  CUPERTINO_MATERIAL_ULTRA_THIN = 0,
  CUPERTINO_MATERIAL_THIN,
  CUPERTINO_MATERIAL_REGULAR,
  CUPERTINO_MATERIAL_THICK,
  CUPERTINO_MATERIAL_CHROME,
  CUPERTINO_MATERIAL_STYLE_COUNT
};

/**
 * @struct cupertino_material_config
 * @brief Configuration parameters for Cupertino blur material synthesis.
 */
struct cupertino_material_config {
  enum cupertino_material_style style; /**< Material thickness hierarchy. */
  int is_dark;                         /**< Non-zero for dark mode palette. */
  int reduce_transparency;             /**< A11y override: opaque fallback. */
  float appearance_tint; /**< 0.0f (More Clear) to 1.0f (More Tinted). */
};

/**
 * @struct cupertino_material_params
 * @brief Resolved physical blur and composite parameters.
 */
struct cupertino_material_params {
  float blur_radius;           /**< Gaussian blur radius in points. */
  float saturation_boost;      /**< Color saturation factor (1.0 = neutral). */
  float luminance_boost;       /**< Luminance additive modifier. */
  float tint_color_rgba[4];    /**< RGBA tint composite layer. */
  float fallback_color_rgb[3]; /**< Solid RGB color if opaque. */
  int is_opaque;               /**< Non-zero if transparency disabled. */
};

/**
 * @brief Resolves physical blur and tint parameters for a given material style.
 *
 * Automatically handles Dark Mode and Reduce Transparency accessibility
 * fallback.
 *
 * @param cfg Material configuration structure.
 * @param out_params Destination structure for resolved parameters.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_material_resolve(const struct cupertino_material_config *cfg,
                           struct cupertino_material_params *out_params);

/**
 * @brief Calculates physical Snell's law refraction UV coordinate distortion.
 *
 * @param uv_x Incident normalized X coordinate in [0, 1].
 * @param uv_y Incident normalized Y coordinate in [0, 1].
 * @param center_x Center of lensing squircle X in [0, 1].
 * @param center_y Center of lensing squircle Y in [0, 1].
 * @param corner_radius Relative corner radius of squircle boundary.
 * @param n_refraction Index of refraction (must be > 1.0).
 * @param out_distorted_u Pointer to receive distorted U coordinate.
 * @param out_distorted_v Pointer to receive distorted V coordinate.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_liquid_glass_refract(
    float uv_x, float uv_y, float center_x, float center_y, float corner_radius,
    float n_refraction, float *out_distorted_u, float *out_distorted_v);

/**
 * @brief Calculates chromatic dispersion offsets for Red and Blue channels.
 *
 * Produces wavelength-dependent separation across refractive squircle edges.
 *
 * @param uv_x Incident normalized X coordinate.
 * @param uv_y Incident normalized Y coordinate.
 * @param center_x Center of refraction X.
 * @param center_y Center of refraction Y.
 * @param dispersion_strength Wavelength separation multiplier (must be >= 0).
 * @param out_disp_r_u Pointer to receive Red channel U offset.
 * @param out_disp_r_v Pointer to receive Red channel V offset.
 * @param out_disp_b_u Pointer to receive Blue channel U offset.
 * @param out_disp_b_v Pointer to receive Blue channel V offset.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_liquid_glass_dispersion(
    float uv_x, float uv_y, float center_x, float center_y,
    float dispersion_strength, float *out_disp_r_u, float *out_disp_r_v,
    float *out_disp_b_u, float *out_disp_b_v);

/**
 * @brief Computes dynamic specular bevel reflection intensity.
 *
 * @param normal_x Surface normal X.
 * @param normal_y Surface normal Y.
 * @param normal_z Surface normal Z.
 * @param light_dir_x Incident light / hover angle X.
 * @param light_dir_y Incident light / hover angle Y.
 * @param light_dir_z Incident light / hover angle Z.
 * @param shininess Specular shininess exponent (must be > 0).
 * @param out_specular Pointer to receive specular highlight intensity in [0,
 * 1].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_liquid_glass_specular(
    float normal_x, float normal_y, float normal_z, float light_dir_x,
    float light_dir_y, float light_dir_z, float shininess, float *out_specular);

/**
 * @brief Polynomial smooth minimum for fluid droplet / metaball morphing.
 *
 * Evaluates smooth union of two signed distance fields with blending factor k.
 *
 * @param d1 First signed distance value.
 * @param d2 Second signed distance value.
 * @param k Smoothing radius / blend factor (must be > 0).
 * @param out_dist Pointer to receive smoothly blended distance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_liquid_glass_smin(float d1, float d2, float k, float *out_dist);

/**
 * @brief Samples background image luminance histogram to dynamically compute
 * adaptive contrast and tint.
 *
 * Samples a grid of ARGB pixels behind the material surface, computes weighted
 * relative luminance (ITU-R BT.709: Y = 0.2126*R + 0.7152*G + 0.0722*B),
 * evaluates average luminance and variance/contrast, and recommends an adaptive
 * boost multiplier for foreground text/borders.
 *
 * @param pixels Array of 32-bit ARGB pixels representing the background canvas
 * under the material.
 * @param pixel_count Number of pixels in the sample buffer (must be > 0).
 * @param out_avg_luminance Pointer to receive average luminance in [0.0, 1.0].
 * @param out_contrast_ratio Pointer to receive luminance variance/contrast
 * metric in [0.0, 1.0].
 * @param out_recommended_tint_boost Pointer to receive recommended foreground
 * contrast boost factor (>= 1.0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_liquid_glass_sample_histogram(const ui_color_t *pixels,
                                        size_t pixel_count,
                                        float *out_avg_luminance,
                                        float *out_contrast_ratio,
                                        float *out_recommended_tint_boost);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_MATERIAL_H */
