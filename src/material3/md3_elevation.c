/**
 * @file md3_elevation.c
 * @brief Implementation of Material 3 elevation and tonal surface tint
 * blending.
 */

/* clang-format off */
#include "material3/md3_elevation.h"
/* clang-format on */

static const float g_elevation_dp[6] = {0.0f, 1.0f, 3.0f, 6.0f, 8.0f, 12.0f};
static const float g_elevation_tint_opacity[6] = {0.00f, 0.05f, 0.08f,
                                                  0.11f, 0.12f, 0.14f};

/**
 * @brief Retrieves the elevation depth in dp for a given level (0 to 5).
 *
 * @param level Elevation level (0 through 5).
 * @param out_dp Pointer to receive elevation in dp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * level/pointer.
 */
ui_error_t md3_elevation_get_dp(int level, float *out_dp) {
  if (level < 0 || level > 5 || !out_dp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_dp = g_elevation_dp[level];
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the surface tint overlay opacity for a given level (0 to 5).
 *
 * @param level Elevation level (0 through 5).
 * @param out_opacity Pointer to receive the opacity fraction (0.0 to 1.0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * level/pointer.
 */
ui_error_t md3_elevation_get_tint_opacity(int level, float *out_opacity) {
  if (level < 0 || level > 5 || !out_opacity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_opacity = g_elevation_tint_opacity[level];
  return UI_ERROR_NONE;
}

/**
 * @brief Computes the composite elevated surface color using surface tint
 * blending.
 *
 * @param surface_base Base surface color.
 * @param primary_tint The primary color used for surface tinting.
 * @param level Elevation level (0 through 5).
 * @param out_color Pointer to store the blended result color.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_elevation_get_surface_color(ui_color_t surface_base,
                                           ui_color_t primary_tint, int level,
                                           ui_color_t *out_color) {
  float opacity;
  float inv_opacity;
  ui_uint8 a, r, g, b;
  ui_error_t rc;

  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = md3_elevation_get_tint_opacity(level, &opacity);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (opacity <= 0.0001f) {
    *out_color = surface_base;
    return UI_ERROR_NONE;
  }

  inv_opacity = 1.0f - opacity;
  a = UI_COLOR_ALPHA(surface_base);
  r = (ui_uint8)((float)UI_COLOR_RED(surface_base) * inv_opacity +
                 (float)UI_COLOR_RED(primary_tint) * opacity + 0.5f);
  g = (ui_uint8)((float)UI_COLOR_GREEN(surface_base) * inv_opacity +
                 (float)UI_COLOR_GREEN(primary_tint) * opacity + 0.5f);
  b = (ui_uint8)((float)UI_COLOR_BLUE(surface_base) * inv_opacity +
                 (float)UI_COLOR_BLUE(primary_tint) * opacity + 0.5f);

  *out_color = UI_COLOR_ARGB(a, r, g, b);
  return UI_ERROR_NONE;
}
