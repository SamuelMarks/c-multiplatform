#ifndef M3_COLOR_H
#define M3_COLOR_H

/**
 * @file m3_color.h
 * @brief Color science helpers (HCT, palettes, schemes) for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_math.h"

/**
 * @brief Hue-Chroma-Tone color description.
 */
typedef struct M3ColorHct {
    M3Scalar hue; /**< Hue angle in degrees (0..360). */
    M3Scalar chroma; /**< Chroma intensity (>= 0). */
    M3Scalar tone; /**< Tone value (0..100). */
} M3ColorHct;

/**
 * @brief Tonal palette descriptor.
 */
typedef struct M3TonalPalette {
    M3Scalar hue; /**< Palette hue in degrees. */
    M3Scalar chroma; /**< Palette chroma. */
} M3TonalPalette;

/**
 * @brief Dynamic color scheme output.
 */
typedef struct M3Scheme {
    m3_u32 primary;
    m3_u32 on_primary;
    m3_u32 primary_container;
    m3_u32 on_primary_container;
    m3_u32 secondary;
    m3_u32 on_secondary;
    m3_u32 secondary_container;
    m3_u32 on_secondary_container;
    m3_u32 tertiary;
    m3_u32 on_tertiary;
    m3_u32 tertiary_container;
    m3_u32 on_tertiary_container;
    m3_u32 background;
    m3_u32 on_background;
    m3_u32 surface;
    m3_u32 on_surface;
    m3_u32 surface_variant;
    m3_u32 on_surface_variant;
    m3_u32 outline;
    m3_u32 error;
    m3_u32 on_error;
    m3_u32 error_container;
    m3_u32 on_error_container;
} M3Scheme;

/**
 * @brief Compose an ARGB color from RGBA components.
 * @param r Red channel.
 * @param g Green channel.
 * @param b Blue channel.
 * @param a Alpha channel.
 * @param out_argb Receives the packed ARGB color.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_argb_from_rgba(m3_u8 r, m3_u8 g, m3_u8 b, m3_u8 a, m3_u32 *out_argb);

/**
 * @brief Decompose an ARGB color into RGBA components.
 * @param argb Packed ARGB color.
 * @param out_r Receives the red channel.
 * @param out_g Receives the green channel.
 * @param out_b Receives the blue channel.
 * @param out_a Receives the alpha channel.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_rgba_from_argb(m3_u32 argb, m3_u8 *out_r, m3_u8 *out_g, m3_u8 *out_b, m3_u8 *out_a);

/**
 * @brief Convert an ARGB color to HCT.
 * @param argb Packed ARGB color.
 * @param out_hct Receives the HCT components.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_hct_from_argb(m3_u32 argb, M3ColorHct *out_hct);

/**
 * @brief Convert an HCT color to ARGB, applying gamut mapping.
 * @param hct HCT descriptor.
 * @param out_argb Receives the packed ARGB color.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_hct_to_argb(const M3ColorHct *hct, m3_u32 *out_argb);

/**
 * @brief Initialize a tonal palette.
 * @param palette Palette descriptor to initialize.
 * @param hue Palette hue in degrees.
 * @param chroma Palette chroma (>= 0).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tonal_palette_init(M3TonalPalette *palette, M3Scalar hue, M3Scalar chroma);

/**
 * @brief Retrieve an ARGB color from a palette at a tone.
 * @param palette Palette descriptor.
 * @param tone Desired tone (0..100).
 * @param out_argb Receives the packed ARGB color.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_tonal_palette_tone_argb(const M3TonalPalette *palette, M3Scalar tone, m3_u32 *out_argb);

/**
 * @brief Generate a dynamic color scheme from a source color.
 * @param source_argb Source color in ARGB.
 * @param dark M3_TRUE for dark scheme; M3_FALSE for light scheme.
 * @param out_scheme Receives the generated scheme.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_scheme_generate(m3_u32 source_argb, M3Bool dark, M3Scheme *out_scheme);

#ifdef M3_TESTING
/**
 * @brief Force argb-to-XYZ conversion failures for testing.
 * @param fail M3_TRUE to force failure; M3_FALSE to disable.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_set_argb_to_xyz_fail(M3Bool fail);

/**
 * @brief Force XYZ-to-Lab conversion failures for testing.
 * @param fail M3_TRUE to force failure; M3_FALSE to disable.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_set_xyz_to_lab_fail(M3Bool fail);

/**
 * @brief Force Lab-to-XYZ conversion failures for testing.
 * @param fail M3_TRUE to force failure; M3_FALSE to disable.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_set_lab_to_xyz_fail(M3Bool fail);

/**
 * @brief Force XYZ-to-ARGB conversion failures for testing.
 * @param fail M3_TRUE to force failure; M3_FALSE to disable.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_set_xyz_to_argb_fail(M3Bool fail);

/**
 * @brief Force LCH-to-ARGB failures on a specific call index.
 * @param call_index 0 to disable or the 1-based call index to fail.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_set_lch_to_argb_fail_call(int call_index);

/**
 * @brief Force tonal palette initialization failures on a specific call index.
 * @param call_index 0 to disable or the 1-based call index to fail.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_set_tonal_palette_init_fail_call(int call_index);

/**
 * @brief Force tonal palette tone failures on a specific call index.
 * @param call_index 0 to disable or the 1-based call index to fail.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_set_tonal_palette_tone_fail_call(int call_index);

/**
 * @brief Reset all color test failure injections.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_reset_failures(void);

/**
 * @brief Test wrapper for ARGB-to-XYZ conversion.
 * @param argb Packed ARGB color.
 * @param out_x Receives X.
 * @param out_y Receives Y.
 * @param out_z Receives Z.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_argb_to_xyz(m3_u32 argb, M3Scalar *out_x, M3Scalar *out_y, M3Scalar *out_z);

/**
 * @brief Test wrapper for XYZ-to-ARGB conversion.
 * @param x XYZ X.
 * @param y XYZ Y.
 * @param z XYZ Z.
 * @param out_argb Receives ARGB.
 * @param out_in_gamut Receives M3_TRUE if in gamut.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_xyz_to_argb(M3Scalar x, M3Scalar y, M3Scalar z, m3_u32 *out_argb, M3Bool *out_in_gamut);

/**
 * @brief Test wrapper for XYZ-to-Lab conversion.
 * @param x XYZ X.
 * @param y XYZ Y.
 * @param z XYZ Z.
 * @param out_l Receives L.
 * @param out_a Receives a.
 * @param out_b Receives b.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_xyz_to_lab(M3Scalar x, M3Scalar y, M3Scalar z, M3Scalar *out_l, M3Scalar *out_a, M3Scalar *out_b);

/**
 * @brief Test wrapper for Lab-to-XYZ conversion.
 * @param l L value.
 * @param a a value.
 * @param b b value.
 * @param out_x Receives X.
 * @param out_y Receives Y.
 * @param out_z Receives Z.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_lab_to_xyz(M3Scalar l, M3Scalar a, M3Scalar b, M3Scalar *out_x, M3Scalar *out_y, M3Scalar *out_z);

/**
 * @brief Test wrapper for LCH-to-ARGB conversion.
 * @param hue Hue in degrees.
 * @param chroma Chroma value.
 * @param tone Tone value.
 * @param out_argb Receives ARGB.
 * @param out_in_gamut Receives M3_TRUE if in gamut.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_color_test_lch_to_argb(M3Scalar hue, M3Scalar chroma, M3Scalar tone, m3_u32 *out_argb, M3Bool *out_in_gamut);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_COLOR_H */
