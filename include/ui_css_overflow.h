#ifndef UI_CSS_OVERFLOW_H
#define UI_CSS_OVERFLOW_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief CSS overflow property values.
 */
enum ui_css_overflow {
  UI_CSS_OVERFLOW_VISIBLE,
  UI_CSS_OVERFLOW_HIDDEN,
  UI_CSS_OVERFLOW_CLIP,
  UI_CSS_OVERFLOW_SCROLL,
  UI_CSS_OVERFLOW_AUTO
};

/**
 * @brief CSS text-overflow and block-ellipsis type.
 */
enum ui_css_text_overflow_type {
  UI_CSS_TEXT_OVERFLOW_CLIP,
  UI_CSS_TEXT_OVERFLOW_ELLIPSIS,
  UI_CSS_TEXT_OVERFLOW_STRING
};

/**
 * @brief CSS text-overflow property.
 */
struct ui_css_text_overflow {
  enum ui_css_text_overflow_type type;
  char string[32]; /* Used if type == UI_CSS_TEXT_OVERFLOW_STRING */
};

/**
 * @brief CSS block-ellipsis property (Level 4).
 */
struct ui_css_block_ellipsis {
  enum ui_css_text_overflow_type type; /* none maps to CLIP, auto to ELLIPSIS */
  char string[32];
};

/**
 * @brief CSS line-clamp property (Level 4).
 */
struct ui_css_line_clamp {
  int is_none;
  int lines;
};

/**
 * @brief CSS max-lines property.
 */
struct ui_css_max_lines {
  int is_none;
  int lines;
};

/**
 * @brief Parses CSS overflow (x or y).
 *
 * @param str The string to parse.
 * @param out_overflow Pointer to receive the parsed overflow value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_overflow(const char *str,
                                    enum ui_css_overflow *out_overflow);

/**
 * @brief Parses CSS text-overflow.
 *
 * @param str The string to parse.
 * @param out_overflow Pointer to receive the parsed text-overflow value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_text_overflow(const char *str,
                           struct ui_css_text_overflow *out_overflow);

/**
 * @brief Parses CSS block-ellipsis.
 *
 * @param str The string to parse.
 * @param out_ellipsis Pointer to receive the parsed block-ellipsis value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_block_ellipsis(const char *str,
                            struct ui_css_block_ellipsis *out_ellipsis);

/**
 * @brief Parses CSS line-clamp.
 *
 * @param str The string to parse.
 * @param out_clamp Pointer to receive the parsed line-clamp value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_line_clamp(const char *str,
                                      struct ui_css_line_clamp *out_clamp);

/**
 * @brief Parses CSS max-lines.
 *
 * @param str The string to parse.
 * @param out_max_lines Pointer to receive the parsed max-lines value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_max_lines(const char *str,
                                     struct ui_css_max_lines *out_max_lines);

/**
 * @brief Parses CSS overflow-clip-margin.
 *
 * @param str The string to parse.
 * @param out_margin Pointer to receive the parsed overflow-clip-margin value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_overflow_clip_margin(const char *str,
                                  struct ui_css_value *out_margin);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_OVERFLOW_H */
