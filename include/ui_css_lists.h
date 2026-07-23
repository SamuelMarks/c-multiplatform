#ifndef UI_CSS_LISTS_H
#define UI_CSS_LISTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief CSS list-style-type property values.
 */
enum ui_css_list_style_type {
  UI_CSS_LIST_STYLE_TYPE_DISC,
  UI_CSS_LIST_STYLE_TYPE_CIRCLE,
  UI_CSS_LIST_STYLE_TYPE_SQUARE,
  UI_CSS_LIST_STYLE_TYPE_DECIMAL,
  UI_CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO,
  UI_CSS_LIST_STYLE_TYPE_LOWER_ROMAN,
  UI_CSS_LIST_STYLE_TYPE_UPPER_ROMAN,
  UI_CSS_LIST_STYLE_TYPE_LOWER_GREEK,
  UI_CSS_LIST_STYLE_TYPE_LOWER_ALPHA,
  UI_CSS_LIST_STYLE_TYPE_LOWER_LATIN,
  UI_CSS_LIST_STYLE_TYPE_UPPER_ALPHA,
  UI_CSS_LIST_STYLE_TYPE_UPPER_LATIN,
  UI_CSS_LIST_STYLE_TYPE_ARMENIAN,
  UI_CSS_LIST_STYLE_TYPE_GEORGIAN,
  UI_CSS_LIST_STYLE_TYPE_HEBREW,
  UI_CSS_LIST_STYLE_TYPE_NONE,
  UI_CSS_LIST_STYLE_TYPE_STRING /* for custom markers like "->" */
};

/**
 * @brief CSS list-style-type property.
 */
struct ui_css_list_style_type_ext {
  enum ui_css_list_style_type type;
  char string_value[32]; /* used if type == STRING */
};

/**
 * @brief CSS list-style-position property values.
 */
enum ui_css_list_style_position {
  UI_CSS_LIST_STYLE_POSITION_OUTSIDE,
  UI_CSS_LIST_STYLE_POSITION_INSIDE
};

/**
 * @brief CSS list-style shorthand.
 */
struct ui_css_list_style {
  struct ui_css_list_style_type_ext type;
  enum ui_css_list_style_position position;
  struct ui_css_image image;
  int has_image;
};

/**
 * @brief Represents a single counter action (reset, increment, set).
 */
struct ui_css_counter_action {
  char name[64];
  int value;
  struct ui_css_counter_action *next;
};

/**
 * @brief Parses CSS list-style-type.
 *
 * @param str The string to parse.
 * @param out_type Pointer to receive the parsed type.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_list_style_type(const char *str,
                             struct ui_css_list_style_type_ext *out_type);

/**
 * @brief Parses CSS list-style-position.
 *
 * @param str The string to parse.
 * @param out_position Pointer to receive the parsed position.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_list_style_position(const char *str,
                                 enum ui_css_list_style_position *out_position);

/**
 * @brief Parses CSS list-style-image.
 *
 * @param str The string to parse.
 * @param out_image Pointer to receive the parsed image.
 * @param out_is_none Pointer to receive whether the value was 'none'.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_list_style_image(const char *str,
                                            struct ui_css_image *out_image,
                                            int *out_is_none);

/**
 * @brief Parses CSS list-style shorthand.
 *
 * @param str The string to parse.
 * @param out_style Pointer to receive the parsed shorthand.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_list_style(const char *str,
                                      struct ui_css_list_style *out_style);

/**
 * @brief Parses CSS counter-reset, counter-increment, counter-set.
 * Note: caller is responsible for freeing the returned linked list.
 *
 * @param str The string to parse.
 * @param out_actions Pointer to receive the parsed actions linked list.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_parse_counter_action(const char *str,
                            struct ui_css_counter_action **out_actions);

/**
 * @brief Frees a list of counter actions.
 *
 * @param actions The list to free.
 */
enum ui_error
ui_css_counter_action_destroy(struct ui_css_counter_action *actions);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_LISTS_H */
