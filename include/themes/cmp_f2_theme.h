#ifndef CMP_F2_THEME_H
#define CMP_F2_THEME_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
/* clang-format on */

/**
 * @brief Represents a single 16-step color ramp in Fluent 2 (from shade 10 to
 * 160).
 */
typedef struct cmp_f2_color_ramp_s {
  uint32_t step_10;
  uint32_t step_20;
  uint32_t step_30;
  uint32_t step_40;
  uint32_t step_50;
  uint32_t step_60;
  uint32_t step_70;
  uint32_t step_80;
  uint32_t step_90;
  uint32_t step_100;
  uint32_t step_110;
  uint32_t step_120;
  uint32_t step_130;
  uint32_t step_140;
  uint32_t step_150;
  uint32_t step_160;
} cmp_f2_color_ramp_t;

/**
 * @brief Represents a Fluent 2 composite shadow (ambient + directional/key).
 */
typedef struct cmp_f2_shadow_s {
  float y1;
  float blur1;
  uint32_t color1; /* ARGB format 0xAARRGGBB */
  float y2;
  float blur2;
  uint32_t color2; /* ARGB format 0xAARRGGBB */
} cmp_f2_shadow_t;

/**
 * @brief Microsoft Fluent 2 theme tokens.
 */
typedef struct cmp_f2_theme_s {
  /* Global Neutral Palette */
  uint32_t neutral_10;
  uint32_t neutral_12;
  uint32_t neutral_14;
  uint32_t neutral_16;
  uint32_t neutral_20;
  uint32_t neutral_22;
  uint32_t neutral_24;
  uint32_t neutral_26;
  uint32_t neutral_28;
  uint32_t neutral_30;
  uint32_t neutral_32;
  uint32_t neutral_34;
  uint32_t neutral_36;
  uint32_t neutral_38;
  uint32_t neutral_40;
  uint32_t neutral_42;
  uint32_t neutral_44;
  uint32_t neutral_46;
  uint32_t neutral_48;
  uint32_t neutral_50;
  uint32_t neutral_60;
  uint32_t neutral_70;
  uint32_t neutral_80;
  uint32_t neutral_90;
  uint32_t neutral_100;

  /* Global Brand Palette */
  cmp_f2_color_ramp_t brand;

  /* Global Shared Palettes */
  cmp_f2_color_ramp_t red;
  cmp_f2_color_ramp_t dark_red;
  cmp_f2_color_ramp_t cranberry;
  cmp_f2_color_ramp_t pumpkin;
  cmp_f2_color_ramp_t peach;
  cmp_f2_color_ramp_t marigold;
  cmp_f2_color_ramp_t yellow;
  cmp_f2_color_ramp_t gold;
  cmp_f2_color_ramp_t brass;
  cmp_f2_color_ramp_t brown;
  cmp_f2_color_ramp_t forest;
  cmp_f2_color_ramp_t seafoam;
  cmp_f2_color_ramp_t dark_green;
  cmp_f2_color_ramp_t light_teal;
  cmp_f2_color_ramp_t teal;
  cmp_f2_color_ramp_t steel;
  cmp_f2_color_ramp_t blue;
  cmp_f2_color_ramp_t royal_blue;
  cmp_f2_color_ramp_t cornflower;
  cmp_f2_color_ramp_t navy;
  cmp_f2_color_ramp_t lavender;
  cmp_f2_color_ramp_t purple;
  cmp_f2_color_ramp_t grape;
  cmp_f2_color_ramp_t berry;
  cmp_f2_color_ramp_t pink;
  cmp_f2_color_ramp_t magenta;
  cmp_f2_color_ramp_t plum;

  /* Layering Tokens */
  uint32_t color_neutral_background_1;
  uint32_t color_neutral_background_1_hover;
  uint32_t color_neutral_background_1_pressed;
  uint32_t color_neutral_background_1_selected;

  uint32_t color_neutral_background_2;
  uint32_t color_neutral_background_2_hover;
  uint32_t color_neutral_background_2_pressed;
  uint32_t color_neutral_background_2_selected;

  uint32_t color_neutral_background_3;
  uint32_t color_neutral_background_3_hover;
  uint32_t color_neutral_background_3_pressed;
  uint32_t color_neutral_background_3_selected;

  uint32_t color_neutral_background_4;
  uint32_t color_neutral_background_4_hover;
  uint32_t color_neutral_background_4_pressed;
  uint32_t color_neutral_background_4_selected;

  uint32_t color_neutral_background_5;
  uint32_t color_neutral_background_5_hover;
  uint32_t color_neutral_background_5_pressed;
  uint32_t color_neutral_background_5_selected;

  uint32_t color_neutral_background_6;

  /* Surface Tokens */
  uint32_t color_transparent_background;
  uint32_t color_transparent_background_hover;
  uint32_t color_transparent_background_pressed;
  uint32_t color_transparent_background_selected;

  /* Foreground Tokens */
  uint32_t color_neutral_foreground_1;
  uint32_t color_neutral_foreground_2;
  uint32_t color_neutral_foreground_3;
  uint32_t color_neutral_foreground_4;
  uint32_t color_neutral_foreground_static_inverted;

  /* Brand Tokens */
  uint32_t color_brand_background;
  uint32_t color_brand_background_hover;
  uint32_t color_brand_background_pressed;
  uint32_t color_brand_background_selected;
  uint32_t color_brand_foreground_1;
  uint32_t color_brand_foreground_2;

  /* Stroke Tokens */
  uint32_t color_neutral_stroke_1;
  uint32_t color_neutral_stroke_2;
  uint32_t color_neutral_stroke_accessible;

  /* Focus Tokens */
  uint32_t color_stroke_focus_1;
  uint32_t color_stroke_focus_2;

  /* Status Tokens */
  uint32_t color_success_background;
  uint32_t color_success_foreground;
  uint32_t color_success_stroke;

  uint32_t color_warning_background;
  uint32_t color_warning_foreground;
  uint32_t color_warning_stroke;

  uint32_t color_danger_background;
  uint32_t color_danger_foreground;
  uint32_t color_danger_stroke;

  uint32_t color_severe_warning_background;
  uint32_t color_severe_warning_foreground;
  uint32_t color_severe_warning_stroke;

  uint32_t color_info_background;
  uint32_t color_info_foreground;
  uint32_t color_info_stroke;
  /* Alpha/Transparent Colors */
  uint32_t color_neutral_background_alpha;
  uint32_t color_brand_background_alpha;

  /* Typography Tokens */
  const char *font_family_base;
  const char *font_family_monospace;
  const char *font_family_numeric;

  uint16_t font_weight_regular;
  uint16_t font_weight_medium;
  uint16_t font_weight_semibold;
  uint16_t font_weight_bold;

  float font_size_caption_2;
  float line_height_caption_2;
  float font_size_caption_1;
  float line_height_caption_1;

  float font_size_body_1;
  float line_height_body_1;
  float font_size_body_2;
  float line_height_body_2;

  float font_size_subtitle_1;
  float line_height_subtitle_1;
  float font_size_subtitle_2;
  float line_height_subtitle_2;

  float font_size_title_1;
  float line_height_title_1;
  float font_size_title_2;
  float line_height_title_2;
  float font_size_title_3;
  float line_height_title_3;

  float font_size_display;
  float line_height_display;

  /* Letter Spacing (Tracking) */
  float letter_spacing_caption_2;
  float letter_spacing_caption_1;
  float letter_spacing_body_1;
  float letter_spacing_body_2;
  float letter_spacing_subtitle_1;
  float letter_spacing_subtitle_2;
  float letter_spacing_title_1;
  float letter_spacing_title_2;
  float letter_spacing_title_3;
  float letter_spacing_display;

  /* Optical Sizing support mappings */
  const char *font_family_optical_display;
  const char *font_family_optical_text;
  const char *font_family_optical_small;

  /* Spacing Tokens */
  float spacing_none;
  float spacing_xxs;
  float spacing_xs;
  float spacing_s;
  float spacing_m;
  float spacing_l;
  float spacing_xl;
  float spacing_xxl;
  float spacing_xxxl;
  float spacing_max;

  /* Shape, Borders & Corner Radius */
  float radius_none;
  float radius_small;
  float radius_medium;
  float radius_large;
  float radius_xlarge;
  float radius_xxlarge;
  float radius_circular;

  /* Stroke Widths */
  float stroke_width_thin;
  float stroke_width_thick;
  float stroke_width_thicker;
  float stroke_width_thickest;

  /* Focus Rings */
  float focus_ring_inner_width;
  float focus_ring_outer_width;
  float focus_ring_offset;

  /* Motion & Animation: Durations (ms) */
  float duration_ultra_fast;
  float duration_faster;
  float duration_fast;
  float duration_normal;
  float duration_gentle;
  float duration_slow;
  float duration_slower;

  /* Motion & Animation: Easing Curves (Cubic Bezier control points: x1, y1, x2,
   * y2) */
  float ease_linear[4];
  float ease_accelerate_max[4];
  float ease_accelerate_mid[4];
  float ease_accelerate_min[4];
  float ease_decelerate_max[4];
  float ease_decelerate_mid[4];
  float ease_decelerate_min[4];
  float ease_max_standard[4];
  float ease_standard[4];

  /* Elevation & Shadows */
  cmp_f2_shadow_t shadow_2;
  cmp_f2_shadow_t shadow_4;
  cmp_f2_shadow_t shadow_8;
  cmp_f2_shadow_t shadow_16;
  cmp_f2_shadow_t shadow_28;
  cmp_f2_shadow_t shadow_64;

  /* Materials */
  uint32_t material_mica_background;
  uint32_t material_mica_alt_background;
  uint32_t material_acrylic_background;
  float material_acrylic_blur;
  uint32_t material_acrylic_fallback;
} cmp_f2_theme_t;

/**
 * @brief Generates all Fluent 2 tokens mathematically based on brand color and
 * mode.
 *
 * @param brand_color The brand color in 0xRRGGBB format.
 * @param is_dark Whether the theme is dark mode (1) or light mode (0).
 * @param out_theme Pointer to store the generated tokens.
 * @return CMP_SUCCESS on success, or an error code.
 */
CMP_API int cmp_f2_theme_generate(uint32_t brand_color, int is_dark,
                          cmp_f2_theme_t *out_theme);

/**
 * @brief Generates High Contrast tokens based on OS provided system colors.
 *
 * @param window The OS Window color.
 * @param window_text The OS WindowText color.
 * @param highlight The OS Highlight color.
 * @param highlight_text The OS HighlightText color.
 * @param button_face The OS ButtonFace color.
 * @param button_text The OS ButtonText color.
 * @param gray_text The OS GrayText color.
 * @param out_theme Pointer to store the generated high-contrast tokens.
 * @return CMP_SUCCESS on success, or an error code.
 */
CMP_API int cmp_f2_theme_generate_hc(uint32_t window, uint32_t window_text,
                             uint32_t highlight, uint32_t highlight_text,
                             uint32_t button_face, uint32_t button_text,
                             uint32_t gray_text, cmp_f2_theme_t *out_theme);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_THEME_H */
