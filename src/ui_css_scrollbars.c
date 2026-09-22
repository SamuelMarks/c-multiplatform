/**
 * @file ui_css_scrollbars.c
 * @brief ui_css_scrollbars.c implementation.
 */
/* clang-format off */
#include "ui_css_scrollbars.h"
#include <string.h>
#include <ctype.h>
#include "ui_internal_mem.h"
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_scrollbars_mock_fail = 0;
static int g_scrollbars_sw_counter = 0;
#endif

/**
 * @brief skip_whitespace.
 * @param p_str Parameter p_str.
 * @return Return value.
 */
static ui_error_t skip_whitespace(const char **p_str) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_scrollbars_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_scrollbars_mock_fail == 2) {
    g_scrollbars_sw_counter++;
    if (g_scrollbars_sw_counter == 2) {
      return UI_ERROR_UNKNOWN;
    }
  }
#endif
  while (**p_str && isspace((unsigned char)**p_str)) {
    (*p_str)++;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Parses scrollbar width value.
 * @param str Parameter str.
 * @param out_width Parameter out_width.
 * @return Return value.
 */
ui_error_t
ui_css_parse_scrollbar_width(const char *str,
                             enum ui_css_scrollbar_width *out_width) {
  ui_error_t rc;

  if (!str || !out_width)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (strcmp(str, "auto") == 0) {
    *out_width = UI_CSS_SCROLLBAR_WIDTH_AUTO;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "thin") == 0) {
    *out_width = UI_CSS_SCROLLBAR_WIDTH_THIN;
    return UI_ERROR_NONE;
  } else if (strcmp(str, "none") == 0) {
    *out_width = UI_CSS_SCROLLBAR_WIDTH_NONE;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_PARSE_FAILED;
}

/**
 * @brief Parses scrollbar color value.
 * @param str Parameter str.
 * @param out_color Parameter out_color.
 * @return Return value.
 */
ui_error_t
ui_css_parse_scrollbar_color(const char *str,
                             struct ui_css_scrollbar_color *out_color) {
  char token_buf[256];
  const char *space;
  size_t len;
  ui_error_t rc;

  if (!str || !out_color)
    return UI_ERROR_INVALID_ARGUMENT;

#ifdef UI_TEST_MOCK_ALLOC
  g_scrollbars_sw_counter = 0;
#endif

  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (strcmp(str, "auto") == 0) {
    out_color->is_auto = 1;
    return UI_ERROR_NONE;
  }

  out_color->is_auto = 0;

  /* Parse thumb color */
  space = str;
  while (*space && !isspace((unsigned char)*space)) {
    space++;
  }

  len = (size_t)(space - str);
  if (len >= sizeof(token_buf))
    len = sizeof(token_buf) - 1;
  memcpy(token_buf, str, len);
  token_buf[len] = '\0';

  rc = ui_css_parse_color(token_buf, &out_color->thumb_color);
  if (rc != UI_ERROR_NONE)
    return rc;

  str = space;
  rc = skip_whitespace(&str);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (*str == '\0') {
    /* CSS scrollbar-color requires exactly two valid colors or auto.
       If there's only one, it's a syntax error. */
    return UI_ERROR_PARSE_FAILED;
  }

  /* Parse track color */
  rc = ui_css_parse_color(str, &out_color->track_color);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}
