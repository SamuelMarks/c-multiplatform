/**
 * @file ui_elevation.c
 * @brief ui_elevation.c implementation.
 */
/* clang-format off */
#include "ui_elevation.h"
/* clang-format on */

/* M3 surface tint opacities for elevation levels 1-5 */
/** @brief Opacity values for different elevation levels */
static const float TINT_OPACITIES[] = {0.0f, 0.05f, 0.08f, 0.11f, 0.12f, 0.14f};

/**
 * @brief ui_elevation_get_tinted_surface.
 * @param surface_color Parameter surface_color.
 * @param tint_color Parameter tint_color.
 * @param level Parameter level.
 * @param out_color Parameter out_color.
 * @return Return value.
 */
ui_error_t ui_elevation_get_tinted_surface(ui_color_t surface_color,
                                           ui_color_t tint_color,
                                           enum ui_elevation_level level,
                                           ui_color_t *out_color) {
  float opacity;
  ui_uint8 r_surf, g_surf, b_surf;
  ui_uint8 r_tint, g_tint, b_tint;
  ui_uint8 r_out, g_out, b_out;

  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((int)level < 0 || (int)level > 5) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (level == UI_ELEVATION_LEVEL_0) {
    *out_color = surface_color;
    return UI_ERROR_NONE;
  }

  opacity = TINT_OPACITIES[level];

  r_surf = UI_COLOR_RED(surface_color);
  g_surf = UI_COLOR_GREEN(surface_color);
  b_surf = UI_COLOR_BLUE(surface_color);

  r_tint = UI_COLOR_RED(tint_color);
  g_tint = UI_COLOR_GREEN(tint_color);
  b_tint = UI_COLOR_BLUE(tint_color);

  r_out = (ui_uint8)((r_tint * opacity) + (r_surf * (1.0f - opacity)));
  g_out = (ui_uint8)((g_tint * opacity) + (g_surf * (1.0f - opacity)));
  b_out = (ui_uint8)((b_tint * opacity) + (b_surf * (1.0f - opacity)));

  *out_color =
      UI_COLOR_ARGB(UI_COLOR_ALPHA(surface_color), r_out, g_out, b_out);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_elevation_get_shadows.
 * @param level Parameter level.
 * @param shadow_color Parameter shadow_color.
 * @param out_shadow1 Parameter out_shadow1.
 * @param out_shadow2 Parameter out_shadow2.
 * @return Return value.
 */
ui_error_t ui_elevation_get_shadows(enum ui_elevation_level level,
                                    ui_color_t shadow_color,
                                    struct ui_drop_shadow *out_shadow1,
                                    struct ui_drop_shadow *out_shadow2) {
  ui_uint8 r_sh, g_sh, b_sh;

  if (!out_shadow1 || !out_shadow2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((int)level < 0 || (int)level > 5) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r_sh = UI_COLOR_RED(shadow_color);
  g_sh = UI_COLOR_GREEN(shadow_color);
  b_sh = UI_COLOR_BLUE(shadow_color);

  /* Very rudimentary fallback shadow config based roughly on levels */
  if (level == UI_ELEVATION_LEVEL_0) {
    out_shadow1->x_offset = 0.0f;
    out_shadow1->y_offset = 0.0f;
    out_shadow1->blur_radius = 0.0f;
    out_shadow1->spread_radius = 0.0f;
    out_shadow1->color = UI_COLOR_ARGB(0, 0, 0, 0);

    *out_shadow2 = *out_shadow1;
    return UI_ERROR_NONE;
  }

  out_shadow1->x_offset = 0.0f;
  out_shadow1->y_offset = (float)level * 1.5f;
  out_shadow1->blur_radius = (float)level * 3.0f;
  out_shadow1->spread_radius = 0.0f;
  out_shadow1->color = UI_COLOR_ARGB((ui_uint8)(255 * 0.15f), r_sh, g_sh, b_sh);

  out_shadow2->x_offset = 0.0f;
  out_shadow2->y_offset = (float)level * 0.5f;
  out_shadow2->blur_radius = (float)level * 1.0f;
  out_shadow2->spread_radius = 0.0f;
  out_shadow2->color = UI_COLOR_ARGB((ui_uint8)(255 * 0.30f), r_sh, g_sh, b_sh);

  return UI_ERROR_NONE;
}
