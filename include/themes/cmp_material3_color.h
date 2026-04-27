#ifndef CMP_MATERIAL3_COLOR_H
#define CMP_MATERIAL3_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
/* clang-format on */

/**
 * @brief Convert HCT (Hue, Chroma, Tone) to sRGB
 *
 * Handles reversing CAM16/CIELAB mapping. Includes clamp boundary logic
 * when the exact Chroma isn't mathematically representable in sRGB space at the
 * requested Tone, ensuring visual continuity without throwing exceptions.
 *
 * @param hue       The input Hue (0-360).
 * @param chroma    The input Chroma.
 * @param tone      The input Tone (0-100).
 * @param out_color Pointer to the resulting sRGB cmp_color_t struct.
 * @return CMP_SUCCESS on success, or CMP_ERROR_INVALID_ARG.
 */
int CMP_API cmp_m3_hct_to_srgb(float hue, float chroma, float tone,
                               cmp_color_t *out_color);

/**
 * @brief Convert an sRGB color into the Hue, Chroma, Tone (HCT) color space.
 *
 * This function translates colors from standard screen RGB vectors through the
 * CAM16 perceptual color appearance model into HCT representations.
 *
 * @param in_color   The input sRGB color struct.
 * @param out_hue    Pointer to store the calculated Hue (0-360 degrees).
 * @param out_chroma Pointer to store the calculated Chroma (colorfulness).
 * @param out_tone   Pointer to store the calculated Tone (lightness 0-100).
 * @return CMP_SUCCESS on success, or CMP_ERROR_INVALID_ARG if pointers are
 * null.
 */
int CMP_API cmp_m3_srgb_to_hct(const cmp_color_t *in_color, float *out_hue,
                               float *out_chroma, float *out_tone);

/**
 * @brief Convert Display P3 to HCT
 */
int CMP_API cmp_m3_p3_to_hct(const cmp_color_t *in_color, float *out_hue,
                             float *out_chroma, float *out_tone);

/**
 * @brief Convert Linear RGB to HCT
 */
int CMP_API cmp_m3_linear_to_hct(const cmp_color_t *in_color, float *out_hue,
                                 float *out_chroma, float *out_tone);

/**
 * @brief Material 3 Tonal Palette
 */
typedef struct cmp_m3_tonal_palette {
  cmp_color_t tone0;
  cmp_color_t tone10;
  cmp_color_t tone20;
  cmp_color_t tone30;
  cmp_color_t tone40;
  cmp_color_t tone50;
  cmp_color_t tone60;
  cmp_color_t tone70;
  cmp_color_t tone80;
  cmp_color_t tone90;
  cmp_color_t tone95;
  cmp_color_t tone99;
  cmp_color_t tone100;
} cmp_m3_tonal_palette_t;

/**
 * @brief Complete set of Material 3 Core Palettes
 */
typedef struct cmp_m3_palettes {
  cmp_m3_tonal_palette_t primary;
  cmp_m3_tonal_palette_t secondary;
  cmp_m3_tonal_palette_t tertiary;
  cmp_m3_tonal_palette_t error;
  cmp_m3_tonal_palette_t neutral;
  cmp_m3_tonal_palette_t neutral_variant;
} cmp_m3_palettes_t;

/**
 * @brief Generate a single Tonal Palette from hue and chroma
 */
int CMP_API cmp_m3_generate_tonal_palette_hct(
    float hue, float chroma, cmp_m3_tonal_palette_t *out_palette);

/**
 * @brief Generate all Core Palettes from a seed color
 */
int CMP_API cmp_m3_palettes_generate(cmp_color_t seed,
                                     cmp_m3_palettes_t *out_palettes);

/**
 * @brief Generate Tonal Palette (tones 0-100)
 */
int CMP_API cmp_m3_generate_tonal_palette(float hue, float chroma, float tone,
                                          cmp_palette_t *out_palette);

/**
 * @brief MCU Scheme Variants
 */
int CMP_API cmp_m3_scheme_tonal_spot(float hue, float chroma, float tone,
                                     int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_spritz(float hue, float chroma, float tone,
                                 int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_vibrant(float hue, float chroma, float tone,
                                  int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_expressive(float hue, float chroma, float tone,
                                     int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_rainbow(float hue, float chroma, float tone,
                                  int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_fruit_salad(float hue, float chroma, float tone,
                                      int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_monochrome(float hue, float chroma, float tone,
                                     int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_fidelity(float hue, float chroma, float tone,
                                   int is_dark, cmp_palette_t *out_palette);
int CMP_API cmp_m3_scheme_content(float hue, float chroma, float tone,
                                  int is_dark, cmp_palette_t *out_palette);

/**
 * @brief Map core palettes to Light scheme semantic roles
 */
int CMP_API cmp_m3_scheme_light(const cmp_m3_palettes_t *palettes,
                                cmp_palette_t *out_scheme);

/**
 * @brief Map core palettes to Dark scheme semantic roles
 */
int CMP_API cmp_m3_scheme_dark(const cmp_m3_palettes_t *palettes,
                               cmp_palette_t *out_scheme);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_COLOR_H */