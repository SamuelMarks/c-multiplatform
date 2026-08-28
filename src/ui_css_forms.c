/**
 * @file ui_css_forms.c
 * @brief ui_css_forms.c implementation.
 */
/* clang-format off */
#include "ui_css_forms.h"
#include <string.h>
/* clang-format on */

/**
 * @brief ui_css_parse_field_sizing.
 * @param str Parameter str.
 * @param out_sizing Parameter out_sizing.
 * @return Return value.
 */
ui_error_t ui_css_parse_field_sizing(const char *str,
                                     enum ui_css_field_sizing *out_sizing) {
  if (!str || !out_sizing) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(str, "content") == 0) {
    *out_sizing = UI_CSS_FIELD_SIZING_CONTENT;
  } else if (strcmp(str, "fixed") == 0) {
    *out_sizing = UI_CSS_FIELD_SIZING_FIXED;
  } else {
    return UI_ERROR_PARSE_FAILED;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_css_parse_accent_color.
 * @param str Parameter str.
 * @param out_color Parameter out_color.
 * @return Return value.
 */
ui_error_t ui_css_parse_accent_color(const char *str,
                                     struct ui_css_accent_color *out_color) {
  ui_error_t rc;

  if (!str || !out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(str, "auto") == 0) {
    out_color->is_auto = 1;
    return UI_ERROR_NONE;
  }

  out_color->is_auto = 0;
  rc = ui_css_parse_color(str, &out_color->color);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_css_parse_caret_color.
 * @param str Parameter str.
 * @param out_color Parameter out_color.
 * @return Return value.
 */
ui_error_t ui_css_parse_caret_color(const char *str,
                                    struct ui_css_caret_color *out_color) {
  ui_error_t rc;

  if (!str || !out_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(str, "auto") == 0) {
    out_color->is_auto = 1;
    return UI_ERROR_NONE;
  }

  out_color->is_auto = 0;
  rc = ui_css_parse_color(str, &out_color->color);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}
