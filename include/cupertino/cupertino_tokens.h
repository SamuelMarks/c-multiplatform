/**
 * @file cupertino_tokens.h
 * @brief Apple Human Interface Guidelines (HIG) design token definitions.
 */

#ifndef CUPERTINO_CUPERTINO_TOKENS_H
#define CUPERTINO_CUPERTINO_TOKENS_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include "ui_color_space.h"
#include "ui_design_tokens.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum cupertino_system_color
 * @brief Standard Apple iOS / macOS dynamic system colors.
 */
enum cupertino_system_color {
  CUPERTINO_COLOR_BLUE = 0,
  CUPERTINO_COLOR_GREEN,
  CUPERTINO_COLOR_INDIGO,
  CUPERTINO_COLOR_ORANGE,
  CUPERTINO_COLOR_PINK,
  CUPERTINO_COLOR_PURPLE,
  CUPERTINO_COLOR_RED,
  CUPERTINO_COLOR_TEAL,
  CUPERTINO_COLOR_YELLOW,
  CUPERTINO_COLOR_MINT,
  CUPERTINO_COLOR_CYAN,
  CUPERTINO_COLOR_COUNT
};

/**
 * @enum cupertino_gray_level
 * @brief Tiered neutral grays (SystemGray 1 through 6).
 */
enum cupertino_gray_level {
  CUPERTINO_GRAY_1 = 0,
  CUPERTINO_GRAY_2,
  CUPERTINO_GRAY_3,
  CUPERTINO_GRAY_4,
  CUPERTINO_GRAY_5,
  CUPERTINO_GRAY_6,
  CUPERTINO_GRAY_COUNT
};

/**
 * @enum cupertino_surface_level
 * @brief Layered semantic surface backgrounds.
 */
enum cupertino_surface_level {
  CUPERTINO_SURFACE_BACKGROUND = 0,
  CUPERTINO_SURFACE_SECONDARY_BACKGROUND,
  CUPERTINO_SURFACE_TERTIARY_BACKGROUND,
  CUPERTINO_SURFACE_GROUPED_BACKGROUND,
  CUPERTINO_SURFACE_SECONDARY_GROUPED_BACKGROUND,
  CUPERTINO_SURFACE_TERTIARY_GROUPED_BACKGROUND,
  CUPERTINO_SURFACE_COUNT
};

/**
 * @enum cupertino_label_level
 * @brief Semantic foreground text label hierarchies.
 */
enum cupertino_label_level {
  CUPERTINO_LABEL_PRIMARY = 0,
  CUPERTINO_LABEL_SECONDARY,
  CUPERTINO_LABEL_TERTIARY,
  CUPERTINO_LABEL_QUATERNARY,
  CUPERTINO_LABEL_COUNT
};

/**
 * @enum cupertino_theme_mode
 * @brief Cupertino appearance themes.
 */
enum cupertino_theme_mode {
  CUPERTINO_THEME_IOS_LIGHT = 0,
  CUPERTINO_THEME_IOS_DARK,
  CUPERTINO_THEME_MACOS_LIGHT,
  CUPERTINO_THEME_MACOS_DARK,
  CUPERTINO_THEME_VISIONOS,
  CUPERTINO_THEME_COUNT
};

/**
 * @brief Resolves an Apple dynamic system color for light/dark mode and high
 * contrast.
 *
 * @param color System color identifier.
 * @param is_dark 1 if dark mode, 0 if light mode.
 * @param high_contrast 1 for WCAG AAA accessible variant, 0 for standard.
 * @param out_color Pointer to store the resolved 32-bit ARGB color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_get_system_color(enum cupertino_system_color color, int is_dark,
                           int high_contrast, ui_color_t *out_color);

/**
 * @brief Resolves a tiered neutral system gray.
 *
 * @param gray_level System gray level (1 through 6).
 * @param is_dark 1 if dark mode, 0 if light mode.
 * @param out_color Pointer to store the resolved 32-bit ARGB color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_get_system_gray(
    enum cupertino_gray_level gray_level, int is_dark, ui_color_t *out_color);

/**
 * @brief Resolves a semantic surface background color.
 *
 * @param surface Surface level.
 * @param is_dark 1 if dark mode, 0 if light mode.
 * @param out_color Pointer to store the resolved 32-bit ARGB color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_get_surface_color(
    enum cupertino_surface_level surface, int is_dark, ui_color_t *out_color);

/**
 * @brief Resolves a semantic text label foreground color with opacity.
 *
 * @param label Label hierarchy level.
 * @param is_dark 1 if dark mode, 0 if light mode.
 * @param out_color Pointer to store the resolved 32-bit ARGB color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_get_label_color(
    enum cupertino_label_level label, int is_dark, ui_color_t *out_color);

/**
 * @brief Resolves separator, opaque separator, or link colors.
 *
 * @param token_name Name of the semantic token (e.g. "--apple-separator").
 * @param is_dark 1 if dark mode, 0 if light mode.
 * @param out_color Pointer to store the resolved 32-bit ARGB color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_resolve_named_color(
    const char *token_name, int is_dark, ui_color_t *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TOKENS_H */
