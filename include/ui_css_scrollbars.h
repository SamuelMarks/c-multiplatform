/**
 * @file ui_css_scrollbars.h
 * @brief CSS Scrollbars properties and parsing.
 *
 * This header defines structures, enumerations, and functions for handling
 * CSS scrollbar styling properties.
 */

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
  UI_CSS_SCROLLBAR_WIDTH_AUTO, /**< Automatic scrollbar width. */
  UI_CSS_SCROLLBAR_WIDTH_THIN, /**< Thin scrollbar width. */
  UI_CSS_SCROLLBAR_WIDTH_NONE  /**< No scrollbar width (hidden). */
};

/**
 * @brief Represents the css scrollbar-color property.
 *
 * If is_auto is 1, thumb_color and track_color are ignored.
 */
struct ui_css_scrollbar_color {
  int is_auto; /**< 1 if color is set to 'auto', 0 otherwise. */
  struct ui_css_color thumb_color; /**< Color of the scrollbar thumb. */
  struct ui_css_color track_color; /**< Color of the scrollbar track. */
};

/**
 * @brief Parses the CSS 'scrollbar-width' property.
 *
 * @param str The string to parse.
 * @param out_width Pointer to receive the parsed width.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_parse_scrollbar_width(const char *str,
                                        enum ui_css_scrollbar_width *out_width);

/**
 * @brief Parses the CSS 'scrollbar-color' property.
 *
 * @param str The string to parse.
 * @param out_color Pointer to receive the parsed color config.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t
ui_css_parse_scrollbar_color(const char *str,
                             struct ui_css_scrollbar_color *out_color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SCROLLBARS_H */
