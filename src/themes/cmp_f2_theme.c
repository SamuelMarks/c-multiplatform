/* clang-format off */
#include "themes/cmp_f2_theme.h"


/* Fluent 2 Mix Constants */
#define CMP_F2_MIX_10 0.20f
#define CMP_F2_MIX_20 0.30f
#define CMP_F2_MIX_30 0.40f
#define CMP_F2_MIX_40 0.50f
#define CMP_F2_MIX_50 0.60f
#define CMP_F2_MIX_60 0.70f
#define CMP_F2_MIX_70 0.85f
#define CMP_F2_MIX_90 0.15f
#define CMP_F2_MIX_100 0.30f
#define CMP_F2_MIX_110 0.40f
#define CMP_F2_MIX_120 0.50f
#define CMP_F2_MIX_130 0.60f
#define CMP_F2_MIX_140 0.70f
#define CMP_F2_MIX_150 0.80f
#define CMP_F2_MIX_160 0.90f

/* Fluent 2 Shadow Colors */
#define CMP_F2_SHADOW_5_PERCENT  0x0D000000U
#define CMP_F2_SHADOW_10_PERCENT 0x1A000000U
#define CMP_F2_SHADOW_12_PERCENT 0x1F000000U
#define CMP_F2_SHADOW_14_PERCENT 0x24000000U
#define CMP_F2_SHADOW_15_PERCENT 0x26000000U
#define CMP_F2_SHADOW_20_PERCENT 0x33000000U
#define CMP_F2_SHADOW_24_PERCENT 0x3D000000U

/* Fluent 2 Alpha Constants */
#define CMP_F2_ALPHA_50_PERCENT 0x80U
#define CMP_F2_ALPHA_100_PERCENT 0xFFU

/* clang-format on */

/**
 * @brief Generate a grayscale RGB value.
 * @param v The brightness value [0..255].
 * @return Packed 0xRRGGBB value.
 */
static uint32_t cmp_math_f2_gray(int v) {
  if (v < 0)
    v = 0;
  if (v > 255)
    v = 255;
  return ((uint32_t)v << 16) | ((uint32_t)v << 8) | (uint32_t)v;
}

/**
 * @brief Linearly mix two RGB colors.
 * @param color1 The start color.
 * @param color2 The end color.
 * @param t The mixing fraction [0.0..1.0].
 * @return Packed 0xRRGGBB value.
 */
static uint32_t cmp_math_f2_mix(uint32_t color1, uint32_t color2, float t) {
  uint32_t r1 = (color1 >> 16) & 0xFF;
  uint32_t g1 = (color1 >> 8) & 0xFF;
  uint32_t b1 = color1 & 0xFF;

  uint32_t r2 = (color2 >> 16) & 0xFF;
  uint32_t g2 = (color2 >> 8) & 0xFF;
  uint32_t b2 = color2 & 0xFF;

  uint32_t r = (uint32_t)(r1 + (r2 - r1) * t);
  uint32_t g = (uint32_t)(g1 + (g2 - g1) * t);
  uint32_t b = (uint32_t)(b1 + (b2 - b1) * t);

  return (r << 16) | (g << 8) | b;
}

/**
 * @brief Generate a full 16-step ramp for a given base color.
 * @param base_color The base color [0xRRGGBB].
 * @param out_ramp Pointer to the ramp structure to fill.
 */
static void cmp_f2_generate_ramp(uint32_t base_color,
                                 cmp_f2_color_ramp_t *out_ramp) {
  out_ramp->step_10 = cmp_math_f2_mix(CMP_COLOR_BLACK, base_color, CMP_F2_MIX_10);
  out_ramp->step_20 = cmp_math_f2_mix(CMP_COLOR_BLACK, base_color, CMP_F2_MIX_100);
  out_ramp->step_30 = cmp_math_f2_mix(CMP_COLOR_BLACK, base_color, CMP_F2_MIX_110);
  out_ramp->step_40 = cmp_math_f2_mix(CMP_COLOR_BLACK, base_color, CMP_F2_MIX_120);
  out_ramp->step_50 = cmp_math_f2_mix(CMP_COLOR_BLACK, base_color, CMP_F2_MIX_130);
  out_ramp->step_60 = cmp_math_f2_mix(CMP_COLOR_BLACK, base_color, CMP_F2_MIX_140);
  out_ramp->step_70 = cmp_math_f2_mix(CMP_COLOR_BLACK, base_color, CMP_F2_MIX_70);
  out_ramp->step_80 = base_color;
  out_ramp->step_90 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_90);
  out_ramp->step_100 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_100);
  out_ramp->step_110 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_110);
  out_ramp->step_120 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_120);
  out_ramp->step_130 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_130);
  out_ramp->step_140 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_140);
  out_ramp->step_150 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_150);
  out_ramp->step_160 = cmp_math_f2_mix(base_color, CMP_COLOR_WHITE, CMP_F2_MIX_160);
}

/**
 * @brief cmp_f2_theme_generate
 *
 * @param brand_color Parameter description.
 * @param is_dark Parameter description.
 * @param out_theme Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_theme_generate(uint32_t brand_color, int is_dark,
                                  cmp_f2_theme_t *out_theme) {
  int rc;
  rc = 0;
  (void)is_dark; /* To be used when mapping semantic tokens or when altering
         base tones */

  if (!out_theme) {
    return CMP_ERROR_INVALID_ARG;
  }

  /*
   * Fluent 2 Neutral Palette roughly maps to lightness.
   * Typically: Neutral10 is very dark, Neutral100 is pure white.
   */
  out_theme->neutral_10 = cmp_math_f2_gray(20);
  out_theme->neutral_12 = cmp_math_f2_gray(28);
  out_theme->neutral_14 = cmp_math_f2_gray(36);
  out_theme->neutral_16 = cmp_math_f2_gray(43);
  out_theme->neutral_20 = cmp_math_f2_gray(51);
  out_theme->neutral_22 = cmp_math_f2_gray(56);
  out_theme->neutral_24 = cmp_math_f2_gray(61);
  out_theme->neutral_26 = cmp_math_f2_gray(66);
  out_theme->neutral_28 = cmp_math_f2_gray(71);
  out_theme->neutral_30 = cmp_math_f2_gray(76);
  out_theme->neutral_32 = cmp_math_f2_gray(81);
  out_theme->neutral_34 = cmp_math_f2_gray(86);
  out_theme->neutral_36 = cmp_math_f2_gray(91);
  out_theme->neutral_38 = cmp_math_f2_gray(96);
  out_theme->neutral_40 = cmp_math_f2_gray(102);
  out_theme->neutral_42 = cmp_math_f2_gray(107);
  out_theme->neutral_44 = cmp_math_f2_gray(112);
  out_theme->neutral_46 = cmp_math_f2_gray(117);
  out_theme->neutral_48 = cmp_math_f2_gray(122);
  out_theme->neutral_50 = cmp_math_f2_gray(127);
  out_theme->neutral_60 = cmp_math_f2_gray(153);
  out_theme->neutral_70 = cmp_math_f2_gray(178);
  out_theme->neutral_80 = cmp_math_f2_gray(204);
  out_theme->neutral_90 = cmp_math_f2_gray(229);
  out_theme->neutral_100 = cmp_math_f2_gray(255);

  /* Global Brand Palette */
  (void)cmp_f2_generate_ramp(brand_color, &out_theme->brand);
  /* Global Shared Palettes */
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_RED, &out_theme->red);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_DARK_RED, &out_theme->dark_red);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_CRANBERRY, &out_theme->cranberry);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_PUMPKIN, &out_theme->pumpkin);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_PEACH, &out_theme->peach);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_MARIGOLD, &out_theme->marigold);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_YELLOW, &out_theme->yellow);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_GOLD, &out_theme->gold);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_BRASS, &out_theme->brass);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_BROWN, &out_theme->brown);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_FOREST, &out_theme->forest);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_SEAFOAM, &out_theme->seafoam);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_DARK_GREEN, &out_theme->dark_green);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_LIGHT_TEAL, &out_theme->light_teal);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_TEAL, &out_theme->teal);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_STEEL, &out_theme->steel);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_BLUE, &out_theme->blue);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_ROYAL_BLUE, &out_theme->royal_blue);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_CORNFLOWER, &out_theme->cornflower);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_NAVY, &out_theme->navy);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_LAVENDER, &out_theme->lavender);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_PURPLE, &out_theme->purple);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_GRAPE, &out_theme->grape);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_BERRY, &out_theme->berry);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_PINK, &out_theme->pink);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_MAGENTA, &out_theme->magenta);
  (void)cmp_f2_generate_ramp(CMP_F2_COLOR_PLUM, &out_theme->plum);
  /* Semantic Token Mapping */
  if (is_dark) {
    /* Layering Tokens - Dark Mode */
    out_theme->color_neutral_background_1 = out_theme->neutral_10;
    out_theme->color_neutral_background_1_hover = out_theme->neutral_14;
    out_theme->color_neutral_background_1_pressed = out_theme->neutral_12;
    out_theme->color_neutral_background_1_selected = out_theme->neutral_16;

    out_theme->color_neutral_background_2 = out_theme->neutral_12;
    out_theme->color_neutral_background_2_hover = out_theme->neutral_16;
    out_theme->color_neutral_background_2_pressed = out_theme->neutral_14;
    out_theme->color_neutral_background_2_selected = out_theme->neutral_20;

    out_theme->color_neutral_background_3 = out_theme->neutral_16;
    out_theme->color_neutral_background_3_hover = out_theme->neutral_22;
    out_theme->color_neutral_background_3_pressed = out_theme->neutral_20;
    out_theme->color_neutral_background_3_selected = out_theme->neutral_24;

    out_theme->color_neutral_background_4 = out_theme->neutral_20;
    out_theme->color_neutral_background_4_hover = out_theme->neutral_26;
    out_theme->color_neutral_background_4_pressed = out_theme->neutral_24;
    out_theme->color_neutral_background_4_selected = out_theme->neutral_28;

    out_theme->color_neutral_background_5 = out_theme->neutral_24;
    out_theme->color_neutral_background_5_hover = out_theme->neutral_30;
    out_theme->color_neutral_background_5_pressed = out_theme->neutral_28;
    out_theme->color_neutral_background_5_selected = out_theme->neutral_32;

    out_theme->color_neutral_background_6 = out_theme->neutral_28;

    /* Surface Tokens - Dark Mode */
    out_theme->color_transparent_background =
        CMP_COLOR_BLACK; /* Transparent technically, simulated as 0 */
    out_theme->color_transparent_background_hover = out_theme->neutral_14;
    out_theme->color_transparent_background_pressed = out_theme->neutral_12;
    out_theme->color_transparent_background_selected = out_theme->neutral_16;

    /* Foreground Tokens - Dark Mode */
    out_theme->color_neutral_foreground_1 = out_theme->neutral_100;
    out_theme->color_neutral_foreground_2 = out_theme->neutral_80;
    out_theme->color_neutral_foreground_3 = out_theme->neutral_60;
    out_theme->color_neutral_foreground_4 = out_theme->neutral_40;
    out_theme->color_neutral_foreground_static_inverted = out_theme->neutral_10;

    /* Brand Tokens - Dark Mode */
    out_theme->color_brand_background = out_theme->brand.step_100;
    out_theme->color_brand_background_hover = out_theme->brand.step_110;
    out_theme->color_brand_background_pressed = out_theme->brand.step_90;
    out_theme->color_brand_background_selected = out_theme->brand.step_100;
    out_theme->color_brand_foreground_1 = out_theme->brand.step_100;
    out_theme->color_brand_foreground_2 = out_theme->brand.step_110;

    /* Stroke Tokens - Dark Mode */
    out_theme->color_neutral_stroke_1 = out_theme->neutral_30;
    out_theme->color_neutral_stroke_2 = out_theme->neutral_24;
    out_theme->color_neutral_stroke_accessible = out_theme->neutral_60;

    /* Focus Tokens - Dark Mode */
    out_theme->color_stroke_focus_1 = CMP_COLOR_BLACK;
    out_theme->color_stroke_focus_2 = out_theme->neutral_100;

    /* Status Tokens - Dark Mode */
    out_theme->color_success_background = out_theme->dark_green.step_30;
    out_theme->color_success_foreground = out_theme->dark_green.step_110;
    out_theme->color_success_stroke = out_theme->dark_green.step_40;

    out_theme->color_warning_background = out_theme->marigold.step_30;
    out_theme->color_warning_foreground = out_theme->marigold.step_110;
    out_theme->color_warning_stroke = out_theme->marigold.step_40;

    out_theme->color_danger_background = out_theme->red.step_30;
    out_theme->color_danger_foreground = out_theme->red.step_110;
    out_theme->color_danger_stroke = out_theme->red.step_40;

    out_theme->color_severe_warning_background = out_theme->pumpkin.step_30;
    out_theme->color_severe_warning_foreground = out_theme->pumpkin.step_110;
    out_theme->color_severe_warning_stroke = out_theme->pumpkin.step_40;

    out_theme->color_info_background = out_theme->blue.step_30;
    out_theme->color_info_foreground = out_theme->blue.step_110;
    out_theme->color_info_stroke = out_theme->blue.step_40;
  } else {
    /* Layering Tokens - Light Mode */
    out_theme->color_neutral_background_1 = out_theme->neutral_100;
    out_theme->color_neutral_background_1_hover = out_theme->neutral_90;
    out_theme->color_neutral_background_1_pressed = out_theme->neutral_80;
    out_theme->color_neutral_background_1_selected = out_theme->neutral_90;

    out_theme->color_neutral_background_2 = out_theme->neutral_90;
    out_theme->color_neutral_background_2_hover = out_theme->neutral_80;
    out_theme->color_neutral_background_2_pressed = out_theme->neutral_70;
    out_theme->color_neutral_background_2_selected = out_theme->neutral_80;

    out_theme->color_neutral_background_3 = out_theme->neutral_80;
    out_theme->color_neutral_background_3_hover = out_theme->neutral_70;
    out_theme->color_neutral_background_3_pressed = out_theme->neutral_60;
    out_theme->color_neutral_background_3_selected = out_theme->neutral_70;

    out_theme->color_neutral_background_4 = out_theme->neutral_70;
    out_theme->color_neutral_background_4_hover = out_theme->neutral_60;
    out_theme->color_neutral_background_4_pressed = out_theme->neutral_50;
    out_theme->color_neutral_background_4_selected = out_theme->neutral_60;

    out_theme->color_neutral_background_5 = out_theme->neutral_60;
    out_theme->color_neutral_background_5_hover = out_theme->neutral_50;
    out_theme->color_neutral_background_5_pressed = out_theme->neutral_40;
    out_theme->color_neutral_background_5_selected = out_theme->neutral_50;

    out_theme->color_neutral_background_6 = out_theme->neutral_50;

    /* Surface Tokens - Light Mode */
    out_theme->color_transparent_background =
        CMP_COLOR_WHITE; /* Transparent technically */
    out_theme->color_transparent_background_hover = out_theme->neutral_90;
    out_theme->color_transparent_background_pressed = out_theme->neutral_80;
    out_theme->color_transparent_background_selected = out_theme->neutral_90;

    /* Foreground Tokens - Light Mode */
    out_theme->color_neutral_foreground_1 = out_theme->neutral_10;
    out_theme->color_neutral_foreground_2 = out_theme->neutral_30;
    out_theme->color_neutral_foreground_3 = out_theme->neutral_50;
    out_theme->color_neutral_foreground_4 = out_theme->neutral_60;
    out_theme->color_neutral_foreground_static_inverted =
        out_theme->neutral_100;

    /* Brand Tokens - Light Mode */
    out_theme->color_brand_background = out_theme->brand.step_80;
    out_theme->color_brand_background_hover = out_theme->brand.step_70;
    out_theme->color_brand_background_pressed = out_theme->brand.step_60;
    out_theme->color_brand_background_selected = out_theme->brand.step_80;
    out_theme->color_brand_foreground_1 = out_theme->brand.step_80;
    out_theme->color_brand_foreground_2 = out_theme->brand.step_70;

    /* Stroke Tokens - Light Mode */
    out_theme->color_neutral_stroke_1 = out_theme->neutral_80;
    out_theme->color_neutral_stroke_2 = out_theme->neutral_90;
    out_theme->color_neutral_stroke_accessible = out_theme->neutral_40;

    /* Focus Tokens - Light Mode */
    out_theme->color_stroke_focus_1 = out_theme->neutral_100;
    out_theme->color_stroke_focus_2 = CMP_COLOR_BLACK;

    /* Status Tokens - Light Mode */
    out_theme->color_success_background = out_theme->dark_green.step_160;
    out_theme->color_success_foreground = out_theme->dark_green.step_40;
    out_theme->color_success_stroke = out_theme->dark_green.step_120;

    out_theme->color_warning_background = out_theme->marigold.step_160;
    out_theme->color_warning_foreground = out_theme->marigold.step_40;
    out_theme->color_warning_stroke = out_theme->marigold.step_120;

    out_theme->color_danger_background = out_theme->red.step_160;
    out_theme->color_danger_foreground = out_theme->red.step_40;
    out_theme->color_danger_stroke = out_theme->red.step_120;

    out_theme->color_severe_warning_background = out_theme->pumpkin.step_160;
    out_theme->color_severe_warning_foreground = out_theme->pumpkin.step_40;
    out_theme->color_severe_warning_stroke = out_theme->pumpkin.step_120;

    out_theme->color_info_background = out_theme->blue.step_160;
    out_theme->color_info_foreground = out_theme->blue.step_40;
    out_theme->color_info_stroke = out_theme->blue.step_120;
  }

  /* Alpha/Transparent Colors (ARGB format: 0xAARRGGBB) */
  /* E.g., 50% opacity neutral background */
  if (is_dark) {
    out_theme->color_neutral_background_alpha =
        (CMP_F2_ALPHA_50_PERCENT << 24) | out_theme->neutral_12;
    out_theme->color_brand_background_alpha =
        (CMP_F2_ALPHA_50_PERCENT << 24) | out_theme->brand.step_100;
  } else {
    out_theme->color_neutral_background_alpha =
        (CMP_F2_ALPHA_50_PERCENT << 24) | out_theme->neutral_90;
    out_theme->color_brand_background_alpha =
        (CMP_F2_ALPHA_50_PERCENT << 24) | out_theme->brand.step_80;
  }

/* Typography Tokens */
#if defined(_WIN32) || defined(_WIN64)
  out_theme->font_family_base = "Segoe UI Variable, Segoe UI, sans-serif";
#elif defined(__APPLE__)
  out_theme->font_family_base =
      "system-ui, -apple-system, BlinkMacSystemFont, sans-serif";
#elif defined(__ANDROID__)
  out_theme->font_family_base = "Roboto, sans-serif";
#else
  out_theme->font_family_base =
      "Segoe UI Variable, Segoe UI, -apple-system, BlinkMacSystemFont, Roboto, "
      "Helvetica Neue, sans-serif";
#endif

  out_theme->font_family_monospace = "Consolas, Courier New, monospace";
  out_theme->font_family_numeric =
      "Segoe UI Variable, Segoe UI, sans-serif"; /* Tnum variants enabled later
                                                    via properties */

  out_theme->font_weight_regular = 400;
  out_theme->font_weight_medium = 500;
  out_theme->font_weight_semibold = 600;
  out_theme->font_weight_bold = 700;

  out_theme->font_size_caption_2 = 10.0f;
  out_theme->line_height_caption_2 = 14.0f;
  out_theme->font_size_caption_1 = 12.0f;
  out_theme->line_height_caption_1 = 16.0f;

  out_theme->font_size_body_1 = 14.0f;
  out_theme->line_height_body_1 = 20.0f;
  out_theme->font_size_body_2 = 16.0f;
  out_theme->line_height_body_2 = 22.0f;

  out_theme->font_size_subtitle_1 = 20.0f;
  out_theme->line_height_subtitle_1 = 28.0f;
  out_theme->font_size_subtitle_2 = 24.0f;
  out_theme->line_height_subtitle_2 = 32.0f;

  out_theme->font_size_title_1 = 28.0f;
  out_theme->line_height_title_1 = 36.0f;
  out_theme->font_size_title_2 = 32.0f;
  out_theme->line_height_title_2 = 40.0f;
  out_theme->font_size_title_3 = 40.0f;
  out_theme->line_height_title_3 = 52.0f;

  out_theme->font_size_display = 68.0f;
  out_theme->line_height_display = 92.0f;

  /* Letter Spacing (Tracking) */
  /* Fluent 2 specifies tighter tracking for larger fonts, looser for smaller */
  out_theme->letter_spacing_caption_2 = 0.4f;
  out_theme->letter_spacing_caption_1 = 0.3f;
  out_theme->letter_spacing_body_1 = 0.2f;
  out_theme->letter_spacing_body_2 = 0.1f;
  out_theme->letter_spacing_subtitle_1 = 0.0f;
  out_theme->letter_spacing_subtitle_2 = -0.1f;
  out_theme->letter_spacing_title_1 = -0.2f;
  out_theme->letter_spacing_title_2 = -0.3f;
  out_theme->letter_spacing_title_3 = -0.4f;
  out_theme->letter_spacing_display = -0.5f;

/* Optical Sizing support mappings */
#if defined(_WIN32) || defined(_WIN64)
  out_theme->font_family_optical_display =
      "Segoe UI Variable Display, Segoe UI, sans-serif";
  out_theme->font_family_optical_text =
      "Segoe UI Variable Text, Segoe UI, sans-serif";
  out_theme->font_family_optical_small =
      "Segoe UI Variable Small, Segoe UI, sans-serif";
#else
  out_theme->font_family_optical_display = out_theme->font_family_base;
  out_theme->font_family_optical_text = out_theme->font_family_base;
  out_theme->font_family_optical_small = out_theme->font_family_base;
#endif

  /* Spacing Tokens */
  out_theme->spacing_none = 0.0f;
  out_theme->spacing_xxs = 2.0f;
  out_theme->spacing_xs = 4.0f;
  out_theme->spacing_s = 8.0f;
  out_theme->spacing_m = 12.0f;
  out_theme->spacing_l = 16.0f;
  out_theme->spacing_xl = 20.0f;
  out_theme->spacing_xxl = 24.0f;
  out_theme->spacing_xxxl = 32.0f;
  out_theme->spacing_max = 40.0f;

  /* Shape, Borders & Corner Radius */
  out_theme->radius_none = 0.0f;
  out_theme->radius_small = 2.0f;
  out_theme->radius_medium = 4.0f;
  out_theme->radius_large = 8.0f;
  out_theme->radius_xlarge = 12.0f;
  out_theme->radius_xxlarge = 16.0f;
  out_theme->radius_circular = 9999.0f; /* 50% Pill equivalent */

  /* Stroke Widths */
  out_theme->stroke_width_thin = 1.0f;
  out_theme->stroke_width_thick = 2.0f;
  out_theme->stroke_width_thicker = 3.0f;
  out_theme->stroke_width_thickest = 4.0f;

  /* Focus Rings */
  out_theme->focus_ring_inner_width = 1.0f;
  out_theme->focus_ring_outer_width = 2.0f;
  out_theme->focus_ring_offset = 2.0f;

  /* Motion & Animation: Durations (ms) */
  out_theme->duration_ultra_fast = 50.0f;
  out_theme->duration_faster = 100.0f;
  out_theme->duration_fast = 150.0f;
  out_theme->duration_normal = 250.0f;
  out_theme->duration_gentle = 300.0f;
  out_theme->duration_slow = 400.0f;
  out_theme->duration_slower = 500.0f;

  /* Motion & Animation: Easing Curves */
  out_theme->ease_linear[0] = 0.0f;
  out_theme->ease_linear[1] = 0.0f;
  out_theme->ease_linear[2] = 1.0f;
  out_theme->ease_linear[3] = 1.0f;

  out_theme->ease_accelerate_max[0] = 1.0f;
  out_theme->ease_accelerate_max[1] = 0.0f;
  out_theme->ease_accelerate_max[2] = 1.0f;
  out_theme->ease_accelerate_max[3] = 1.0f;

  out_theme->ease_accelerate_mid[0] = 0.7f;
  out_theme->ease_accelerate_mid[1] = 0.0f;
  out_theme->ease_accelerate_mid[2] = 1.0f;
  out_theme->ease_accelerate_mid[3] = 0.5f;

  out_theme->ease_accelerate_min[0] = 0.8f;
  out_theme->ease_accelerate_min[1] = 0.0f;
  out_theme->ease_accelerate_min[2] = 1.0f;
  out_theme->ease_accelerate_min[3] = 1.0f;

  out_theme->ease_decelerate_max[0] = 0.0f;
  out_theme->ease_decelerate_max[1] = 0.0f;
  out_theme->ease_decelerate_max[2] = 0.0f;
  out_theme->ease_decelerate_max[3] = 1.0f;

  out_theme->ease_decelerate_mid[0] = 0.1f;
  out_theme->ease_decelerate_mid[1] = 0.9f;
  out_theme->ease_decelerate_mid[2] = 0.2f;
  out_theme->ease_decelerate_mid[3] = 1.0f;

  out_theme->ease_decelerate_min[0] = 0.33f;
  out_theme->ease_decelerate_min[1] = 0.0f;
  out_theme->ease_decelerate_min[2] = 0.1f;
  out_theme->ease_decelerate_min[3] = 1.0f;

  out_theme->ease_max_standard[0] = 0.0f;
  out_theme->ease_max_standard[1] = 0.0f;
  out_theme->ease_max_standard[2] = 0.0f;
  out_theme->ease_max_standard[3] = 1.0f;

  out_theme->ease_standard[0] = 0.33f;
  out_theme->ease_standard[1] = 0.0f;
  out_theme->ease_standard[2] = 0.67f;
  out_theme->ease_standard[3] = 1.0f;

  /* Elevation & Shadows */ if (is_dark) {
    /* In dark mode, shadows are typically suppressed and elevation relies on
       surface tints, but we'll define very subtle black shadows to maintain
       structural parity. */
    out_theme->shadow_2.y1 = 1.0f;
    out_theme->shadow_2.blur1 = 2.0f;
    out_theme->shadow_2.color1 = CMP_F2_SHADOW_10_PERCENT; /* 10% black */
    out_theme->shadow_2.y2 = 0.0f;
    out_theme->shadow_2.blur2 = 2.0f;
    out_theme->shadow_2.color2 = CMP_F2_SHADOW_5_PERCENT; /* 5% black */

    out_theme->shadow_4.y1 = 2.0f;
    out_theme->shadow_4.blur1 = 4.0f;
    out_theme->shadow_4.color1 = CMP_F2_SHADOW_15_PERCENT;
    out_theme->shadow_4.y2 = 0.0f;
    out_theme->shadow_4.blur2 = 2.0f;
    out_theme->shadow_4.color2 = CMP_F2_SHADOW_10_PERCENT;

    out_theme->shadow_8.y1 = 4.0f;
    out_theme->shadow_8.blur1 = 8.0f;
    out_theme->shadow_8.color1 = CMP_F2_SHADOW_15_PERCENT;
    out_theme->shadow_8.y2 = 0.0f;
    out_theme->shadow_8.blur2 = 2.0f;
    out_theme->shadow_8.color2 = CMP_F2_SHADOW_10_PERCENT;

    out_theme->shadow_16.y1 = 8.0f;
    out_theme->shadow_16.blur1 = 16.0f;
    out_theme->shadow_16.color1 = CMP_F2_SHADOW_15_PERCENT;
    out_theme->shadow_16.y2 = 0.0f;
    out_theme->shadow_16.blur2 = 2.0f;
    out_theme->shadow_16.color2 = CMP_F2_SHADOW_10_PERCENT;

    out_theme->shadow_28.y1 = 14.0f;
    out_theme->shadow_28.blur1 = 28.0f;
    out_theme->shadow_28.color1 = CMP_F2_SHADOW_20_PERCENT;
    out_theme->shadow_28.y2 = 0.0f;
    out_theme->shadow_28.blur2 = 8.0f;
    out_theme->shadow_28.color2 = CMP_F2_SHADOW_10_PERCENT;

    out_theme->shadow_64.y1 = 32.0f;
    out_theme->shadow_64.blur1 = 64.0f;
    out_theme->shadow_64.color1 = CMP_F2_SHADOW_20_PERCENT;
    out_theme->shadow_64.y2 = 0.0f;
    out_theme->shadow_64.blur2 = 16.0f;
    out_theme->shadow_64.color2 = CMP_F2_SHADOW_10_PERCENT;

    /* Materials */
    out_theme->material_mica_background = (CMP_F2_ALPHA_100_PERCENT << 24) | out_theme->neutral_10;
    out_theme->material_mica_alt_background =
        (CMP_F2_ALPHA_100_PERCENT << 24) | out_theme->neutral_12;
    out_theme->material_acrylic_background =
        (CMP_F2_ALPHA_50_PERCENT << 24) | out_theme->neutral_10;
    out_theme->material_acrylic_blur = 30.0f;
    out_theme->material_acrylic_fallback = out_theme->neutral_14;

  } else {
    /* Light mode shadows */
    out_theme->shadow_2.y1 = 1.0f;
    out_theme->shadow_2.blur1 = 2.0f;
    out_theme->shadow_2.color1 = CMP_F2_SHADOW_14_PERCENT; /* 14% */
    out_theme->shadow_2.y2 = 0.0f;
    out_theme->shadow_2.blur2 = 2.0f;
    out_theme->shadow_2.color2 = CMP_F2_SHADOW_12_PERCENT; /* 12% */

    out_theme->shadow_4.y1 = 2.0f;
    out_theme->shadow_4.blur1 = 4.0f;
    out_theme->shadow_4.color1 = CMP_F2_SHADOW_14_PERCENT;
    out_theme->shadow_4.y2 = 0.0f;
    out_theme->shadow_4.blur2 = 2.0f;
    out_theme->shadow_4.color2 = CMP_F2_SHADOW_14_PERCENT;

    out_theme->shadow_8.y1 = 4.0f;
    out_theme->shadow_8.blur1 = 8.0f;
    out_theme->shadow_8.color1 = CMP_F2_SHADOW_14_PERCENT;
    out_theme->shadow_8.y2 = 0.0f;
    out_theme->shadow_8.blur2 = 2.0f;
    out_theme->shadow_8.color2 = CMP_F2_SHADOW_12_PERCENT;

    out_theme->shadow_16.y1 = 8.0f;
    out_theme->shadow_16.blur1 = 16.0f;
    out_theme->shadow_16.color1 = CMP_F2_SHADOW_14_PERCENT;
    out_theme->shadow_16.y2 = 0.0f;
    out_theme->shadow_16.blur2 = 2.0f;
    out_theme->shadow_16.color2 = CMP_F2_SHADOW_12_PERCENT;

    out_theme->shadow_28.y1 = 14.0f;
    out_theme->shadow_28.blur1 = 28.0f;
    out_theme->shadow_28.color1 = CMP_F2_SHADOW_24_PERCENT; /* 24% */
    out_theme->shadow_28.y2 = 0.0f;
    out_theme->shadow_28.blur2 = 8.0f;
    out_theme->shadow_28.color2 = CMP_F2_SHADOW_20_PERCENT; /* 20% */

    out_theme->shadow_64.y1 = 32.0f;
    out_theme->shadow_64.blur1 = 64.0f;
    out_theme->shadow_64.color1 = CMP_F2_SHADOW_24_PERCENT;
    out_theme->shadow_64.y2 = 0.0f;
    out_theme->shadow_64.blur2 = 16.0f;
    out_theme->shadow_64.color2 = CMP_F2_SHADOW_20_PERCENT;

    /* Materials */
    out_theme->material_mica_background = (CMP_F2_ALPHA_100_PERCENT << 24) | out_theme->neutral_90;
    out_theme->material_mica_alt_background =
        (CMP_F2_ALPHA_100_PERCENT << 24) | out_theme->neutral_100;
    out_theme->material_acrylic_background =
        (CMP_F2_ALPHA_50_PERCENT << 24) | out_theme->neutral_90;
    out_theme->material_acrylic_blur = 30.0f;
    out_theme->material_acrylic_fallback = out_theme->neutral_80;
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
 * @brief cmp_f2_theme_generate_hc
 *
 * @param window Parameter description.
 * @param window_text Parameter description.
 * @param highlight Parameter description.
 * @param highlight_text Parameter description.
 * @param button_face Parameter description.
 * @param button_text Parameter description.
 * @param gray_text Parameter description.
 * @param out_theme Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int CMP_API cmp_f2_theme_generate_hc(uint32_t window, uint32_t window_text,
                                     uint32_t highlight,
                                     uint32_t highlight_text,
                                     uint32_t button_face, uint32_t button_text,
                                     uint32_t gray_text,
                                     cmp_f2_theme_t *out_theme) {
  int rc;
  rc = 0;
  (void)button_text;

  if (!out_theme) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Fill structures with HC fallbacks, collapsing the subtle shading ramps */

  /* Backgrounds map strictly to the OS Window or ButtonFace colors */
  out_theme->color_neutral_background_1 = window;
  out_theme->color_neutral_background_1_hover = highlight;
  out_theme->color_neutral_background_1_pressed = highlight;
  out_theme->color_neutral_background_1_selected = highlight;

  out_theme->color_neutral_background_2 = window;
  out_theme->color_neutral_background_2_hover = highlight;
  out_theme->color_neutral_background_2_pressed = highlight;
  out_theme->color_neutral_background_2_selected = highlight;

  out_theme->color_neutral_background_3 = button_face;
  out_theme->color_neutral_background_3_hover = highlight;
  out_theme->color_neutral_background_3_pressed = highlight;
  out_theme->color_neutral_background_3_selected = highlight;

  out_theme->color_neutral_background_4 = window;
  out_theme->color_neutral_background_5 = window;
  out_theme->color_neutral_background_6 = window;

  /* Surfaces */
  out_theme->color_transparent_background = window;
  out_theme->color_transparent_background_hover = highlight;
  out_theme->color_transparent_background_pressed = highlight;
  out_theme->color_transparent_background_selected = highlight;

  /* Foregrounds */
  out_theme->color_neutral_foreground_1 = window_text;
  out_theme->color_neutral_foreground_2 = window_text;
  out_theme->color_neutral_foreground_3 = window_text;
  out_theme->color_neutral_foreground_4 = gray_text;
  out_theme->color_neutral_foreground_static_inverted = highlight_text;

  /* Brand tokens map to Highlight for active states, WindowText for static
   * outlines */
  out_theme->color_brand_background = highlight;
  out_theme->color_brand_background_hover = highlight;
  out_theme->color_brand_background_pressed = highlight;
  out_theme->color_brand_background_selected = highlight;
  out_theme->color_brand_foreground_1 = highlight_text;
  out_theme->color_brand_foreground_2 = highlight_text;

  /* Strokes */
  out_theme->color_neutral_stroke_1 = window_text;
  out_theme->color_neutral_stroke_2 = window_text;
  out_theme->color_neutral_stroke_accessible = window_text;

  out_theme->color_stroke_focus_1 = highlight;
  out_theme->color_stroke_focus_2 = window;

  /* Status Colors */
  out_theme->color_success_background = window;
  out_theme->color_success_foreground = window_text;
  out_theme->color_success_stroke = window_text;

  out_theme->color_warning_background = window;
  out_theme->color_warning_foreground = window_text;
  out_theme->color_warning_stroke = window_text;

  out_theme->color_danger_background = window;
  out_theme->color_danger_foreground = window_text;
  out_theme->color_danger_stroke = window_text;

  out_theme->color_severe_warning_background = window;
  out_theme->color_severe_warning_foreground = window_text;
  out_theme->color_severe_warning_stroke = window_text;

  out_theme->color_info_background = window;
  out_theme->color_info_foreground = window_text;
  out_theme->color_info_stroke = window_text;

  /* Alphas remain completely opaque in HC mode for accessibility */
  out_theme->color_neutral_background_alpha = (CMP_F2_ALPHA_100_PERCENT << 24) | window;
  out_theme->color_brand_background_alpha = (CMP_F2_ALPHA_100_PERCENT << 24) | highlight;

  /* Spacing Tokens (Same as standard) */
  out_theme->spacing_none = 0.0f;
  out_theme->spacing_xxs = 2.0f;
  out_theme->spacing_xs = 4.0f;
  out_theme->spacing_s = 8.0f;
  out_theme->spacing_m = 12.0f;
  out_theme->spacing_l = 16.0f;
  out_theme->spacing_xl = 20.0f;
  out_theme->spacing_xxl = 24.0f;
  out_theme->spacing_xxxl = 32.0f;
  out_theme->spacing_max = 40.0f;

  /* Shape, Borders & Corner Radius (Often less rounded in HC, but maintaining
   * standard sizes for now) */
  out_theme->radius_none = 0.0f;
  out_theme->radius_small = 2.0f;
  out_theme->radius_medium = 4.0f;
  out_theme->radius_large = 8.0f;
  out_theme->radius_xlarge = 12.0f;
  out_theme->radius_xxlarge = 16.0f;
  out_theme->radius_circular = 9999.0f;

  /* Stroke Widths (Thicker by default in HC for visibility) */
  out_theme->stroke_width_thin = 2.0f; /* 1px -> 2px */
  out_theme->stroke_width_thick = 3.0f;
  out_theme->stroke_width_thicker = 4.0f;
  out_theme->stroke_width_thickest = 5.0f;

  /* Focus Rings (Thicker in HC) */
  out_theme->focus_ring_inner_width = 2.0f;
  out_theme->focus_ring_outer_width = 3.0f;
  out_theme->focus_ring_offset = 2.0f;

  /* Shadows - effectively suppressed in HC mode */
  out_theme->shadow_2.y1 = 0.0f;
  out_theme->shadow_2.blur1 = 0.0f;
  out_theme->shadow_2.color1 = CMP_COLOR_TRANSPARENT;
  out_theme->shadow_2.y2 = 0.0f;
  out_theme->shadow_2.blur2 = 0.0f;
  out_theme->shadow_2.color2 = CMP_COLOR_TRANSPARENT;

  out_theme->shadow_4 = out_theme->shadow_2;
  out_theme->shadow_8 = out_theme->shadow_2;
  out_theme->shadow_16 = out_theme->shadow_2;
  out_theme->shadow_28 = out_theme->shadow_2;
  out_theme->shadow_64 = out_theme->shadow_2;

  /* Materials */
  out_theme->material_mica_background = (CMP_F2_ALPHA_100_PERCENT << 24) | window;
  out_theme->material_mica_alt_background = (CMP_F2_ALPHA_100_PERCENT << 24) | window;
  out_theme->material_acrylic_background = (CMP_F2_ALPHA_100_PERCENT << 24) | window;
  out_theme->material_acrylic_blur = 0.0f;
  out_theme->material_acrylic_fallback = window;

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
