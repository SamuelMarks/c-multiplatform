/**
 * @file cupertino_sf_symbols.h
 * @brief Apple SF Symbols Vector Engine and Optical Weight Matching.
 */

#ifndef CUPERTINO_CUPERTINO_SF_SYMBOLS_H
#define CUPERTINO_CUPERTINO_SF_SYMBOLS_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_SYMBOL_MAX_LAYERS 3

/**
 * @enum cupertino_symbol_rendering_mode
 * @brief Apple HIG SF Symbols rendering modes.
 */
enum cupertino_symbol_rendering_mode {
  CUPERTINO_SYMBOL_RENDERING_MONOCHROME = 0, /**< Single uniform tint color. */
  CUPERTINO_SYMBOL_RENDERING_HIERARCHICAL, /**< Single color with 100%, 50%, 25%
                                              opacities. */
  CUPERTINO_SYMBOL_RENDERING_PALETTE, /**< Distinct primary, secondary, tertiary
                                         colors. */
  CUPERTINO_SYMBOL_RENDERING_MULTICOLOR /**< Spec-defined intrinsic multicolor.
                                         */
};

/**
 * @enum cupertino_symbol_weight
 * @brief SF Pro typography weight matching.
 */
enum cupertino_symbol_weight {
  CUPERTINO_SYMBOL_WEIGHT_ULTRALIGHT = 0,
  CUPERTINO_SYMBOL_WEIGHT_THIN,
  CUPERTINO_SYMBOL_WEIGHT_LIGHT,
  CUPERTINO_SYMBOL_WEIGHT_REGULAR,
  CUPERTINO_SYMBOL_WEIGHT_MEDIUM,
  CUPERTINO_SYMBOL_WEIGHT_SEMIBOLD,
  CUPERTINO_SYMBOL_WEIGHT_BOLD,
  CUPERTINO_SYMBOL_WEIGHT_HEAVY,
  CUPERTINO_SYMBOL_WEIGHT_BLACK,
  CUPERTINO_SYMBOL_WEIGHT_COUNT
};

/**
 * @enum cupertino_symbol_size
 * @brief Sizing scale relative to font metrics.
 */
enum cupertino_symbol_size {
  CUPERTINO_SYMBOL_SIZE_SMALL = 0, /**< 14pt point size. */
  CUPERTINO_SYMBOL_SIZE_MEDIUM,    /**< 18pt point size. */
  CUPERTINO_SYMBOL_SIZE_LARGE      /**< 24pt point size. */
};

/**
 * @struct cupertino_symbol_layer
 * @brief Evaluated styling parameters for a single vector layer.
 */
struct cupertino_symbol_layer {
  ui_color_t color;   /**< Evaluated color in ARGB format. */
  float opacity;      /**< Layer opacity multiplier [0.0, 1.0]. */
  float stroke_width; /**< Stroke thickness in points. */
};

/**
 * @struct cupertino_symbol_descriptor
 * @brief Configuration descriptor for creating an SF Symbol instance.
 */
struct cupertino_symbol_descriptor {
  const char *name; /**< Symbol name (e.g. "star", "gear"). */
  enum cupertino_symbol_rendering_mode rendering_mode; /**< Rendering mode. */
  enum cupertino_symbol_weight weight; /**< Typography weight. */
  enum cupertino_symbol_size size;     /**< Scale size. */
  ui_color_t primary_color;            /**< Primary tint color. */
  ui_color_t secondary_color;          /**< Secondary tint color. */
  ui_color_t tertiary_color;           /**< Tertiary tint color. */
};

/**
 * @struct cupertino_symbol
 * @brief Evaluated SF Symbol instance.
 */
struct cupertino_symbol {
  char name[64];                                       /**< Symbol name. */
  enum cupertino_symbol_rendering_mode rendering_mode; /**< Rendering mode. */
  enum cupertino_symbol_weight weight;                 /**< Weight. */
  enum cupertino_symbol_size size;                     /**< Size scale. */
  ui_color_t primary_color;                            /**< Primary tint. */
  ui_color_t secondary_color;                          /**< Secondary tint. */
  ui_color_t tertiary_color;                           /**< Tertiary tint. */
  float point_size;                                    /**< Point size. */
  float stroke_thickness;                              /**< Stroke width. */
  int layer_count;                                     /**< Layer count. */
  struct cupertino_symbol_layer
      layers[CUPERTINO_SYMBOL_MAX_LAYERS]; /**< Layer array. */
};

/**
 * @brief Creates a new Cupertino SF Symbol instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_symbol Pointer to receive newly created symbol instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_symbol_create(
    struct ui_engine *engine, const struct cupertino_symbol_descriptor *desc,
    struct cupertino_symbol **out_symbol);

/**
 * @brief Destroys an SF Symbol instance.
 *
 * @param symbol Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_symbol_destroy(struct cupertino_symbol *symbol);

/**
 * @brief Updates the glyph identifier name.
 *
 * @param symbol Target symbol.
 * @param name New symbol name.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_symbol_set_name(struct cupertino_symbol *symbol, const char *name);

/**
 * @brief Sets rendering mode (Monochrome, Hierarchical, Palette, Multicolor).
 *
 * @param symbol Target symbol.
 * @param mode Rendering mode enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_symbol_set_rendering_mode(
    struct cupertino_symbol *symbol, enum cupertino_symbol_rendering_mode mode);

/**
 * @brief Sets optical font weight matching.
 *
 * @param symbol Target symbol.
 * @param weight Typographic weight enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_symbol_set_weight(
    struct cupertino_symbol *symbol, enum cupertino_symbol_weight weight);

/**
 * @brief Sets sizing scale (Small, Medium, Large).
 *
 * @param symbol Target symbol.
 * @param size Sizing scale enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_symbol_set_size(
    struct cupertino_symbol *symbol, enum cupertino_symbol_size size);

/**
 * @brief Sets palette colors for Palette and Hierarchical rendering.
 *
 * @param symbol Target symbol.
 * @param primary Primary tint color.
 * @param secondary Secondary tint color.
 * @param tertiary Tertiary tint color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_symbol_set_palette(
    struct cupertino_symbol *symbol, ui_color_t primary, ui_color_t secondary,
    ui_color_t tertiary);

/**
 * @brief Retrieves evaluated layer properties for rendering.
 *
 * @param symbol Target symbol.
 * @param layer_index Index of the layer (0 to layer_count - 1).
 * @param out_layer Pointer to receive evaluated layer styling.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_symbol_get_layer(
    const struct cupertino_symbol *symbol, int layer_index,
    struct cupertino_symbol_layer *out_layer);

/**
 * @brief Gets bounding dimensions of the symbol.
 *
 * @param symbol Target symbol.
 * @param out_width Pointer to receive width in points.
 * @param out_height Pointer to receive height in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_symbol_get_dimensions(
    const struct cupertino_symbol *symbol, float *out_width, float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SF_SYMBOLS_H */
