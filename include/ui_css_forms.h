#ifndef UI_CSS_FORMS_H
#define UI_CSS_FORMS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief CSS field-sizing values.
 */
enum ui_css_field_sizing {
  UI_CSS_FIELD_SIZING_FIXED,
  UI_CSS_FIELD_SIZING_CONTENT
};

/**
 * @brief CSS accent-color property.
 */
struct ui_css_accent_color {
  int is_auto;
  struct ui_css_color color;
};

/**
 * @brief CSS caret-color property.
 */
struct ui_css_caret_color {
  int is_auto;
  struct ui_css_color color;
};

/**
 * @brief Parses the field-sizing CSS property.
 *
 * @param str The string to parse.
 * @param out_sizing Pointer to receive the parsed field-sizing value.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT or
 * UI_ERROR_INVALID_FORMAT on error.
 */
enum ui_error ui_css_parse_field_sizing(const char *str,
                                        enum ui_css_field_sizing *out_sizing);

/**
 * @brief Parses the accent-color CSS property.
 *
 * @param str The string to parse.
 * @param out_color Pointer to receive the parsed accent-color structure.
 * @return UI_ERROR_NONE on success, error code otherwise.
 */
enum ui_error ui_css_parse_accent_color(const char *str,
                                        struct ui_css_accent_color *out_color);

/**
 * @brief Parses the caret-color CSS property.
 *
 * @param str The string to parse.
 * @param out_color Pointer to receive the parsed caret-color structure.
 * @return UI_ERROR_NONE on success, error code otherwise.
 */
enum ui_error ui_css_parse_caret_color(const char *str,
                                       struct ui_css_caret_color *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_FORMS_H */
