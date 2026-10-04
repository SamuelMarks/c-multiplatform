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
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct md2_color_palette
 * @brief Complete Material Design 2 color palette (12 standard roles).
 */
struct md2_color_palette {
  ui_uint32 primary;
  ui_uint32 primary_variant;
  ui_uint32 secondary;
  ui_uint32 secondary_variant;
  ui_uint32 background;
  ui_uint32 surface;
  ui_uint32 error;
  ui_uint32 on_primary;
  ui_uint32 on_secondary;
  ui_uint32 on_background;
  ui_uint32 on_surface;
  ui_uint32 on_error;
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
#endif

#endif /* MATERIAL2_MD2_COLOR_H */
