#ifndef M3_COLOR_H
#define M3_COLOR_H

/**
 * @file m3_color.h
 * @brief Color science helpers (HCT, palettes, schemes) for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"

/**
 * @brief Hue-Chroma-Tone color description.
 */
typedef struct M3ColorHct {
  CMPScalar hue;    /**< Hue angle in degrees (0..360). */
  CMPScalar chroma; /**< Chroma intensity (>= 0). */
  CMPScalar tone;   /**< Tone value (0..100). */
} M3ColorHct;

/**
 * @brief Tonal palette descriptor.
 */
typedef struct M3TonalPalette {
  CMPScalar hue;    /**< Palette hue in degrees. */
  CMPScalar chroma; /**< Palette chroma. */
} M3TonalPalette;

/**
 * @brief Dynamic color scheme output.
 */
typedef struct M3Scheme {
  cmp_u32 primary;
  cmp_u32 on_primary;
  cmp_u32 primary_container;
  cmp_u32 on_primary_container;
  cmp_u32 secondary;
  cmp_u32 on_secondary;
  cmp_u32 secondary_container;
  cmp_u32 on_secondary_container;
  cmp_u32 tertiary;
  cmp_u32 on_tertiary;
  cmp_u32 tertiary_container;
  cmp_u32 on_tertiary_container;
  cmp_u32 background;
  cmp_u32 on_background;
  cmp_u32 surface;
  cmp_u32 on_surface;
  cmp_u32 surface_variant;
  cmp_u32 on_surface_variant;
  cmp_u32 outline;
  cmp_u32 error;
  cmp_u32 on_error;
  cmp_u32 error_container;
  cmp_u32 on_error_container;
} M3Scheme;

/**
 * @brief Compose an ARGB color from RGBA components.
 * @param r Red channel.
 * @param g Green channel.
 * @param b Blue channel.
 * @param a Alpha channel.
 * @param out_argb Receives the packed ARGB color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_argb_from_rgba(cmp_u8 r, cmp_u8 g, cmp_u8 b, cmp_u8 a,
                                           cmp_u32 *out_argb);

/**
 * @brief Decompose an ARGB color into RGBA components.
 * @param argb Packed ARGB color.
 * @param out_r Receives the red channel.
 * @param out_g Receives the green channel.
 * @param out_b Receives the blue channel.
 * @param out_a Receives the alpha channel.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_rgba_from_argb(cmp_u32 argb, cmp_u8 *out_r,
                                           cmp_u8 *out_g, cmp_u8 *out_b,
                                           cmp_u8 *out_a);

/**
 * @brief Convert an ARGB color to HCT.
 * @param argb Packed ARGB color.
 * @param out_hct Receives the HCT components.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_hct_from_argb(cmp_u32 argb, M3ColorHct *out_hct);

/**
 * @brief Convert an HCT color to ARGB, applying gamut mapping.
 * @param hct HCT descriptor.
 * @param out_argb Receives the packed ARGB color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_hct_to_argb(const M3ColorHct *hct, cmp_u32 *out_argb);

/**
 * @brief Initialize a tonal palette.
 * @param palette Palette descriptor to initialize.
 * @param hue Palette hue in degrees.
 * @param chroma Palette chroma (>= 0).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tonal_palette_init(M3TonalPalette *palette, CMPScalar hue,
                                         CMPScalar chroma);

/**
 * @brief Retrieve an ARGB color from a palette at a tone.
 * @param palette Palette descriptor.
 * @param tone Desired tone (0..100).
 * @param out_argb Receives the packed ARGB color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tonal_palette_tone_argb(const M3TonalPalette *palette,
                                              CMPScalar tone, cmp_u32 *out_argb);

/**
 * @brief Generate a dynamic color scheme from a source color.
 * @param source_argb Source color in ARGB.
 * @param dark CMP_TRUE for dark scheme; CMP_FALSE for light scheme.
 * @param out_scheme Receives the generated scheme.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scheme_generate(cmp_u32 source_argb, CMPBool dark,
                                      M3Scheme *out_scheme);

#ifdef CMP_TESTING
/**
 * @brief Force argb-to-XYZ conversion failures for testing.
 * @param fail CMP_TRUE to force failure; CMP_FALSE to disable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_set_argb_to_xyz_fail(CMPBool fail);

/**
 * @brief Force XYZ-to-Lab conversion failures for testing.
 * @param fail CMP_TRUE to force failure; CMP_FALSE to disable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_set_xyz_to_lab_fail(CMPBool fail);

/**
 * @brief Force Lab-to-XYZ conversion failures for testing.
 * @param fail CMP_TRUE to force failure; CMP_FALSE to disable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_set_lab_to_xyz_fail(CMPBool fail);

/**
 * @brief Force XYZ-to-ARGB conversion failures for testing.
 * @param fail CMP_TRUE to force failure; CMP_FALSE to disable.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_set_xyz_to_argb_fail(CMPBool fail);

/**
 * @brief Force LCH-to-ARGB failures on a specific call index.
 * @param call_index 0 to disable or the 1-based call index to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_set_lch_to_argb_fail_call(int call_index);

/**
 * @brief Force tonal palette initialization failures on a specific call index.
 * @param call_index 0 to disable or the 1-based call index to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_color_test_set_tonal_palette_init_fail_call(int call_index);

/**
 * @brief Force tonal palette tone failures on a specific call index.
 * @param call_index 0 to disable or the 1-based call index to fail.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
m3_color_test_set_tonal_palette_tone_fail_call(int call_index);

/**
 * @brief Reset all color test failure injections.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_reset_failures(void);

/**
 * @brief Test wrapper for ARGB-to-XYZ conversion.
 * @param argb Packed ARGB color.
 * @param out_x Receives X.
 * @param out_y Receives Y.
 * @param out_z Receives Z.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_argb_to_xyz(cmp_u32 argb, CMPScalar *out_x,
                                             CMPScalar *out_y, CMPScalar *out_z);

/**
 * @brief Test wrapper for XYZ-to-ARGB conversion.
 * @param x XYZ X.
 * @param y XYZ Y.
 * @param z XYZ Z.
 * @param out_argb Receives ARGB.
 * @param out_in_gamut Receives CMP_TRUE if in gamut.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_xyz_to_argb(CMPScalar x, CMPScalar y, CMPScalar z,
                                             cmp_u32 *out_argb,
                                             CMPBool *out_in_gamut);

/**
 * @brief Test wrapper for XYZ-to-Lab conversion.
 * @param x XYZ X.
 * @param y XYZ Y.
 * @param z XYZ Z.
 * @param out_l Receives L.
 * @param out_a Receives a.
 * @param out_b Receives b.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_xyz_to_lab(CMPScalar x, CMPScalar y, CMPScalar z,
                                            CMPScalar *out_l, CMPScalar *out_a,
                                            CMPScalar *out_b);

/**
 * @brief Test wrapper for Lab-to-XYZ conversion.
 * @param l L value.
 * @param a a value.
 * @param b b value.
 * @param out_x Receives X.
 * @param out_y Receives Y.
 * @param out_z Receives Z.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_lab_to_xyz(CMPScalar l, CMPScalar a, CMPScalar b,
                                            CMPScalar *out_x, CMPScalar *out_y,
                                            CMPScalar *out_z);

/**
 * @brief Test wrapper for LCH-to-ARGB conversion.
 * @param hue Hue in degrees.
 * @param chroma Chroma value.
 * @param tone Tone value.
 * @param out_argb Receives ARGB.
 * @param out_in_gamut Receives CMP_TRUE if in gamut.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_lch_to_argb(CMPScalar hue, CMPScalar chroma,
                                             CMPScalar tone, cmp_u32 *out_argb,
                                             CMPBool *out_in_gamut);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_COLOR_H */
