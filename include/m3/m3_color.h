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
/**
 * @brief Represents a full Material 3 color scheme.
 */
typedef struct M3Scheme {
  cmp_u32 primary;                 /**< Primary color. */
  cmp_u32 on_primary;              /**< On-primary color. */
  cmp_u32 primary_container;       /**< Primary container color. */
  cmp_u32 on_primary_container;    /**< On-primary container color. */
  cmp_u32 secondary;               /**< Secondary color. */
  cmp_u32 on_secondary;            /**< On-secondary color. */
  cmp_u32 secondary_container;     /**< Secondary container color. */
  cmp_u32 on_secondary_container;  /**< On-secondary container color. */
  cmp_u32 tertiary;                /**< Tertiary color. */
  cmp_u32 on_tertiary;             /**< On-tertiary color. */
  cmp_u32 tertiary_container;      /**< Tertiary container color. */
  cmp_u32 on_tertiary_container;   /**< On-tertiary container color. */
  cmp_u32 background;              /**< Background color. */
  cmp_u32 on_background;           /**< On-background color. */
  cmp_u32 surface;                 /**< Surface color. */
  cmp_u32 on_surface;              /**< On-surface color. */
  cmp_u32 surface_variant;         /**< Surface variant color. */
  cmp_u32 on_surface_variant;      /**< On-surface variant color. */
  cmp_u32 outline;                 /**< Outline color. */
  cmp_u32 error;                   /**< Error color. */
  cmp_u32 on_error;                /**< On-error color. */
  cmp_u32 error_container;         /**< Error container color. */
  cmp_u32 on_error_container;      /**< On-error container color. */
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
CMP_API int CMP_CALL m3_color_argb_from_rgba(cmp_u8 r, cmp_u8 g, cmp_u8 b,
                                             cmp_u8 a, cmp_u32 *out_argb);

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
CMP_API int CMP_CALL m3_tonal_palette_init(M3TonalPalette *palette,
                                           CMPScalar hue, CMPScalar chroma);

/**
 * @brief Retrieve an ARGB color from a palette at a tone.
 * @param palette Palette descriptor.
 * @param tone Desired tone (0..100).
 * @param out_argb Receives the packed ARGB color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_tonal_palette_tone_argb(const M3TonalPalette *palette,
                                                CMPScalar tone,
                                                cmp_u32 *out_argb);

/**
 * @brief Dynamic color scheme variants (Material Design 3).
 */
typedef enum M3SchemeVariant {
  M3_SCHEME_VARIANT_TONAL_SPOT = 0, /**< Default: Tonal Spot. */
  M3_SCHEME_VARIANT_EXPRESSIVE, /**< Expressive: Shifted hues, high chroma. */
  M3_SCHEME_VARIANT_FIDELITY,   /**< Fidelity: Matches source color exactly. */
  M3_SCHEME_VARIANT_CONTENT,    /**< Content: Designed for image extraction. */
  M3_SCHEME_VARIANT_VIBRANT,    /**< Vibrant: Maximum chroma scheme. */
  M3_SCHEME_VARIANT_NEUTRAL     /**< Neutral: Low chroma throughout. */
} M3SchemeVariant;

/**
 * @brief Generate a dynamic color scheme from a source color.
 * @param source_argb Source color in ARGB.
 * @param dark CMP_TRUE for dark scheme; CMP_FALSE for light scheme.
 * @param out_scheme Receives the generated scheme.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scheme_generate(cmp_u32 source_argb, CMPBool dark,
                                        M3Scheme *out_scheme);

/**
 * @brief Generate a specific variant of dynamic color scheme.
 * @param source_argb Source color in ARGB.
 * @param dark CMP_TRUE for dark scheme; CMP_FALSE for light scheme.
 * @param variant The scheme variant to generate.
 * @param out_scheme Receives the generated scheme.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scheme_generate_variant(cmp_u32 source_argb,
                                                CMPBool dark,
                                                M3SchemeVariant variant,
                                                M3Scheme *out_scheme);

/**
 * @brief Get the surface tint opacity for a specific elevation level (0..5).
 * @param elevation_level Elevation level (0 to 5).
 * @param out_opacity Receives the alpha multiplier (0.0 to 1.0).
 * @return CMP_OK on success or a failure code.
 */
struct CMPWS;

/**
 * @brief Generate a dynamic color scheme from the system accent color.
 * @param ws Window system instance.
 * @param dark CMP_TRUE for dark scheme; CMP_FALSE for light scheme.
 * @param out_scheme Receives the generated scheme.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_scheme_generate_system(struct CMPWS *ws, CMPBool dark,
                                               M3Scheme *out_scheme);

/**
 * @brief Get the surface tint opacity for a given elevation level.
 * @param elevation_level The elevation level.
 * @param out_opacity Output pointer for the opacity scalar.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL m3_color_surface_tint_opacity(cmp_u32 elevation_level,
                                                   CMPScalar *out_opacity);

/**
 * @brief Blend a foreground tint color over a background surface color.
 * @param bg_argb Background color in ARGB format.
 * @param fg_argb Foreground (tint) color in ARGB format.
 * @param fg_alpha Alpha multiplier for the foreground color (0.0 to 1.0).
 * @param out_argb Receives the blended ARGB color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_blend_surface_tint(cmp_u32 bg_argb,
                                                 cmp_u32 fg_argb,
                                                 CMPScalar fg_alpha,
                                                 cmp_u32 *out_argb);

#if defined(CMP_TESTING) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
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
                                               CMPScalar *out_y,
                                               CMPScalar *out_z);

/**
 * @brief Test wrapper for XYZ-to-ARGB conversion.
 * @param x XYZ X.
 * @param y XYZ Y.
 * @param z XYZ Z.
 * @param out_argb Receives ARGB.
 * @param out_in_gamut Receives CMP_TRUE if in gamut.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_test_xyz_to_argb(CMPScalar x, CMPScalar y,
                                               CMPScalar z, cmp_u32 *out_argb,
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
CMP_API int CMP_CALL m3_color_test_xyz_to_lab(CMPScalar x, CMPScalar y,
                                              CMPScalar z, CMPScalar *out_l,
                                              CMPScalar *out_a,
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
CMP_API int CMP_CALL m3_color_test_lab_to_xyz(CMPScalar l, CMPScalar a,
                                              CMPScalar b, CMPScalar *out_x,
                                              CMPScalar *out_y,
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
                                               CMPScalar tone,
                                               cmp_u32 *out_argb,
                                               CMPBool *out_in_gamut);
#endif

/**
 * @brief Extract a prominent seed color from an image buffer using
 * quantization.
 * @param argb_pixels Array of ARGB pixel data.
 * @param pixel_count Number of pixels in the array.
 * @param out_seed_argb Receives the extracted ARGB seed color.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL m3_color_extract_seed_from_image(
    const cmp_u32 *argb_pixels, cmp_usize pixel_count, cmp_u32 *out_seed_argb);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_COLOR_H */
