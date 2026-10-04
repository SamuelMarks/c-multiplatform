/**
 * @file md2_typography.h
 * @brief Material Design 2 typography scale definitions and font styling.
 */

#ifndef MATERIAL2_MD2_TYPOGRAPHY_H
#define MATERIAL2_MD2_TYPOGRAPHY_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md2_typography_style
 * @brief The 13 standard Material Design 2 typography styles.
 */
enum md2_typography_style {
  MD2_TYPOGRAPHY_H1 = 0,
  MD2_TYPOGRAPHY_H2 = 1,
  MD2_TYPOGRAPHY_H3 = 2,
  MD2_TYPOGRAPHY_H4 = 3,
  MD2_TYPOGRAPHY_H5 = 4,
  MD2_TYPOGRAPHY_H6 = 5,
  MD2_TYPOGRAPHY_SUBTITLE1 = 6,
  MD2_TYPOGRAPHY_SUBTITLE2 = 7,
  MD2_TYPOGRAPHY_BODY1 = 8,
  MD2_TYPOGRAPHY_BODY2 = 9,
  MD2_TYPOGRAPHY_BUTTON = 10,
  MD2_TYPOGRAPHY_CAPTION = 11,
  MD2_TYPOGRAPHY_OVERLINE = 12
};

/**
 * @struct md2_text_style
 * @brief Attributes for a Material Design 2 typography style.
 */
struct md2_text_style {
  float font_size_sp;
  float line_height_sp;
  int font_weight;
  float letter_spacing_sp;
  int all_caps;
};

/**
 * @brief Retrieves text style metrics for a given Material Design 2 typography
 * token.
 *
 * @param style Typography style token.
 * @param out_style Pointer receiving style metrics.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_typography_get_style(
    enum md2_typography_style style, struct md2_text_style *out_style);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_TYPOGRAPHY_H */
