/**
 * @file md3_typography.h
 * @brief Material 3 baseline (15-style) and Expressive (30-style) typography
 * scales.
 */

#ifndef MATERIAL3_MD3_TYPOGRAPHY_H
#define MATERIAL3_MD3_TYPOGRAPHY_H

/* clang-format off */
#include "ui_design_tokens.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum md3_typescale_role
 * @brief Typography scale roles in Material 3.
 */
enum md3_typescale_role {
  MD3_TYPESCALE_DISPLAY_LARGE = 0,
  MD3_TYPESCALE_DISPLAY_MEDIUM,
  MD3_TYPESCALE_DISPLAY_SMALL,
  MD3_TYPESCALE_HEADLINE_LARGE,
  MD3_TYPESCALE_HEADLINE_MEDIUM,
  MD3_TYPESCALE_HEADLINE_SMALL,
  MD3_TYPESCALE_TITLE_LARGE,
  MD3_TYPESCALE_TITLE_MEDIUM,
  MD3_TYPESCALE_TITLE_SMALL,
  MD3_TYPESCALE_BODY_LARGE,
  MD3_TYPESCALE_BODY_MEDIUM,
  MD3_TYPESCALE_BODY_SMALL,
  MD3_TYPESCALE_LABEL_LARGE,
  MD3_TYPESCALE_LABEL_MEDIUM,
  MD3_TYPESCALE_LABEL_SMALL,
  MD3_TYPESCALE_ROLE_COUNT
};

/**
 * @struct md3_type_style
 * @brief Typographic metrics for a type style.
 */
struct md3_type_style {
  float size_sp;        /**< Font size in sp */
  float line_height_sp; /**< Line height in sp */
  float tracking_sp;    /**< Letter spacing in sp */
  int weight;           /**< Font weight (e.g. 400, 500, 700, 800) */
};

/**
 * @brief Retrieves typography metrics for a role, with optional expressive
 * emphasis.
 *
 * @param role Typography scale role.
 * @param is_emphasized Non-zero for Material 3 Expressive emphasized weight.
 * @param out_style Pointer to receive typographic metrics.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT on bad
 * role/pointer.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_typography_get_style(enum md3_typescale_role role, int is_emphasized,
                         struct md3_type_style *out_style);

/**
 * @brief Injects typography tokens into a design token dictionary.
 *
 * @param is_expressive Non-zero to inject both baseline and emphasized
 * expressive tokens.
 * @param dict Pointer to the design token dictionary.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_typography_apply_tokens(
    int is_expressive, struct ui_design_token_dict *dict);

struct ui_font;

/**
 * @brief Applies Material 3 typographic variable font axis modulation
 * (weight 'wght', width 'wdth', optical size 'opsz') to a font instance.
 *
 * @param font Pointer to the ui_font instance.
 * @param role Typography scale role.
 * @param is_emphasized Non-zero for expressive emphasized font styling.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_typography_apply_font_variations(
    struct ui_font *font, enum md3_typescale_role role, int is_emphasized);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_TYPOGRAPHY_H */
