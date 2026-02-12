#ifndef M3_ICON_H
#define M3_ICON_H

/**
 * @file m3_icon.h
 * @brief Icon primitives and Material Symbols helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_gfx.h"

/** @brief Default Material Symbols font family. */
#define M3_ICON_DEFAULT_FAMILY "Material Symbols Outlined"
/** @brief Default icon size in pixels. */
#define M3_ICON_DEFAULT_SIZE_PX 24
/** @brief Default Material Symbols weight. */
#define M3_ICON_DEFAULT_WEIGHT 400

/** @brief Automatically pick font or SVG rendering. */
#define M3_ICON_RENDER_AUTO 0
/** @brief Force font-based rendering. */
#define M3_ICON_RENDER_FONT 1
/** @brief Force SVG path rendering. */
#define M3_ICON_RENDER_SVG 2

/**
 * @brief Icon style descriptor.
 */
typedef struct M3IconStyle {
  const char *utf8_family; /**< Font family name (UTF-8). */
  m3_i32 size_px;          /**< Icon size in pixels. */
  m3_i32 weight;           /**< Font weight (100..900). */
  M3Bool italic;           /**< M3_TRUE for italic. */
  M3Color color;           /**< Icon color (RGBA in 0..1 range). */
} M3IconStyle;

/**
 * @brief SVG path descriptor for icon fallback.
 */
typedef struct M3IconSvg {
  const char *utf8_path;   /**< SVG path data string (UTF-8). */
  M3Scalar viewbox_x;      /**< Viewbox X origin. */
  M3Scalar viewbox_y;      /**< Viewbox Y origin. */
  M3Scalar viewbox_width;  /**< Viewbox width (> 0). */
  M3Scalar viewbox_height; /**< Viewbox height (> 0). */
} M3IconSvg;

/**
 * @brief Icon measurement output.
 */
typedef struct M3IconMetrics {
  M3Scalar width;    /**< Icon width in pixels. */
  M3Scalar height;   /**< Icon height in pixels. */
  M3Scalar baseline; /**< Baseline offset in pixels. */
} M3IconMetrics;

/**
 * @brief Initialize an icon style with defaults.
 * @param style Style descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_style_init(M3IconStyle *style);

/**
 * @brief Initialize an SVG icon descriptor with defaults.
 * @param svg SVG descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_svg_init(M3IconSvg *svg);

/**
 * @brief Measure an icon specified by a UTF-8 buffer.
 * @param gfx Graphics backend interface.
 * @param style Icon style descriptor.
 * @param utf8_name UTF-8 icon name (ligature string; may be NULL when
 *                  utf8_len is 0).
 * @param utf8_len Length of the UTF-8 name in bytes.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (M3_ICON_RENDER_*).
 * @param out_metrics Receives the measured icon metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_measure_utf8(const M3Gfx *gfx,
                                        const M3IconStyle *style,
                                        const char *utf8_name,
                                        m3_usize utf8_len, const M3IconSvg *svg,
                                        m3_u32 render_mode,
                                        M3IconMetrics *out_metrics);

/**
 * @brief Measure an icon specified by a null-terminated UTF-8 string.
 * @param gfx Graphics backend interface.
 * @param style Icon style descriptor.
 * @param utf8_name Null-terminated UTF-8 icon name.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (M3_ICON_RENDER_*).
 * @param out_metrics Receives the measured icon metrics.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_measure_cstr(
    const M3Gfx *gfx, const M3IconStyle *style, const char *utf8_name,
    const M3IconSvg *svg, m3_u32 render_mode, M3IconMetrics *out_metrics);

/**
 * @brief Draw an icon specified by a UTF-8 buffer.
 * @param gfx Graphics backend interface.
 * @param bounds Destination bounds for the icon.
 * @param style Icon style descriptor.
 * @param utf8_name UTF-8 icon name (ligature string; may be NULL when
 *                  utf8_len is 0).
 * @param utf8_len Length of the UTF-8 name in bytes.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (M3_ICON_RENDER_*).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_draw_utf8(const M3Gfx *gfx, const M3Rect *bounds,
                                     const M3IconStyle *style,
                                     const char *utf8_name, m3_usize utf8_len,
                                     const M3IconSvg *svg, m3_u32 render_mode);

/**
 * @brief Draw an icon specified by a null-terminated UTF-8 string.
 * @param gfx Graphics backend interface.
 * @param bounds Destination bounds for the icon.
 * @param style Icon style descriptor.
 * @param utf8_name Null-terminated UTF-8 icon name.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (M3_ICON_RENDER_*).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_draw_cstr(const M3Gfx *gfx, const M3Rect *bounds,
                                     const M3IconStyle *style,
                                     const char *utf8_name,
                                     const M3IconSvg *svg, m3_u32 render_mode);

#ifdef M3_TESTING
/**
 * @brief Set an icon test fail point.
 * @param fail_point Fail point identifier.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_test_set_fail_point(m3_u32 fail_point);

/**
 * @brief Clear icon test fail points.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_test_clear_fail_points(void);

/**
 * @brief Override the maximum C-string length used by icon helpers.
 * @param max_len Maximum allowed length (0 disables override).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_test_set_cstr_limit(m3_usize max_len);

/**
 * @brief Test wrapper for icon style validation.
 * @param style Style to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_test_validate_style(const M3IconStyle *style);

/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_test_validate_color(const M3Color *color);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_test_validate_rect(const M3Rect *rect);

/**
 * @brief Test wrapper for SVG validation.
 * @param svg SVG descriptor to validate.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_icon_test_validate_svg(const M3IconSvg *svg);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_ICON_H */
