/**
 * @file md3_color.h
 * @brief Material 3 and Material 3 Expressive dynamic color schemes,
 * tonal palette generation, and harmonization.
 */

#ifndef MATERIAL3_MD3_COLOR_H
#define MATERIAL3_MD3_COLOR_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_design_tokens.h"
#include "ui_error.h"
#include "ui_tonal_palette.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @enum md3_palette_mode
 * @brief Generation modes for tonal palettes and dynamic schemes.
 */
enum md3_palette_mode {
  MD3_PALETTE_MODE_TONAL_SPOT = 0, /**< Standard Material 3 baseline default */
  MD3_PALETTE_MODE_EXPRESSIVE,     /**< Higher chroma and shifted tertiary */
  MD3_PALETTE_MODE_VIBRANT,        /**< Maximum chroma preservation */
  MD3_PALETTE_MODE_CONTENT_BASED,  /**< Strict fidelity to source image */
  MD3_PALETTE_MODE_MONOCHROME,     /**< Grayscale 0-chroma scheme */
  MD3_PALETTE_MODE_FRUIT_SALAD,    /**< Playful contrasting two-tone */
  MD3_PALETTE_MODE_RAINBOW         /**< Multi-chromatic palette */
};

/**
 * @struct md3_color_scheme
 * @brief Complete set of Material 3 and Material 3 Expressive system colors.
 */
struct md3_color_scheme {
  /* Primary roles */
  ui_color_t primary;              /**< primary field. */
  ui_color_t on_primary;           /**< on_primary field. */
  ui_color_t primary_container;    /**< primary_container field. */
  ui_color_t on_primary_container; /**< on_primary_container field. */
  ui_color_t inverse_primary;      /**< inverse_primary field. */

  /* Secondary roles */
  ui_color_t secondary;              /**< secondary field. */
  ui_color_t on_secondary;           /**< on_secondary field. */
  ui_color_t secondary_container;    /**< secondary_container field. */
  ui_color_t on_secondary_container; /**< on_secondary_container field. */

  /* Tertiary roles */
  ui_color_t tertiary;              /**< tertiary field. */
  ui_color_t on_tertiary;           /**< on_tertiary field. */
  ui_color_t tertiary_container;    /**< tertiary_container field. */
  ui_color_t on_tertiary_container; /**< on_tertiary_container field. */

  /* Error roles */
  ui_color_t error;              /**< error field. */
  ui_color_t on_error;           /**< on_error field. */
  ui_color_t error_container;    /**< error_container field. */
  ui_color_t on_error_container; /**< on_error_container field. */

  /* Surface & Background roles */
  ui_color_t surface;            /**< surface field. */
  ui_color_t on_surface;         /**< on_surface field. */
  ui_color_t surface_variant;    /**< surface_variant field. */
  ui_color_t on_surface_variant; /**< on_surface_variant field. */
  ui_color_t inverse_surface;    /**< inverse_surface field. */
  ui_color_t inverse_on_surface; /**< inverse_on_surface field. */

  /* Expressive Surface Containers */
  ui_color_t surface_container_lowest;  /**< surface_container_lowest field. */
  ui_color_t surface_container_low;     /**< surface_container_low field. */
  ui_color_t surface_container;         /**< surface_container field. */
  ui_color_t surface_container_high;    /**< surface_container_high field. */
  ui_color_t surface_container_highest; /**< surface_container_highest field. */
  ui_color_t surface_dim;               /**< surface_dim field. */
  ui_color_t surface_bright;            /**< surface_bright field. */

  /* Outlines & Utilities */
  ui_color_t outline;         /**< outline field. */
  ui_color_t outline_variant; /**< outline_variant field. */
  ui_color_t shadow;          /**< shadow field. */
  ui_color_t scrim;           /**< scrim field. */
  ui_color_t surface_tint;    /**< surface_tint field. */
};

/**
 * @enum md3_contrast_level
 * @brief Contrast levels for accessible color scheme variants.
 */
enum md3_contrast_level {
  MD3_CONTRAST_NORMAL = 0, /**< Standard contrast */
  MD3_CONTRAST_MEDIUM,     /**< Medium contrast (4.5:1 accessible) */
  MD3_CONTRAST_HIGH        /**< High contrast (7:1 accessible) */
};

/**
 * @brief Shifts the hue of a design color toward a source color
 * (Harmonization).
 *
 * @param design_color The color to harmonize.
 * @param source_color The target anchor color (e.g. primary seed).
 * @param out_color Pointer to store the harmonized color.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_harmonize(
    ui_color_t design_color, ui_color_t source_color, ui_color_t *out_color);

/**
 * @brief Generates a full Material 3 or Material 3 Expressive color scheme from
 * a seed color.
 *
 * @param seed_color The source key color to derive palettes from.
 * @param is_dark Non-zero to generate dark theme, zero for light theme.
 * @param mode Palette derivation mode (e.g. tonal spot, expressive, vibrant,
 * monochrome).
 * @param out_scheme Pointer to the color scheme structure to populate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_scheme_create(
    ui_color_t seed_color, int is_dark, enum md3_palette_mode mode,
    struct md3_color_scheme *out_scheme);

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
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_scheme_create_contrast(
    ui_color_t seed_color, int is_dark, enum md3_palette_mode mode,
    enum md3_contrast_level contrast, struct md3_color_scheme *out_scheme);

/**
 * @brief Injects all CSS custom property tokens from a color scheme into a
 * token dictionary.
 *
 * @param scheme Pointer to the color scheme.
 * @param dict Pointer to the design token dictionary.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_color_scheme_apply_tokens(
    const struct md3_color_scheme *scheme, struct ui_design_token_dict *dict);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_COLOR_H */
