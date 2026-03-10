/* clang-format off */
#include "f2/f2_color.h"
#include "cmpc/cmp_math.h"
/* clang-format on */

static int interpolate_rgb(cmp_u8 r1, cmp_u8 g1, cmp_u8 b1, cmp_u8 r2,
                           cmp_u8 g2, cmp_u8 b2, CMPScalar t, cmp_u8 *out_r,
                           cmp_u8 *out_g, cmp_u8 *out_b) {
  *out_r = (cmp_u8)(r1 + (r2 - r1) * t);
  *out_g = (cmp_u8)(g1 + (g2 - g1) * t);
  *out_b = (cmp_u8)(b1 + (b2 - b1) * t);
  return CMP_OK;
}

CMP_API int CMP_CALL f2_color_ramp_generate(cmp_u32 base_argb,
                                            F2ColorRamp *out_ramp) {
  cmp_u8 base_r, base_g, base_b, base_a;
  cmp_u8 black_r = 0, black_g = 0, black_b = 0;
  cmp_u8 white_r = 255, white_g = 255, white_b = 255;
  cmp_u8 r, g, b;

  if (!out_ramp) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  base_a = (cmp_u8)((base_argb >> 24) & 0xFF);
  base_r = (cmp_u8)((base_argb >> 16) & 0xFF);
  base_g = (cmp_u8)((base_argb >> 8) & 0xFF);
  base_b = (cmp_u8)(base_argb & 0xFF);

  /* Shade 10 (lightest, near white) */
  interpolate_rgb(white_r, white_g, white_b, base_r, base_g, base_b, 0.1f, &r,
                  &g, &b);
  out_ramp->shade10 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 20 */
  interpolate_rgb(white_r, white_g, white_b, base_r, base_g, base_b, 0.3f, &r,
                  &g, &b);
  out_ramp->shade20 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 30 */
  interpolate_rgb(white_r, white_g, white_b, base_r, base_g, base_b, 0.5f, &r,
                  &g, &b);
  out_ramp->shade30 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 40 */
  interpolate_rgb(white_r, white_g, white_b, base_r, base_g, base_b, 0.7f, &r,
                  &g, &b);
  out_ramp->shade40 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 50 (base color) */
  out_ramp->shade50 = base_argb;

  /* Shade 60 */
  interpolate_rgb(base_r, base_g, base_b, black_r, black_g, black_b, 0.2f, &r,
                  &g, &b);
  out_ramp->shade60 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 70 */
  interpolate_rgb(base_r, base_g, base_b, black_r, black_g, black_b, 0.4f, &r,
                  &g, &b);
  out_ramp->shade70 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 80 */
  interpolate_rgb(base_r, base_g, base_b, black_r, black_g, black_b, 0.6f, &r,
                  &g, &b);
  out_ramp->shade80 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 90 */
  interpolate_rgb(base_r, base_g, base_b, black_r, black_g, black_b, 0.8f, &r,
                  &g, &b);
  out_ramp->shade90 = (base_a << 24) | (r << 16) | (g << 8) | b;

  /* Shade 100 (darkest, near black) */
  interpolate_rgb(base_r, base_g, base_b, black_r, black_g, black_b, 0.95f, &r,
                  &g, &b);
  out_ramp->shade100 = (base_a << 24) | (r << 16) | (g << 8) | b;

  return CMP_OK;
}

CMP_API int CMP_CALL f2_theme_generate(cmp_u32 brand_argb, CMPBool is_dark,
                                       F2Theme *out_theme) {
  int res;

  if (!out_theme) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  res = f2_color_ramp_generate(brand_argb, &out_theme->brand);
  if (res != CMP_OK) {
    return res;
  }

  res = f2_color_ramp_generate(0xFF808080, &out_theme->neutral);
  if (res != CMP_OK) {
    return res;
  }

  if (is_dark) {
    out_theme->background = 0xFF202020;
    out_theme->fill_color_transparent = 0x00FFFFFF;
    out_theme->fill_color_subtle = 0x0AFFFFFF;
    out_theme->fill_color_control = 0x0FFFFFFF;
    out_theme->fill_color_text = 0xFFFFFFFF;
    out_theme->accent_fill_color_default = out_theme->brand.shade60;

    out_theme->semantics.success_background = 0xFF0F5419;
    out_theme->semantics.success_text = 0xFF9FD89F;
    out_theme->semantics.warning_background = 0xFF433519;
    out_theme->semantics.warning_text = 0xFFFCE100;
    out_theme->semantics.danger_background = 0xFF8B0000;
    out_theme->semantics.danger_text = 0xFFFFB3B3;
    out_theme->semantics.info_background = 0xFF004080;
    out_theme->semantics.info_text = 0xFF99CCFF;
  } else {
    out_theme->background = 0xFFF3F3F3;
    out_theme->fill_color_transparent = 0x00000000;
    out_theme->fill_color_subtle = 0x0A000000;
    out_theme->fill_color_control = 0x0F000000;
    out_theme->fill_color_text = 0xFF000000;
    out_theme->accent_fill_color_default = out_theme->brand.shade50;

    out_theme->semantics.success_background = 0xFFDFF6DD;
    out_theme->semantics.success_text = 0xFF0F5419;
    out_theme->semantics.warning_background = 0xFFFFF4CE;
    out_theme->semantics.warning_text = 0xFF433519;
    out_theme->semantics.danger_background = 0xFFFDE7E9;
    out_theme->semantics.danger_text = 0xFFA4262C;
    out_theme->semantics.info_background = 0xFFE0F0FF;
    out_theme->semantics.info_text = 0xFF004080;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL f2_theme_apply_high_contrast(F2Theme *theme,
                                                  CMPBool is_dark_hcm) {
  if (!theme)
    return CMP_ERR_INVALID_ARGUMENT;

  if (is_dark_hcm) {
    /* Typical dark HCM (e.g. "Aquatic" or "Desert" with black background) */
    theme->background = 0xFF000000;
    theme->fill_color_text = 0xFFFFFFFF;
    theme->accent_fill_color_default = 0xFF00FFFF; /* Cyan accent */

    theme->semantics.success_background = 0xFF000000;
    theme->semantics.success_text = 0xFF00FF00;
    theme->semantics.warning_background = 0xFF000000;
    theme->semantics.warning_text = 0xFFFFFF00;
    theme->semantics.danger_background = 0xFF000000;
    theme->semantics.danger_text = 0xFFFF0000;
    theme->semantics.info_background = 0xFF000000;
    theme->semantics.info_text = 0xFF00FFFF;
  } else {
    /* Typical light HCM (e.g. white background) */
    theme->background = 0xFFFFFFFF;
    theme->fill_color_text = 0xFF000000;
    theme->accent_fill_color_default = 0xFF000080; /* Dark blue accent */

    theme->semantics.success_background = 0xFFFFFFFF;
    theme->semantics.success_text = 0xFF008000;
    theme->semantics.warning_background = 0xFFFFFFFF;
    theme->semantics.warning_text = 0xFF808000;
    theme->semantics.danger_background = 0xFFFFFFFF;
    theme->semantics.danger_text = 0xFF800000;
    theme->semantics.info_background = 0xFFFFFFFF;
    theme->semantics.info_text = 0xFF000080;
  }

  return CMP_OK;
}
