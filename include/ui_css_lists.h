/**
 * @file ui_css_lists.h
 */
/**
 * \file ui_css_lists.h
 * \brief CSS Lists and Counters definitions.
 * \author UI Framework Team
 * \date 2026
 */

#ifndef UI_CSS_LISTS_H
#define UI_CSS_LISTS_H

/**
 * \defgroup ui_css_lists CSS Lists and Counters
 * \brief Types and functions for parsing and representing CSS lists and
 * counters.
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
 * \brief CSS list-style-type property values.
 */
enum ui_css_list_style_type {
  UI_CSS_LIST_STYLE_TYPE_DISC,                 /**< Disc style. */
  UI_CSS_LIST_STYLE_TYPE_CIRCLE,               /**< Circle style. */
  UI_CSS_LIST_STYLE_TYPE_SQUARE,               /**< Square style. */
  UI_CSS_LIST_STYLE_TYPE_DECIMAL,              /**< Decimal style. */
  UI_CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO, /**< Decimal with leading zero
                                                  style. */
  UI_CSS_LIST_STYLE_TYPE_LOWER_ROMAN,          /**< Lower Roman style. */
  UI_CSS_LIST_STYLE_TYPE_UPPER_ROMAN,          /**< Upper Roman style. */
  UI_CSS_LIST_STYLE_TYPE_LOWER_GREEK,          /**< Lower Greek style. */
  UI_CSS_LIST_STYLE_TYPE_LOWER_ALPHA,          /**< Lower Alpha style. */
  UI_CSS_LIST_STYLE_TYPE_LOWER_LATIN,          /**< Lower Latin style. */
  UI_CSS_LIST_STYLE_TYPE_UPPER_ALPHA,          /**< Upper Alpha style. */
  UI_CSS_LIST_STYLE_TYPE_UPPER_LATIN,          /**< Upper Latin style. */
  UI_CSS_LIST_STYLE_TYPE_ARMENIAN,             /**< Armenian style. */
  UI_CSS_LIST_STYLE_TYPE_GEORGIAN,             /**< Georgian style. */
  UI_CSS_LIST_STYLE_TYPE_HEBREW,               /**< Hebrew style. */
  UI_CSS_LIST_STYLE_TYPE_NONE,                 /**< None style. */
  UI_CSS_LIST_STYLE_TYPE_STRING /**< Custom string style (e.g. "->"). */
};

/**
 * \brief CSS list-style-type property extending string value.
 */
struct ui_css_list_style_type_ext {
  enum ui_css_list_style_type type; /**< List style type. */
  char string_value[32];            /**< String value used if type ==
                                       UI_CSS_LIST_STYLE_TYPE_STRING. */
};

/**
 * \brief CSS list-style-position property values.
 */
enum ui_css_list_style_position {
  UI_CSS_LIST_STYLE_POSITION_OUTSIDE, /**< Outside list style position. */
  UI_CSS_LIST_STYLE_POSITION_INSIDE   /**< Inside list style position. */
};

/**
 * \brief CSS list-style shorthand.
 */
struct ui_css_list_style {
  struct ui_css_list_style_type_ext type;   /**< The list style type. */
  enum ui_css_list_style_position position; /**< The list style position. */
  struct ui_css_image image;                /**< The list style image. */
  int has_image;                            /**< Non-zero if list has image. */
};

/**
 * \brief Represents a single counter action (reset, increment, set).
 */
struct ui_css_counter_action {
  char name[64];                      /**< Counter name. */
  int value;                          /**< Counter action value. */
  struct ui_css_counter_action *next; /**< Pointer to the next action. */
};

/**
 * \brief Parses CSS list-style-type.
 *
 * \param str The string to parse.
 * \param out_type Pointer to receive the parsed type.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_list_style_type(const char *str,
                             struct ui_css_list_style_type_ext *out_type);

/**
 * \brief Parses CSS list-style-position.
 *
 * \param str The string to parse.
 * \param out_position Pointer to receive the parsed position.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_list_style_position(const char *str,
                                 enum ui_css_list_style_position *out_position);

/**
 * \brief Parses CSS list-style-image.
 *
 * \param str The string to parse.
 * \param out_image Pointer to receive the parsed image.
 * \param out_is_none Pointer to receive whether the value was 'none'.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_list_style_image(const char *str,
                                         struct ui_css_image *out_image,
                                         int *out_is_none);

/**
 * \brief Parses CSS list-style shorthand.
 *
 * \param str The string to parse.
 * \param out_style Pointer to receive the parsed shorthand.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_list_style(const char *str,
                                   struct ui_css_list_style *out_style);

/**
 * \brief Parses CSS counter-reset, counter-increment, counter-set.
 * \note caller is responsible for freeing the returned linked list.
 *
 * \param str The string to parse.
 * \param out_actions Pointer to receive the parsed actions linked list.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_counter_action(const char *str,
                            struct ui_css_counter_action **out_actions);

/**
 * \brief Frees a list of counter actions.
 *
 * \param actions The list to free.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_counter_action_destroy(struct ui_css_counter_action *actions);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** \} */

#endif /* UI_CSS_LISTS_H */
