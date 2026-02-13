#ifndef CMP_ICON_H
#define CMP_ICON_H

/**
 * @file cmp_icon.h
 * @brief Icon primitives and Material Symbols helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_gfx.h"

/** @brief Default Material Symbols font family. */
#define CMP_ICON_DEFAULT_FAMILY "Material Symbols Outlined"
/** @brief Default icon size in pixels. */
#define CMP_ICON_DEFAULT_SIZE_PX 24
/** @brief Default Material Symbols weight. */
#define CMP_ICON_DEFAULT_WEIGHT 400

/** @brief Automatically pick font or SVG rendering. */
#define CMP_ICON_RENDER_AUTO 0
/** @brief Force font-based rendering. */
#define CMP_ICON_RENDER_FONT 1
/** @brief Force SVG path rendering. */
#define CMP_ICON_RENDER_SVG 2

/**
 * @brief Icon style descriptor.
 */
typedef struct CMPIconStyle {
  const char *utf8_family; /**< Font family name (UTF-8). */
  cmp_i32 size_px;          /**< Icon size in pixels. */
  cmp_i32 weight;           /**< Font weight (100..900). */
  CMPBool italic;           /**< CMP_TRUE for italic. */
  CMPColor color;           /**< Icon color (RGBA in 0..1 range). */
} CMPIconStyle;

/**
 * @brief SVG path descriptor for icon fallback.
 */
typedef struct CMPIconSvg {
  const char *utf8_path;   /**< SVG path data string (UTF-8). */
  CMPScalar viewbox_x;      /**< Viewbox X origin. */
  CMPScalar viewbox_y;      /**< Viewbox Y origin. */
  CMPScalar viewbox_width;  /**< Viewbox width (> 0). */
  CMPScalar viewbox_height; /**< Viewbox height (> 0). */
} CMPIconSvg;

/**
 * @brief Icon measurement output.
 */
typedef struct CMPIconMetrics {
  CMPScalar width;    /**< Icon width in pixels. */
  CMPScalar height;   /**< Icon height in pixels. */
  CMPScalar baseline; /**< Baseline offset in pixels. */
} CMPIconMetrics;

/**
 * @brief Initialize an icon style with defaults.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_style_init(CMPIconStyle *style);

/**
 * @brief Initialize an SVG icon descriptor with defaults.
 * @param svg SVG descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_svg_init(CMPIconSvg *svg);

/**
 * @brief Measure an icon specified by a UTF-8 buffer.
 * @param gfx Graphics backend interface.
 * @param style Icon style descriptor.
 * @param utf8_name UTF-8 icon name (ligature string; may be NULL when
 *                  utf8_len is 0).
 * @param utf8_len Length of the UTF-8 name in bytes.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (CMP_ICON_RENDER_*).
 * @param out_metrics Receives the measured icon metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_measure_utf8(const CMPGfx *gfx,
                                        const CMPIconStyle *style,
                                        const char *utf8_name,
                                        cmp_usize utf8_len, const CMPIconSvg *svg,
                                        cmp_u32 render_mode,
                                        CMPIconMetrics *out_metrics);

/**
 * @brief Measure an icon specified by a null-terminated UTF-8 string.
 * @param gfx Graphics backend interface.
 * @param style Icon style descriptor.
 * @param utf8_name Null-terminated UTF-8 icon name.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (CMP_ICON_RENDER_*).
 * @param out_metrics Receives the measured icon metrics.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_measure_cstr(
    const CMPGfx *gfx, const CMPIconStyle *style, const char *utf8_name,
    const CMPIconSvg *svg, cmp_u32 render_mode, CMPIconMetrics *out_metrics);

/**
 * @brief Draw an icon specified by a UTF-8 buffer.
 * @param gfx Graphics backend interface.
 * @param bounds Destination bounds for the icon.
 * @param style Icon style descriptor.
 * @param utf8_name UTF-8 icon name (ligature string; may be NULL when
 *                  utf8_len is 0).
 * @param utf8_len Length of the UTF-8 name in bytes.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (CMP_ICON_RENDER_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_draw_utf8(const CMPGfx *gfx, const CMPRect *bounds,
                                     const CMPIconStyle *style,
                                     const char *utf8_name, cmp_usize utf8_len,
                                     const CMPIconSvg *svg, cmp_u32 render_mode);

/**
 * @brief Draw an icon specified by a null-terminated UTF-8 string.
 * @param gfx Graphics backend interface.
 * @param bounds Destination bounds for the icon.
 * @param style Icon style descriptor.
 * @param utf8_name Null-terminated UTF-8 icon name.
 * @param svg Optional SVG fallback descriptor (NULL to skip).
 * @param render_mode Rendering mode (CMP_ICON_RENDER_*).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_draw_cstr(const CMPGfx *gfx, const CMPRect *bounds,
                                     const CMPIconStyle *style,
                                     const char *utf8_name,
                                     const CMPIconSvg *svg, cmp_u32 render_mode);

#ifdef CMP_TESTING
/**
 * @brief Set an icon test fail point.
 * @param fail_point Fail point identifier.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_test_set_fail_point(cmp_u32 fail_point);

/**
 * @brief Clear icon test fail points.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_test_clear_fail_points(void);

/**
 * @brief Override the maximum C-string length used by icon helpers.
 * @param max_len Maximum allowed length (0 disables override).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_test_set_cstr_limit(cmp_usize max_len);

/**
 * @brief Test wrapper for icon style validation.
 * @param style Style to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_test_validate_style(const CMPIconStyle *style);

/**
 * @brief Test wrapper for color validation.
 * @param color Color to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_test_validate_color(const CMPColor *color);

/**
 * @brief Test wrapper for rectangle validation.
 * @param rect Rectangle to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_test_validate_rect(const CMPRect *rect);

/**
 * @brief Test wrapper for SVG validation.
 * @param svg SVG descriptor to validate.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_icon_test_validate_svg(const CMPIconSvg *svg);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_ICON_H */
