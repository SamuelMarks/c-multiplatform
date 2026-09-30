/**
 * @file md3_color.c
 * @brief Material 3 dynamic color scheme, tonal palette generation, and
 * harmonization.
 */

/* clang-format off */
#include "material3/md3_color.h"
#include <math.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Normalizes an angle in degrees to the [0, 360) range.
 *
 * @param deg Input degrees.
 * @return Normalized angle in [0, 360).
 */
#ifdef UI_TEST_MOCK_ALLOC
int g_md3_color_mock_fail = 0;

static ui_error_t mock_color_argb_to_hct(ui_color_t argb,
                                         struct ui_color_hct *out_hct) {
  if (g_md3_color_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_md3_color_mock_fail == 2) {
    g_md3_color_mock_fail = 1;
    return ui_color_argb_to_hct(argb, out_hct);
  }
  return ui_color_argb_to_hct(argb, out_hct);
}

static ui_error_t
mock_tonal_palette_get_tone(const struct ui_tonal_palette *palette, float tone,
                            ui_color_t *out_color) {
  if (g_md3_color_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_tonal_palette_get_tone(palette, tone, out_color);
}

#undef ui_color_argb_to_hct
/** @cond */
#define ui_color_argb_to_hct mock_color_argb_to_hct
/** @endcond */
#undef ui_tonal_palette_get_tone
/** @cond */
#define ui_tonal_palette_get_tone mock_tonal_palette_get_tone
/** @endcond */
#endif

static float sanitize_degrees(float deg) {
  deg = (float)fmod((double)deg, 360.0);
  if (deg < 0.0f) {
    deg += 360.0f;
  }
  return deg;
}

/**
 * @brief Shifts the hue of a design color toward a source color
 * (Harmonization).
 *
 * @param design_color The color to harmonize.
 * @param source_color The target anchor color (e.g. primary seed).
 * @param out_color Pointer to store the harmonized color.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_color_harmonize(ui_color_t design_color, ui_color_t source_color,
                               ui_color_t *out_color) {
  struct ui_color_hct from_hct;
  struct ui_color_hct to_hct;
  struct ui_color_hct res_hct;
  float diff;
  float rotation;
  ui_error_t rc;

  if (!out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_color_argb_to_hct(design_color, &from_hct);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_color_argb_to_hct(source_color, &to_hct);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  diff = sanitize_degrees(to_hct.hue - from_hct.hue);
  if (diff > 180.0f) {
    diff -= 360.0f;
  }

  /* Rotate up to 15 degrees toward source color */
  rotation = diff * 0.5f;
  if (rotation > 15.0f) {
    rotation = 15.0f;
  } else if (rotation < -15.0f) {
    rotation = -15.0f;
  }

  res_hct.hue = sanitize_degrees(from_hct.hue + rotation);
  res_hct.chroma = from_hct.chroma;
  res_hct.tone = from_hct.tone;

  return ui_color_hct_to_argb(&res_hct, out_color);
}

/**
 * @brief Helper to generate a color from a palette hue/chroma and tone.
 *
 * @param hue Palette hue.
 * @param chroma Palette chroma.
 * @param tone Tone stop (0.0 to 100.0).
 * @param out_color Pointer to receive result.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t palette_tone(float hue, float chroma, float tone,
                               ui_color_t *out_color) {
  struct ui_tonal_palette pal;

  pal.hue = sanitize_degrees(hue);
  pal.chroma = chroma;

  return ui_tonal_palette_get_tone(&pal, tone, out_color);
}

/**
 * @brief Generates a full Material 3 color scheme with a specific contrast
 * level.
 *
 * @param seed_color The source key color to derive palettes from.
 * @param is_dark Non-zero to generate dark theme, zero for light theme.
 * @param mode Palette derivation mode.
 * @param contrast Contrast level (normal, medium 4.5:1, or high 7:1).
 * @param out_scheme Pointer to the color scheme structure to populate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
struct md3_palette_role_def {
  float hue;
  float chroma;
  float tone;
  size_t offset;
};

ui_error_t md3_color_scheme_create_contrast(
    ui_color_t seed_color, int is_dark, enum md3_palette_mode mode,
    enum md3_contrast_level contrast, struct md3_color_scheme *out_scheme) {
  struct ui_color_hct seed_hct;
  float p_hue, p_chroma;
  float s_hue, s_chroma;
  float t_hue, t_chroma;
  float n_hue, n_chroma;
  float nv_hue, nv_chroma;
  float e_hue, e_chroma;
  float primary_tone, on_primary_tone, container_tone, on_container_tone;
  float outline_tone, outline_variant_tone;
  ui_error_t rc;

  if (!out_scheme || (int)contrast < (int)MD3_CONTRAST_NORMAL ||
      (int)contrast > (int)MD3_CONTRAST_HIGH) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_color_argb_to_hct(seed_color, &seed_hct);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  e_hue = 25.0f;
  e_chroma = 84.0f;

  switch (mode) {
  case MD3_PALETTE_MODE_EXPRESSIVE:
    p_hue = sanitize_degrees(seed_hct.hue + 120.0f);
    p_chroma = 40.0f;
    s_hue = sanitize_degrees(seed_hct.hue + 120.0f);
    s_chroma = 24.0f;
    t_hue = sanitize_degrees(seed_hct.hue + 240.0f);
    t_chroma = 32.0f;
    n_hue = sanitize_degrees(seed_hct.hue + 15.0f);
    n_chroma = 8.0f;
    nv_hue = sanitize_degrees(seed_hct.hue + 15.0f);
    nv_chroma = 12.0f;
    break;

  case MD3_PALETTE_MODE_VIBRANT:
    p_hue = seed_hct.hue;
    p_chroma = (seed_hct.chroma > 74.0f) ? seed_hct.chroma : 74.0f;
    s_hue = seed_hct.hue;
    s_chroma = 24.0f;
    t_hue = sanitize_degrees(seed_hct.hue + 60.0f);
    t_chroma = 32.0f;
    n_hue = seed_hct.hue;
    n_chroma = 6.0f;
    nv_hue = seed_hct.hue;
    nv_chroma = 12.0f;
    break;

  case MD3_PALETTE_MODE_CONTENT_BASED:
    p_hue = seed_hct.hue;
    p_chroma = seed_hct.chroma;
    s_hue = seed_hct.hue;
    s_chroma = seed_hct.chroma * 0.33f;
    t_hue = sanitize_degrees(seed_hct.hue + 60.0f);
    t_chroma = seed_hct.chroma * 0.66f;
    n_hue = seed_hct.hue;
    n_chroma = 4.0f;
    nv_hue = seed_hct.hue;
    nv_chroma = 8.0f;
    break;

  case MD3_PALETTE_MODE_MONOCHROME:
    p_hue = seed_hct.hue;
    p_chroma = 0.0f;
    s_hue = seed_hct.hue;
    s_chroma = 0.0f;
    t_hue = seed_hct.hue;
    t_chroma = 0.0f;
    n_hue = seed_hct.hue;
    n_chroma = 0.0f;
    nv_hue = seed_hct.hue;
    nv_chroma = 0.0f;
    break;

  case MD3_PALETTE_MODE_FRUIT_SALAD:
    p_hue = sanitize_degrees(seed_hct.hue - 50.0f);
    p_chroma = 48.0f;
    s_hue = sanitize_degrees(seed_hct.hue - 50.0f);
    s_chroma = 36.0f;
    t_hue = seed_hct.hue;
    t_chroma = 36.0f;
    n_hue = seed_hct.hue;
    n_chroma = 10.0f;
    nv_hue = seed_hct.hue;
    nv_chroma = 16.0f;
    break;

  case MD3_PALETTE_MODE_RAINBOW:
    p_hue = seed_hct.hue;
    p_chroma = 48.0f;
    s_hue = sanitize_degrees(seed_hct.hue + 30.0f);
    s_chroma = 16.0f;
    t_hue = sanitize_degrees(seed_hct.hue + 60.0f);
    t_chroma = 24.0f;
    n_hue = seed_hct.hue;
    n_chroma = 0.0f;
    nv_hue = seed_hct.hue;
    nv_chroma = 0.0f;
    break;

  case MD3_PALETTE_MODE_TONAL_SPOT:
  default:
    p_hue = seed_hct.hue;
    p_chroma = (seed_hct.chroma > 48.0f) ? seed_hct.chroma : 48.0f;
    s_hue = seed_hct.hue;
    s_chroma = 16.0f;
    t_hue = sanitize_degrees(seed_hct.hue + 60.0f);
    t_chroma = 24.0f;
    n_hue = seed_hct.hue;
    n_chroma = 4.0f;
    nv_hue = seed_hct.hue;
    nv_chroma = 8.0f;
    break;
  }

  {
    struct md3_palette_role_def roles[32];
    size_t i;
    size_t role_count;

    if (!is_dark) {
      /* Light Theme */
      primary_tone = 40.0f;
      on_primary_tone = 100.0f;
      container_tone = 90.0f;
      on_container_tone = 10.0f;
      outline_tone = 50.0f;
      outline_variant_tone = 80.0f;

      if (contrast == MD3_CONTRAST_MEDIUM) {
        primary_tone = 30.0f;
        container_tone = 80.0f;
        on_container_tone = 0.0f;
        outline_tone = 40.0f;
        outline_variant_tone = 70.0f;
      } else if (contrast == MD3_CONTRAST_HIGH) {
        primary_tone = 20.0f;
        container_tone = 70.0f;
        on_container_tone = 0.0f;
        outline_tone = 30.0f;
        outline_variant_tone = 60.0f;
      }

      roles[0].hue = p_hue;
      roles[0].chroma = p_chroma;
      roles[0].tone = primary_tone;
      roles[0].offset = offsetof(struct md3_color_scheme, primary);
      roles[1].hue = p_hue;
      roles[1].chroma = p_chroma;
      roles[1].tone = on_primary_tone;
      roles[1].offset = offsetof(struct md3_color_scheme, on_primary);
      roles[2].hue = p_hue;
      roles[2].chroma = p_chroma;
      roles[2].tone = container_tone;
      roles[2].offset = offsetof(struct md3_color_scheme, primary_container);
      roles[3].hue = p_hue;
      roles[3].chroma = p_chroma;
      roles[3].tone = on_container_tone;
      roles[3].offset = offsetof(struct md3_color_scheme, on_primary_container);
      roles[4].hue = p_hue;
      roles[4].chroma = p_chroma;
      roles[4].tone = 80.0f;
      roles[4].offset = offsetof(struct md3_color_scheme, inverse_primary);

      roles[5].hue = s_hue;
      roles[5].chroma = s_chroma;
      roles[5].tone = primary_tone;
      roles[5].offset = offsetof(struct md3_color_scheme, secondary);
      roles[6].hue = s_hue;
      roles[6].chroma = s_chroma;
      roles[6].tone = on_primary_tone;
      roles[6].offset = offsetof(struct md3_color_scheme, on_secondary);
      roles[7].hue = s_hue;
      roles[7].chroma = s_chroma;
      roles[7].tone = container_tone;
      roles[7].offset = offsetof(struct md3_color_scheme, secondary_container);
      roles[8].hue = s_hue;
      roles[8].chroma = s_chroma;
      roles[8].tone = on_container_tone;
      roles[8].offset =
          offsetof(struct md3_color_scheme, on_secondary_container);

      roles[9].hue = t_hue;
      roles[9].chroma = t_chroma;
      roles[9].tone = primary_tone;
      roles[9].offset = offsetof(struct md3_color_scheme, tertiary);
      roles[10].hue = t_hue;
      roles[10].chroma = t_chroma;
      roles[10].tone = on_primary_tone;
      roles[10].offset = offsetof(struct md3_color_scheme, on_tertiary);
      roles[11].hue = t_hue;
      roles[11].chroma = t_chroma;
      roles[11].tone = container_tone;
      roles[11].offset = offsetof(struct md3_color_scheme, tertiary_container);
      roles[12].hue = t_hue;
      roles[12].chroma = t_chroma;
      roles[12].tone = on_container_tone;
      roles[12].offset =
          offsetof(struct md3_color_scheme, on_tertiary_container);

      roles[13].hue = e_hue;
      roles[13].chroma = e_chroma;
      roles[13].tone = primary_tone;
      roles[13].offset = offsetof(struct md3_color_scheme, error);
      roles[14].hue = e_hue;
      roles[14].chroma = e_chroma;
      roles[14].tone = on_primary_tone;
      roles[14].offset = offsetof(struct md3_color_scheme, on_error);
      roles[15].hue = e_hue;
      roles[15].chroma = e_chroma;
      roles[15].tone = container_tone;
      roles[15].offset = offsetof(struct md3_color_scheme, error_container);
      roles[16].hue = e_hue;
      roles[16].chroma = e_chroma;
      roles[16].tone = on_container_tone;
      roles[16].offset = offsetof(struct md3_color_scheme, on_error_container);

      roles[17].hue = n_hue;
      roles[17].chroma = n_chroma;
      roles[17].tone = 98.0f;
      roles[17].offset = offsetof(struct md3_color_scheme, surface);
      roles[18].hue = n_hue;
      roles[18].chroma = n_chroma;
      roles[18].tone = 10.0f;
      roles[18].offset = offsetof(struct md3_color_scheme, on_surface);
      roles[19].hue = nv_hue;
      roles[19].chroma = nv_chroma;
      roles[19].tone = 90.0f;
      roles[19].offset = offsetof(struct md3_color_scheme, surface_variant);
      roles[20].hue = nv_hue;
      roles[20].chroma = nv_chroma;
      roles[20].tone = 30.0f;
      roles[20].offset = offsetof(struct md3_color_scheme, on_surface_variant);
      roles[21].hue = n_hue;
      roles[21].chroma = n_chroma;
      roles[21].tone = 20.0f;
      roles[21].offset = offsetof(struct md3_color_scheme, inverse_surface);
      roles[22].hue = n_hue;
      roles[22].chroma = n_chroma;
      roles[22].tone = 95.0f;
      roles[22].offset = offsetof(struct md3_color_scheme, inverse_on_surface);

      roles[23].hue = n_hue;
      roles[23].chroma = n_chroma;
      roles[23].tone = 100.0f;
      roles[23].offset =
          offsetof(struct md3_color_scheme, surface_container_lowest);
      roles[24].hue = n_hue;
      roles[24].chroma = n_chroma;
      roles[24].tone = 96.0f;
      roles[24].offset =
          offsetof(struct md3_color_scheme, surface_container_low);
      roles[25].hue = n_hue;
      roles[25].chroma = n_chroma;
      roles[25].tone = 94.0f;
      roles[25].offset = offsetof(struct md3_color_scheme, surface_container);
      roles[26].hue = n_hue;
      roles[26].chroma = n_chroma;
      roles[26].tone = 92.0f;
      roles[26].offset =
          offsetof(struct md3_color_scheme, surface_container_high);
      roles[27].hue = n_hue;
      roles[27].chroma = n_chroma;
      roles[27].tone = 90.0f;
      roles[27].offset =
          offsetof(struct md3_color_scheme, surface_container_highest);
      roles[28].hue = n_hue;
      roles[28].chroma = n_chroma;
      roles[28].tone = 87.0f;
      roles[28].offset = offsetof(struct md3_color_scheme, surface_dim);
      roles[29].hue = n_hue;
      roles[29].chroma = n_chroma;
      roles[29].tone = 98.0f;
      roles[29].offset = offsetof(struct md3_color_scheme, surface_bright);

      roles[30].hue = nv_hue;
      roles[30].chroma = nv_chroma;
      roles[30].tone = outline_tone;
      roles[30].offset = offsetof(struct md3_color_scheme, outline);
      roles[31].hue = nv_hue;
      roles[31].chroma = nv_chroma;
      roles[31].tone = outline_variant_tone;
      roles[31].offset = offsetof(struct md3_color_scheme, outline_variant);
    } else {
      /* Dark Theme */
      primary_tone = 80.0f;
      on_primary_tone = 20.0f;
      container_tone = 30.0f;
      on_container_tone = 90.0f;
      outline_tone = 60.0f;
      outline_variant_tone = 30.0f;

      if (contrast == MD3_CONTRAST_MEDIUM) {
        primary_tone = 90.0f;
        on_primary_tone = 10.0f;
        container_tone = 40.0f;
        on_container_tone = 95.0f;
        outline_tone = 70.0f;
        outline_variant_tone = 40.0f;
      } else if (contrast == MD3_CONTRAST_HIGH) {
        primary_tone = 95.0f;
        on_primary_tone = 0.0f;
        container_tone = 50.0f;
        on_container_tone = 100.0f;
        outline_tone = 80.0f;
        outline_variant_tone = 50.0f;
      }

      roles[0].hue = p_hue;
      roles[0].chroma = p_chroma;
      roles[0].tone = primary_tone;
      roles[0].offset = offsetof(struct md3_color_scheme, primary);
      roles[1].hue = p_hue;
      roles[1].chroma = p_chroma;
      roles[1].tone = on_primary_tone;
      roles[1].offset = offsetof(struct md3_color_scheme, on_primary);
      roles[2].hue = p_hue;
      roles[2].chroma = p_chroma;
      roles[2].tone = container_tone;
      roles[2].offset = offsetof(struct md3_color_scheme, primary_container);
      roles[3].hue = p_hue;
      roles[3].chroma = p_chroma;
      roles[3].tone = on_container_tone;
      roles[3].offset = offsetof(struct md3_color_scheme, on_primary_container);
      roles[4].hue = p_hue;
      roles[4].chroma = p_chroma;
      roles[4].tone = 40.0f;
      roles[4].offset = offsetof(struct md3_color_scheme, inverse_primary);

      roles[5].hue = s_hue;
      roles[5].chroma = s_chroma;
      roles[5].tone = primary_tone;
      roles[5].offset = offsetof(struct md3_color_scheme, secondary);
      roles[6].hue = s_hue;
      roles[6].chroma = s_chroma;
      roles[6].tone = on_primary_tone;
      roles[6].offset = offsetof(struct md3_color_scheme, on_secondary);
      roles[7].hue = s_hue;
      roles[7].chroma = s_chroma;
      roles[7].tone = container_tone;
      roles[7].offset = offsetof(struct md3_color_scheme, secondary_container);
      roles[8].hue = s_hue;
      roles[8].chroma = s_chroma;
      roles[8].tone = on_container_tone;
      roles[8].offset =
          offsetof(struct md3_color_scheme, on_secondary_container);

      roles[9].hue = t_hue;
      roles[9].chroma = t_chroma;
      roles[9].tone = primary_tone;
      roles[9].offset = offsetof(struct md3_color_scheme, tertiary);
      roles[10].hue = t_hue;
      roles[10].chroma = t_chroma;
      roles[10].tone = on_primary_tone;
      roles[10].offset = offsetof(struct md3_color_scheme, on_tertiary);
      roles[11].hue = t_hue;
      roles[11].chroma = t_chroma;
      roles[11].tone = container_tone;
      roles[11].offset = offsetof(struct md3_color_scheme, tertiary_container);
      roles[12].hue = t_hue;
      roles[12].chroma = t_chroma;
      roles[12].tone = on_container_tone;
      roles[12].offset =
          offsetof(struct md3_color_scheme, on_tertiary_container);

      roles[13].hue = e_hue;
      roles[13].chroma = e_chroma;
      roles[13].tone = primary_tone;
      roles[13].offset = offsetof(struct md3_color_scheme, error);
      roles[14].hue = e_hue;
      roles[14].chroma = e_chroma;
      roles[14].tone = on_primary_tone;
      roles[14].offset = offsetof(struct md3_color_scheme, on_error);
      roles[15].hue = e_hue;
      roles[15].chroma = e_chroma;
      roles[15].tone = container_tone;
      roles[15].offset = offsetof(struct md3_color_scheme, error_container);
      roles[16].hue = e_hue;
      roles[16].chroma = e_chroma;
      roles[16].tone = on_container_tone;
      roles[16].offset = offsetof(struct md3_color_scheme, on_error_container);

      roles[17].hue = n_hue;
      roles[17].chroma = n_chroma;
      roles[17].tone = 6.0f;
      roles[17].offset = offsetof(struct md3_color_scheme, surface);
      roles[18].hue = n_hue;
      roles[18].chroma = n_chroma;
      roles[18].tone = 90.0f;
      roles[18].offset = offsetof(struct md3_color_scheme, on_surface);
      roles[19].hue = nv_hue;
      roles[19].chroma = nv_chroma;
      roles[19].tone = 30.0f;
      roles[19].offset = offsetof(struct md3_color_scheme, surface_variant);
      roles[20].hue = nv_hue;
      roles[20].chroma = nv_chroma;
      roles[20].tone = 80.0f;
      roles[20].offset = offsetof(struct md3_color_scheme, on_surface_variant);
      roles[21].hue = n_hue;
      roles[21].chroma = n_chroma;
      roles[21].tone = 90.0f;
      roles[21].offset = offsetof(struct md3_color_scheme, inverse_surface);
      roles[22].hue = n_hue;
      roles[22].chroma = n_chroma;
      roles[22].tone = 20.0f;
      roles[22].offset = offsetof(struct md3_color_scheme, inverse_on_surface);

      roles[23].hue = n_hue;
      roles[23].chroma = n_chroma;
      roles[23].tone = 4.0f;
      roles[23].offset =
          offsetof(struct md3_color_scheme, surface_container_lowest);
      roles[24].hue = n_hue;
      roles[24].chroma = n_chroma;
      roles[24].tone = 10.0f;
      roles[24].offset =
          offsetof(struct md3_color_scheme, surface_container_low);
      roles[25].hue = n_hue;
      roles[25].chroma = n_chroma;
      roles[25].tone = 12.0f;
      roles[25].offset = offsetof(struct md3_color_scheme, surface_container);
      roles[26].hue = n_hue;
      roles[26].chroma = n_chroma;
      roles[26].tone = 17.0f;
      roles[26].offset =
          offsetof(struct md3_color_scheme, surface_container_high);
      roles[27].hue = n_hue;
      roles[27].chroma = n_chroma;
      roles[27].tone = 22.0f;
      roles[27].offset =
          offsetof(struct md3_color_scheme, surface_container_highest);
      roles[28].hue = n_hue;
      roles[28].chroma = n_chroma;
      roles[28].tone = 6.0f;
      roles[28].offset = offsetof(struct md3_color_scheme, surface_dim);
      roles[29].hue = n_hue;
      roles[29].chroma = n_chroma;
      roles[29].tone = 24.0f;
      roles[29].offset = offsetof(struct md3_color_scheme, surface_bright);

      roles[30].hue = nv_hue;
      roles[30].chroma = nv_chroma;
      roles[30].tone = outline_tone;
      roles[30].offset = offsetof(struct md3_color_scheme, outline);
      roles[31].hue = nv_hue;
      roles[31].chroma = nv_chroma;
      roles[31].tone = outline_variant_tone;
      roles[31].offset = offsetof(struct md3_color_scheme, outline_variant);
    }

    role_count = sizeof(roles) / sizeof(roles[0]);
    for (i = 0; i < role_count; i++) {
      ui_color_t *color_ptr =
          (ui_color_t *)((char *)out_scheme + roles[i].offset);
      rc =
          palette_tone(roles[i].hue, roles[i].chroma, roles[i].tone, color_ptr);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }

    out_scheme->surface_tint = out_scheme->primary;
  }
  out_scheme->shadow = UI_COLOR_ARGB(255, 0, 0, 0);
  out_scheme->scrim = UI_COLOR_ARGB(255, 0, 0, 0);

  return UI_ERROR_NONE;
}

/**
 * @brief Generates a full Material 3 or Material 3 Expressive color scheme from
 * a seed color.
 *
 * @param seed_color The source key color to derive palettes from.
 * @param is_dark Non-zero to generate dark theme, zero for light theme.
 * @param mode Palette derivation mode.
 * @param out_scheme Pointer to the color scheme structure to populate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_color_scheme_create(ui_color_t seed_color, int is_dark,
                                   enum md3_palette_mode mode,
                                   struct md3_color_scheme *out_scheme) {
  return md3_color_scheme_create_contrast(seed_color, is_dark, mode,
                                          MD3_CONTRAST_NORMAL, out_scheme);
}

/**
 * @brief Injects all CSS custom property tokens from a color scheme into a
 * token dictionary.
 *
 * @param scheme Pointer to the color scheme.
 * @param dict Pointer to the design token dictionary.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
struct md3_color_token_map {
  const char *name;
  size_t offset;
};

ui_error_t md3_color_scheme_apply_tokens(const struct md3_color_scheme *scheme,
                                         struct ui_design_token_dict *dict) {
  static const struct md3_color_token_map maps[] = {
      {"--md-sys-color-primary", offsetof(struct md3_color_scheme, primary)},
      {"--md-sys-color-on-primary",
       offsetof(struct md3_color_scheme, on_primary)},
      {"--md-sys-color-primary-container",
       offsetof(struct md3_color_scheme, primary_container)},
      {"--md-sys-color-on-primary-container",
       offsetof(struct md3_color_scheme, on_primary_container)},
      {"--md-sys-color-inverse-primary",
       offsetof(struct md3_color_scheme, inverse_primary)},
      {"--md-sys-color-secondary",
       offsetof(struct md3_color_scheme, secondary)},
      {"--md-sys-color-on-secondary",
       offsetof(struct md3_color_scheme, on_secondary)},
      {"--md-sys-color-secondary-container",
       offsetof(struct md3_color_scheme, secondary_container)},
      {"--md-sys-color-on-secondary-container",
       offsetof(struct md3_color_scheme, on_secondary_container)},
      {"--md-sys-color-tertiary", offsetof(struct md3_color_scheme, tertiary)},
      {"--md-sys-color-on-tertiary",
       offsetof(struct md3_color_scheme, on_tertiary)},
      {"--md-sys-color-tertiary-container",
       offsetof(struct md3_color_scheme, tertiary_container)},
      {"--md-sys-color-on-tertiary-container",
       offsetof(struct md3_color_scheme, on_tertiary_container)},
      {"--md-sys-color-error", offsetof(struct md3_color_scheme, error)},
      {"--md-sys-color-on-error", offsetof(struct md3_color_scheme, on_error)},
      {"--md-sys-color-error-container",
       offsetof(struct md3_color_scheme, error_container)},
      {"--md-sys-color-on-error-container",
       offsetof(struct md3_color_scheme, on_error_container)},
      {"--md-sys-color-surface", offsetof(struct md3_color_scheme, surface)},
      {"--md-sys-color-on-surface",
       offsetof(struct md3_color_scheme, on_surface)},
      {"--md-sys-color-surface-variant",
       offsetof(struct md3_color_scheme, surface_variant)},
      {"--md-sys-color-on-surface-variant",
       offsetof(struct md3_color_scheme, on_surface_variant)},
      {"--md-sys-color-inverse-surface",
       offsetof(struct md3_color_scheme, inverse_surface)},
      {"--md-sys-color-inverse-on-surface",
       offsetof(struct md3_color_scheme, inverse_on_surface)},
      {"--md-sys-color-surface-container-lowest",
       offsetof(struct md3_color_scheme, surface_container_lowest)},
      {"--md-sys-color-surface-container-low",
       offsetof(struct md3_color_scheme, surface_container_low)},
      {"--md-sys-color-surface-container",
       offsetof(struct md3_color_scheme, surface_container)},
      {"--md-sys-color-surface-container-high",
       offsetof(struct md3_color_scheme, surface_container_high)},
      {"--md-sys-color-surface-container-highest",
       offsetof(struct md3_color_scheme, surface_container_highest)},
      {"--md-sys-color-surface-dim",
       offsetof(struct md3_color_scheme, surface_dim)},
      {"--md-sys-color-surface-bright",
       offsetof(struct md3_color_scheme, surface_bright)},
      {"--md-sys-color-outline", offsetof(struct md3_color_scheme, outline)},
      {"--md-sys-color-outline-variant",
       offsetof(struct md3_color_scheme, outline_variant)},
      {"--md-sys-color-shadow", offsetof(struct md3_color_scheme, shadow)},
      {"--md-sys-color-scrim", offsetof(struct md3_color_scheme, scrim)},
      {"--md-sys-color-surface-tint",
       offsetof(struct md3_color_scheme, surface_tint)}};
  size_t i;
  size_t count;
  ui_error_t rc;

  if (!scheme || !dict) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  count = sizeof(maps) / sizeof(maps[0]);
  for (i = 0; i < count; i++) {
    const ui_color_t *color_ptr =
        (const ui_color_t *)((const char *)scheme + maps[i].offset);
    rc = ui_design_token_set_color(dict, maps[i].name, *color_ptr);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}
