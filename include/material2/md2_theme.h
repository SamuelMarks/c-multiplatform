/**
 * @file md2_theme.h
 * @brief Complete Material Design 2 theme configuration.
 */

#ifndef MATERIAL2_MD2_THEME_H
#define MATERIAL2_MD2_THEME_H

/* clang-format off */
#include "material2/md2_color.h"
#include "material2/md2_typography.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct md2_theme
 * @brief Material Design 2 global theme configuration.
 */
struct md2_theme {
  int is_dark;
  struct md2_color_palette colors;
  float font_scale;
};

/**
 * @brief Initializes a default Material Design 2 light theme.
 *
 * @param out_theme Pointer receiving theme struct.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_theme_init_light(struct md2_theme *out_theme);

/**
 * @brief Initializes a default Material Design 2 dark theme.
 *
 * @param out_theme Pointer receiving theme struct.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_theme_init_dark(struct md2_theme *out_theme);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_THEME_H */
