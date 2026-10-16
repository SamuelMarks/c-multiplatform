/**
 * @file md2_color.h
 * @brief Material Design 2 color system and baseline palettes.
 */

#ifndef MATERIAL2_MD2_COLOR_H
#define MATERIAL2_MD2_COLOR_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @struct md2_color_palette
 * @brief Complete Material Design 2 color palette (12 standard roles).
 */
struct md2_color_palette {
  ui_uint32 primary;           /**< primary field. */
  ui_uint32 primary_variant;   /**< primary_variant field. */
  ui_uint32 secondary;         /**< secondary field. */
  ui_uint32 secondary_variant; /**< secondary_variant field. */
  ui_uint32 background;        /**< background field. */
  ui_uint32 surface;           /**< surface field. */
  ui_uint32 error;             /**< error field. */
  ui_uint32 on_primary;        /**< on_primary field. */
  ui_uint32 on_secondary;      /**< on_secondary field. */
  ui_uint32 on_background;     /**< on_background field. */
  ui_uint32 on_surface;        /**< on_surface field. */
  ui_uint32 on_error;          /**< on_error field. */
};

/**
 * @brief Initializes a color palette with Material Design 2 light theme
 * baseline tokens.
 *
 * @param out_palette Pointer to struct receiving light theme colors.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_color_palette_init_light(struct md2_color_palette *out_palette);

/**
 * @brief Initializes a color palette with Material Design 2 dark theme baseline
 * tokens.
 *
 * @param out_palette Pointer to struct receiving dark theme colors.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_color_palette_init_dark(struct md2_color_palette *out_palette);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL2_MD2_COLOR_H */
