/**
 * @file cupertino_material.c
 * @brief Cupertino Material Vibrancy and Liquid Glass Meta-Material
 * implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_material.h"
#include <math.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Resolves physical blur and tint parameters for a given material style.
 */
ui_error_t
cupertino_material_resolve(const struct cupertino_material_config *cfg,
                           struct cupertino_material_params *out_params) {
  float tint_mod;
  float base_blur;
  float sat_boost;
  float lum_boost;
  float alpha;

  if (!cfg || !out_params || (int)cfg->style < 0 ||
      (int)cfg->style >= (int)CUPERTINO_MATERIAL_STYLE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(out_params, 0, sizeof(*out_params));

  if (cfg->reduce_transparency) {
    /* Accessibility override: solid opaque background */
    out_params->is_opaque = 1;
    out_params->blur_radius = 0.0f;
    out_params->saturation_boost = 1.0f;
    out_params->luminance_boost = 0.0f;
    out_params->tint_color_rgba[0] = 0.0f;
    out_params->tint_color_rgba[1] = 0.0f;
    out_params->tint_color_rgba[2] = 0.0f;
    out_params->tint_color_rgba[3] = 1.0f;

    if (cfg->is_dark) {
      out_params->fallback_color_rgb[0] = 0.11f; /* #1C1C1E */
      out_params->fallback_color_rgb[1] = 0.11f;
      out_params->fallback_color_rgb[2] = 0.12f;
    } else {
      out_params->fallback_color_rgb[0] = 1.0f; /* #FFFFFF */
      out_params->fallback_color_rgb[1] = 1.0f;
      out_params->fallback_color_rgb[2] = 1.0f;
    }
    return UI_ERROR_NONE;
  }

  out_params->is_opaque = 0;

  tint_mod = cfg->appearance_tint;
  if (tint_mod < 0.0f) {
    tint_mod = 0.0f;
  } else if (tint_mod > 1.0f) {
    tint_mod = 1.0f;
  }

  if (cfg->style == CUPERTINO_MATERIAL_ULTRA_THIN) {
    base_blur = 10.0f;
    sat_boost = 1.6f;
    lum_boost = 0.05f;
    alpha = 0.25f + 0.15f * tint_mod;
  } else if (cfg->style == CUPERTINO_MATERIAL_THIN) {
    base_blur = 20.0f;
    sat_boost = 1.8f;
    lum_boost = 0.08f;
    alpha = 0.40f + 0.15f * tint_mod;
  } else if (cfg->style == CUPERTINO_MATERIAL_REGULAR) {
    base_blur = 30.0f;
    sat_boost = 1.9f;
    lum_boost = 0.10f;
    alpha = 0.55f + 0.15f * tint_mod;
  } else if (cfg->style == CUPERTINO_MATERIAL_THICK) {
    base_blur = 45.0f;
    sat_boost = 2.0f;
    lum_boost = 0.12f;
    alpha = 0.70f + 0.15f * tint_mod;
  } else {
    base_blur = 55.0f;
    sat_boost = 2.1f;
    lum_boost = 0.15f;
    alpha = 0.82f + 0.10f * tint_mod;
  }

  out_params->blur_radius = base_blur;
  out_params->saturation_boost = sat_boost;
  out_params->luminance_boost = lum_boost;

  if (cfg->is_dark) {
    out_params->tint_color_rgba[0] = 0.15f;
    out_params->tint_color_rgba[1] = 0.15f;
    out_params->tint_color_rgba[2] = 0.16f;
    out_params->tint_color_rgba[3] = alpha;
  } else {
    out_params->tint_color_rgba[0] = 0.95f;
    out_params->tint_color_rgba[1] = 0.95f;
    out_params->tint_color_rgba[2] = 0.97f;
    out_params->tint_color_rgba[3] = alpha;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Calculates physical Snell's law refraction UV coordinate distortion.
 */
ui_error_t cupertino_liquid_glass_refract(
    float uv_x, float uv_y, float center_x, float center_y, float corner_radius,
    float n_refraction, float *out_distorted_u, float *out_distorted_v) {
  float dx;
  float dy;
  float dist;
  float bend_factor;
  float r_safe;

  if (!out_distorted_u || !out_distorted_v || n_refraction <= 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dx = uv_x - center_x;
  dy = uv_y - center_y;
  dist = (float)sqrt(dx * dx + dy * dy);

  r_safe = (corner_radius > 0.001f) ? corner_radius : 0.5f;

  /* Snell's law angle distortion gradient towards boundary */
  bend_factor = (1.0f - (1.0f / n_refraction)) / (1.0f + (dist / r_safe));

  *out_distorted_u = uv_x - (dx * bend_factor);
  *out_distorted_v = uv_y - (dy * bend_factor);

  return UI_ERROR_NONE;
}

/**
 * @brief Calculates chromatic dispersion offsets for Red and Blue channels.
 */
ui_error_t
cupertino_liquid_glass_dispersion(float uv_x, float uv_y, float center_x,
                                  float center_y, float dispersion_strength,
                                  float *out_disp_r_u, float *out_disp_r_v,
                                  float *out_disp_b_u, float *out_disp_b_v) {
  float dx;
  float dy;
  float dist;

  if (!out_disp_r_u || !out_disp_r_v || !out_disp_b_u || !out_disp_b_v ||
      dispersion_strength < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dx = uv_x - center_x;
  dy = uv_y - center_y;
  dist = (float)sqrt(dx * dx + dy * dy);

  if (dist > 1e-5f) {
    dx /= dist;
    dy /= dist;
  } else {
    dx = 0.0f;
    dy = 0.0f;
  }

  /* Red channel bends less: positive radial offset */
  *out_disp_r_u = dx * dispersion_strength;
  *out_disp_r_v = dy * dispersion_strength;

  /* Blue channel bends more: negative radial offset */
  *out_disp_b_u = -dx * dispersion_strength;
  *out_disp_b_v = -dy * dispersion_strength;

  return UI_ERROR_NONE;
}

/**
 * @brief Computes dynamic specular bevel reflection intensity.
 */
ui_error_t cupertino_liquid_glass_specular(float normal_x, float normal_y,
                                           float normal_z, float light_dir_x,
                                           float light_dir_y, float light_dir_z,
                                           float shininess,
                                           float *out_specular) {
  float n_len;
  float l_len;
  float dot_nl;
  float rz;

  if (!out_specular || shininess <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  n_len = (float)sqrt(normal_x * normal_x + normal_y * normal_y +
                      normal_z * normal_z);
  l_len = (float)sqrt(light_dir_x * light_dir_x + light_dir_y * light_dir_y +
                      light_dir_z * light_dir_z);

  if (n_len <= 1e-5f || l_len <= 1e-5f) {
    *out_specular = 0.0f;
    return UI_ERROR_NONE;
  }

  normal_x /= n_len;
  normal_y /= n_len;
  normal_z /= n_len;

  light_dir_x /= l_len;
  light_dir_y /= l_len;
  light_dir_z /= l_len;

  dot_nl =
      normal_x * light_dir_x + normal_y * light_dir_y + normal_z * light_dir_z;
  if (dot_nl <= 0.0f) {
    *out_specular = 0.0f;
    return UI_ERROR_NONE;
  }

  /* Reflected ray along Z for view vector V = (0, 0, 1): R_z = 2*(N.L)*N_z -
   * L_z */
  rz = 2.0f * dot_nl * normal_z - light_dir_z;

  if (rz <= 0.0f) {
    *out_specular = 0.0f;
  } else {
    *out_specular = (float)pow(rz, shininess);
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Polynomial smooth minimum for fluid droplet / metaball morphing.
 */
ui_error_t cupertino_liquid_glass_smin(float d1, float d2, float k,
                                       float *out_dist) {
  float h;
  float min_val;

  if (!out_dist || k <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  h = k - (float)fabs(d1 - d2);
  if (h < 0.0f) {
    h = 0.0f;
  }
  h /= k;

  min_val = (d1 < d2) ? d1 : d2;
  *out_dist = min_val - h * h * k * 0.25f;

  return UI_ERROR_NONE;
}

/**
 * @brief Samples background image luminance histogram to dynamically compute
 * adaptive contrast and tint.
 */
ui_error_t cupertino_liquid_glass_sample_histogram(
    const ui_color_t *pixels, size_t pixel_count, float *out_avg_luminance,
    float *out_contrast_ratio, float *out_recommended_tint_boost) {
  size_t i;
  double sum_lum;
  double sum_sq_diff;
  double mean_lum;
  double variance;
  double std_dev;
  float boost;

  if (!pixels || pixel_count == 0 || !out_avg_luminance ||
      !out_contrast_ratio || !out_recommended_tint_boost) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sum_lum = 0.0;
  for (i = 0; i < pixel_count; ++i) {
    ui_color_t c;
    double r, g, b, y;
    c = pixels[i];
    r = (double)UI_COLOR_RED(c) / 255.0;
    g = (double)UI_COLOR_GREEN(c) / 255.0;
    b = (double)UI_COLOR_BLUE(c) / 255.0;
    /* ITU-R BT.709 perceived luminance */
    y = 0.2126 * r + 0.7152 * g + 0.0722 * b;
    sum_lum += y;
  }

  mean_lum = sum_lum / (double)pixel_count;

  sum_sq_diff = 0.0;
  for (i = 0; i < pixel_count; ++i) {
    ui_color_t c;
    double r, g, b, y, diff;
    c = pixels[i];
    r = (double)UI_COLOR_RED(c) / 255.0;
    g = (double)UI_COLOR_GREEN(c) / 255.0;
    b = (double)UI_COLOR_BLUE(c) / 255.0;
    y = 0.2126 * r + 0.7152 * g + 0.0722 * b;
    diff = y - mean_lum;
    sum_sq_diff += diff * diff;
  }

  variance = sum_sq_diff / (double)pixel_count;
  std_dev = sqrt(variance);

  /* High variance / complex wallpaper or mid-gray ambiguity requires higher
   * contrast boost */
  /* Boost baseline is 1.0; increases up to 1.5 based on visual complexity
   * (variance) and mid-tone ambiguity */
  boost =
      1.0f + (float)(std_dev * 0.8 + (1.0 - fabs(mean_lum - 0.5) * 2.0) * 0.2);

  *out_avg_luminance = (float)mean_lum;
  *out_contrast_ratio = (float)std_dev;
  *out_recommended_tint_boost = boost;

  return UI_ERROR_NONE;
}
