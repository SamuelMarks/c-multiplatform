/**
 * @file sampler_theme.h
 * @brief Dynamic runtime theme model and serialization for Compose Material
 * Catalog.
 */

#ifndef SAMPLER_THEME_H
#define SAMPLER_THEME_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @enum sampler_theme_color_mode
 * @brief Determines whether system settings, light theme, or dark theme is
 * active.
 */
typedef enum sampler_theme_color_mode {
  SAMPLER_THEME_COLOR_SYSTEM = 0,
  SAMPLER_THEME_COLOR_LIGHT = 1,
  SAMPLER_THEME_COLOR_DARK = 2
} sampler_theme_color_mode_t;

/**
 * @enum sampler_color_mode
 * @brief Determines baseline, custom palette, or system dynamic color
 * extraction.
 */
typedef enum sampler_color_mode {
  SAMPLER_COLOR_BASELINE = 0,
  SAMPLER_COLOR_CUSTOM = 1,
  SAMPLER_COLOR_DYNAMIC = 2
} sampler_color_mode_t;

/**
 * @enum sampler_expressive_theme_mode
 * @brief Toggles standard baseline vs expressive typography and shapes.
 */
typedef enum sampler_expressive_theme_mode {
  SAMPLER_EXPRESSIVE_NON_EXPRESSIVE = 0,
  SAMPLER_EXPRESSIVE_EXPRESSIVE = 1
} sampler_expressive_theme_mode_t;

/**
 * @enum sampler_focus_indication_style
 * @brief Focus ring rendering style for keyboard navigation.
 */
typedef enum sampler_focus_indication_style {
  SAMPLER_FOCUS_INDICATION_OPACITY = 0,
  SAMPLER_FOCUS_INDICATION_INSET_FOCUS_RING = 1
} sampler_focus_indication_style_t;

/**
 * @enum sampler_font_scale_mode
 * @brief System font scaling or user-customized font scaling.
 */
typedef enum sampler_font_scale_mode {
  SAMPLER_FONT_SCALE_SYSTEM = 0,
  SAMPLER_FONT_SCALE_CUSTOM = 1
} sampler_font_scale_mode_t;

/**
 * @enum sampler_text_direction
 * @brief Layout direction override for testing BiDi and RTL.
 */
typedef enum sampler_text_direction {
  SAMPLER_TEXT_DIRECTION_SYSTEM = 0,
  SAMPLER_TEXT_DIRECTION_LTR = 1,
  SAMPLER_TEXT_DIRECTION_RTL = 2
} sampler_text_direction_t;

#define SAMPLER_MIN_FONT_SCALE 0.4f
#define SAMPLER_MAX_FONT_SCALE 2.0f

/**
 * @struct sampler_theme
 * @brief Complete runtime theme state matching Compose Material Catalog
 * Theme.kt.
 */
struct sampler_theme {
  sampler_theme_color_mode_t theme_color_mode;
  sampler_color_mode_t color_mode;
  sampler_expressive_theme_mode_t expressive_theme_mode;
  sampler_focus_indication_style_t focus_indication_style;
  sampler_font_scale_mode_t font_scale_mode;
  sampler_text_direction_t text_direction;
  float font_scale;
  int show_only_expressive_components;
  int mark_expressive_components;
};

/**
 * @brief Initialize a theme struct with specification baseline default values.
 * @param out_theme Pointer to struct receiving default theme configuration.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_theme_init_default(struct sampler_theme *out_theme);

/**
 * @brief Copy theme state from source to destination.
 * @param src Pointer to source theme struct.
 * @param dst Pointer to destination theme struct.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_theme_copy(const struct sampler_theme *src,
                                   struct sampler_theme *dst);

/**
 * @brief Compare two theme configurations for equality.
 * @param a Pointer to first theme struct.
 * @param b Pointer to second theme struct.
 * @param out_equals Pointer receiving 1 if equal or 0 if unequal.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_theme_equals(const struct sampler_theme *a,
                                     const struct sampler_theme *b,
                                     int *out_equals);

/**
 * @brief Serialize theme struct into key-value format matching AndroidX
 * DataStore string.
 * @param theme Theme struct to serialize.
 * @param out_buf Destination character buffer.
 * @param buf_size Size of destination buffer in bytes.
 * @param out_written Pointer receiving total number of bytes written.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t sampler_theme_serialize(const struct sampler_theme *theme,
                                        char *out_buf, size_t buf_size,
                                        size_t *out_written);

/**
 * @brief Deserialize theme struct from key-value formatted string.
 * @param buf Source string buffer.
 * @param out_theme Pointer to struct receiving parsed theme.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_PARSE_FAILURE.
 */
sampler_error_t sampler_theme_deserialize(const char *buf,
                                          struct sampler_theme *out_theme);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_THEME_H */
