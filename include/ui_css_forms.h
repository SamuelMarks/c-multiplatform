/**
 * @file ui_css_forms.h
 */
/**
 * \file ui_css_forms.h
 * \brief CSS Forms properties definitions and parsing.
 * \author UI Framework Team
 * \date 2026
 */

#ifndef UI_CSS_FORMS_H
#define UI_CSS_FORMS_H

/**
 * \defgroup ui_css_forms CSS Forms
 * \brief Types and functions for CSS forms properties like field-sizing,
 * accent-color, and caret-color.
 * \{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * \brief CSS field-sizing values.
 */
enum ui_css_field_sizing {
  UI_CSS_FIELD_SIZING_FIXED,  /**< Fixed field sizing. */
  UI_CSS_FIELD_SIZING_CONTENT /**< Content field sizing. */
};

/**
 * \brief CSS accent-color property.
 */
struct ui_css_accent_color {
  int is_auto;               /**< Non-zero if color is 'auto'. */
  struct ui_css_color color; /**< Accent color value (valid if not 'auto'). */
};

/**
 * \brief CSS caret-color property.
 */
struct ui_css_caret_color {
  int is_auto;               /**< Non-zero if color is 'auto'. */
  struct ui_css_color color; /**< Caret color value (valid if not 'auto'). */
};

/**
 * \brief Parses the field-sizing CSS property.
 *
 * \param str The string to parse.
 * \param out_sizing Pointer to receive the parsed field-sizing value.
 * \return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT or
 * UI_ERROR_INVALID_FORMAT on error.
 */
ui_error_t ui_css_parse_field_sizing(const char *str,
                                     enum ui_css_field_sizing *out_sizing);

/**
 * \brief Parses the accent-color CSS property.
 *
 * \param str The string to parse.
 * \param out_color Pointer to receive the parsed accent-color structure.
 * \return UI_ERROR_NONE on success, error code otherwise.
 */
ui_error_t ui_css_parse_accent_color(const char *str,
                                     struct ui_css_accent_color *out_color);

/**
 * \brief Parses the caret-color CSS property.
 *
 * \param str The string to parse.
 * \param out_color Pointer to receive the parsed caret-color structure.
 * \return UI_ERROR_NONE on success, error code otherwise.
 */
ui_error_t ui_css_parse_caret_color(const char *str,
                                    struct ui_css_caret_color *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** \} */

#endif /* UI_CSS_FORMS_H */
