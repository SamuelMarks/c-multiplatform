#ifndef UI_CSS_SCROLLBARS_H
#define UI_CSS_SCROLLBARS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief Represents the css scrollbar-width property.
 */
enum ui_css_scrollbar_width {
  UI_CSS_SCROLLBAR_WIDTH_AUTO,
  UI_CSS_SCROLLBAR_WIDTH_THIN,
  UI_CSS_SCROLLBAR_WIDTH_NONE
};

/**
 * @brief Represents the css scrollbar-color property.
 *
 * If is_auto is 1, thumb_color and track_color are ignored.
 */
struct ui_css_scrollbar_color {
  int is_auto;
  struct ui_css_color thumb_color;
  struct ui_css_color track_color;
};

/**
 * @brief Parses the CSS 'scrollbar-width' property.
 *
 * @param str The string to parse.
 * @param out_width Pointer to receive the parsed width.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_scrollbar_width(const char *str,
                             enum ui_css_scrollbar_width *out_width);

/**
 * @brief Parses the CSS 'scrollbar-color' property.
 *
 * @param str The string to parse.
 * @param out_color Pointer to receive the parsed color config.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_scrollbar_color(const char *str,
                             struct ui_css_scrollbar_color *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SCROLLBARS_H */
