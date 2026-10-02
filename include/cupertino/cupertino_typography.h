/**
 * @file cupertino_typography.h
 * @brief Apple SF Typography hierarchy and Dynamic Type engine.
 */

#ifndef CUPERTINO_CUPERTINO_TYPOGRAPHY_H
#define CUPERTINO_CUPERTINO_TYPOGRAPHY_H

/* clang-format off */
#include "ui_design_tokens.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Minimum tap target dimension mandated by Apple HIG (44x44pt).
 */
#define CUPERTINO_MINIMUM_TAP_TARGET_PT 44.0f

/**
 * @enum cupertino_text_style
 * @brief Standard Apple HIG text style hierarchy.
 */
enum cupertino_text_style {
  CUPERTINO_TEXT_STYLE_LARGE_TITLE = 0,
  CUPERTINO_TEXT_STYLE_TITLE1,
  CUPERTINO_TEXT_STYLE_TITLE2,
  CUPERTINO_TEXT_STYLE_TITLE3,
  CUPERTINO_TEXT_STYLE_HEADLINE,
  CUPERTINO_TEXT_STYLE_BODY,
  CUPERTINO_TEXT_STYLE_CALLOUT,
  CUPERTINO_TEXT_STYLE_SUBHEADLINE,
  CUPERTINO_TEXT_STYLE_FOOTNOTE,
  CUPERTINO_TEXT_STYLE_CAPTION1,
  CUPERTINO_TEXT_STYLE_CAPTION2,
  CUPERTINO_TEXT_STYLE_COUNT
};

/**
 * @enum cupertino_dynamic_type_size
 * @brief Apple Dynamic Type scales (7 standard + 5 accessibility sizes).
 */
enum cupertino_dynamic_type_size {
  CUPERTINO_DYNAMIC_TYPE_XSMALL = 0,
  CUPERTINO_DYNAMIC_TYPE_SMALL,
  CUPERTINO_DYNAMIC_TYPE_MEDIUM,
  CUPERTINO_DYNAMIC_TYPE_LARGE, /**< Default reference scale. */
  CUPERTINO_DYNAMIC_TYPE_XLARGE,
  CUPERTINO_DYNAMIC_TYPE_XXLARGE,
  CUPERTINO_DYNAMIC_TYPE_XXXLARGE,
  CUPERTINO_DYNAMIC_TYPE_AX1,
  CUPERTINO_DYNAMIC_TYPE_AX2,
  CUPERTINO_DYNAMIC_TYPE_AX3,
  CUPERTINO_DYNAMIC_TYPE_AX4,
  CUPERTINO_DYNAMIC_TYPE_AX5,
  CUPERTINO_DYNAMIC_TYPE_SIZE_COUNT
};

/**
 * @struct cupertino_type_metrics
 * @brief Typographic metrics for Apple HIG text presentation.
 */
struct cupertino_type_metrics {
  float point_size;     /**< Font size in points. */
  float leading;        /**< Line height / leading in points. */
  float tracking;       /**< Dynamic tracking (kerning) in points. */
  int weight;           /**< Font weight (e.g. 400, 600, 700). */
  int use_display_face; /**< 1 for SF Pro Display (>=20pt), 0 for Text. */
};

/**
 * @brief Computes typographic metrics for a style under a given Dynamic Type
 * scale.
 *
 * @param style Text style role.
 * @param dt_size Dynamic Type scale size.
 * @param is_bold_text_enabled Non-zero to shift font weight up for
 * accessibility.
 * @param out_metrics Pointer to receive typographic metrics.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_typography_get_style(
    enum cupertino_text_style style, enum cupertino_dynamic_type_size dt_size,
    int is_bold_text_enabled, struct cupertino_type_metrics *out_metrics);

/**
 * @brief Calculates Apple SF dynamic tracking (kerning) for a given point size.
 *
 * @param point_size Font size in points (must be > 0).
 * @param out_tracking Pointer to receive tracking offset in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_typography_get_tracking(float point_size, float *out_tracking);

/**
 * @brief Clamps dimensions to Apple's minimum 44x44pt tap target boundary.
 *
 * @param width Input width in points.
 * @param height Input height in points.
 * @param out_w Pointer to receive guarded width (>= 44pt).
 * @param out_h Pointer to receive guarded height (>= 44pt).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_typography_ensure_min_tap_target(float width, float height,
                                           float *out_w, float *out_h);

/**
 * @brief Injects Apple typographic tokens into a design token dictionary.
 *
 * @param dict Pointer to design token dictionary.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_typography_apply_tokens(struct ui_design_token_dict *dict);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_TYPOGRAPHY_H */
