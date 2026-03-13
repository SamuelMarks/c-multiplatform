#ifndef F2_COLOR_H
#define F2_COLOR_H

/**
 * @file f2_color.h
 * @brief Microsoft Fluent 2 color system (palette generation, ramps, tokens).
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_math.h"
/* clang-format on */

/**
 * @brief Represents a Fluent 2 color ramp (10 to 100).
 */
typedef struct F2ColorRamp {
  cmp_u32 shade10;  /**< Shade 10 */
  cmp_u32 shade20;  /**< Shade 20 */
  cmp_u32 shade30;  /**< Shade 30 */
  cmp_u32 shade40;  /**< Shade 40 */
  cmp_u32 shade50;  /**< Shade 50 */
  cmp_u32 shade60;  /**< Shade 60 */
  cmp_u32 shade70;  /**< Shade 70 */
  cmp_u32 shade80;  /**< Shade 80 */
  cmp_u32 shade90;  /**< Shade 90 */
  cmp_u32 shade100; /**< Shade 100 */
} F2ColorRamp;

/**
 * @brief Represents Fluent 2 semantic colors.
 */
typedef struct F2SemanticColors {
  cmp_u32 success_background; /**< Success background */
  cmp_u32 success_text;       /**< Success text */
  cmp_u32 warning_background; /**< Warning background */
  cmp_u32 warning_text;       /**< Warning text */
  cmp_u32 danger_background;  /**< Danger background */
  cmp_u32 danger_text;        /**< Danger text */
  cmp_u32 info_background;    /**< Info background */
  cmp_u32 info_text;          /**< Info text */
} F2SemanticColors;

/**
 * @brief Fluent 2 theme tokens (light or dark).
 */
typedef struct F2Theme {
  cmp_u32 background;                /**< Base background */
  cmp_u32 fill_color_transparent;    /**< Transparent fill */
  cmp_u32 fill_color_subtle;         /**< Subtle fill */
  cmp_u32 fill_color_control;        /**< Control fill */
  cmp_u32 fill_color_text;           /**< Text fill */
  cmp_u32 accent_fill_color_default; /**< Default accent fill */

  F2ColorRamp neutral;        /**< Neutral palette */
  F2ColorRamp brand;          /**< Brand palette */
  F2SemanticColors semantics; /**< Semantic colors */
} F2Theme;

/**
 * @brief Generate a 10-100 step color ramp from a base ARGB color.
 * @param base_argb The base color in ARGB format.
 * @param out_ramp Pointer to the F2ColorRamp to receive the shades.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_color_ramp_generate(cmp_u32 base_argb,
                                            F2ColorRamp *out_ramp);

/**
 * @brief Generate a complete Fluent 2 theme based on a brand color.
 * @param brand_argb Brand key color in ARGB format.
 * @param is_dark CMP_TRUE for dark mode, CMP_FALSE for light mode.
 * @param out_theme Pointer to the F2Theme to receive the generated tokens.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL f2_theme_generate(cmp_u32 brand_argb, CMPBool is_dark,
                                       F2Theme *out_theme);

/**
 * @brief Apply Windows High Contrast Mode (HCM) mappings to a theme.
 * @param theme The theme to override with High Contrast tokens.
 * @param is_dark_hcm True if the HCM theme is a dark variant (e.g. Aquatic,
 * Desert).
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_theme_apply_high_contrast(F2Theme *theme,
                                                  CMPBool is_dark_hcm);

#ifdef __cplusplus
}
#endif

#endif /* F2_COLOR_H */
