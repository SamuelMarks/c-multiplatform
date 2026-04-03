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
 */
CMP_API int cmp_m3_hct_to_srgb(float hue, float chroma, float tone,
                               cmp_color_t *out_color);

/**
 * @brief Convert sRGB to HCT
 */
CMP_API int cmp_m3_srgb_to_hct(const cmp_color_t *in_color, float *out_hue,
                               float *out_chroma, float *out_tone);

/**
 * @brief Convert Display P3 to HCT
 */
CMP_API int cmp_m3_p3_to_hct(const cmp_color_t *in_color, float *out_hue,
                             float *out_chroma, float *out_tone);

/**
 * @brief Convert Linear RGB to HCT
 */
CMP_API int cmp_m3_linear_to_hct(const cmp_color_t *in_color, float *out_hue,
                                 float *out_chroma, float *out_tone);

/**
 * @brief Generate Tonal Palette (tones 0-100)
 */
CMP_API int cmp_m3_generate_tonal_palette(float hue, float chroma, float tone,
                                          cmp_palette_t *out_palette);

/**
 * @brief MCU Scheme Variants
 */
CMP_API int cmp_m3_scheme_tonal_spot(float hue, float chroma, float tone,
                                     int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_spritz(float hue, float chroma, float tone,
                                 int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_vibrant(float hue, float chroma, float tone,
                                  int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_expressive(float hue, float chroma, float tone,
                                     int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_rainbow(float hue, float chroma, float tone,
                                  int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_fruit_salad(float hue, float chroma, float tone,
                                      int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_monochrome(float hue, float chroma, float tone,
                                     int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_fidelity(float hue, float chroma, float tone,
                                   int is_dark, cmp_palette_t *out_palette);
CMP_API int cmp_m3_scheme_content(float hue, float chroma, float tone,
                                  int is_dark, cmp_palette_t *out_palette);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_MATERIAL3_COLOR_H */