/**
 * @file ui_css_contain.h
 */
/**
 * \file ui_css_contain.h
 * \brief CSS Containment definitions and parsing.
 * \author UI Framework Team
 * \date 2026
 */

#ifndef UI_CSS_CONTAIN_H
#define UI_CSS_CONTAIN_H

/**
 * \defgroup ui_css_contain CSS Containment
 * \brief Types and functions for CSS containment properties.
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
 * \brief CSS Containment flags.
 */
enum ui_css_contain_flags {
  UI_CSS_CONTAIN_NONE = 0,            /**< No containment. */
  UI_CSS_CONTAIN_STRICT = 1 << 0,     /**< Strict containment. */
  UI_CSS_CONTAIN_CONTENT = 1 << 1,    /**< Content containment. */
  UI_CSS_CONTAIN_SIZE = 1 << 2,       /**< Size containment. */
  UI_CSS_CONTAIN_LAYOUT = 1 << 3,     /**< Layout containment. */
  UI_CSS_CONTAIN_STYLE = 1 << 4,      /**< Style containment. */
  UI_CSS_CONTAIN_PAINT = 1 << 5,      /**< Paint containment. */
  UI_CSS_CONTAIN_INLINE_SIZE = 1 << 6 /**< Inline size containment. */
};

/**
 * \brief CSS Content Visibility.
 */
enum ui_css_content_visibility {
  UI_CSS_CONTENT_VISIBILITY_VISIBLE, /**< Visible content. */
  UI_CSS_CONTENT_VISIBILITY_AUTO,    /**< Auto content visibility. */
  UI_CSS_CONTENT_VISIBILITY_HIDDEN   /**< Hidden content. */
};

/**
 * \brief Represents a single dimension's intrinsic size.
 */
struct ui_css_contain_intrinsic_dim {
  int has_auto;               /**< 1 if 'auto' keyword was present */
  int is_none;                /**< 1 if 'none' */
  struct ui_css_value length; /**< valid if !is_none */
};

/**
 * \brief CSS contain-intrinsic-size.
 */
struct ui_css_contain_intrinsic_size {
  struct ui_css_contain_intrinsic_dim width;  /**< Intrinsic width. */
  struct ui_css_contain_intrinsic_dim height; /**< Intrinsic height. */
};

/**
 * \brief Parses the CSS 'contain' property.
 *
 * \param str The string to parse.
 * \param out_flags Pointer to receive the parsed flags.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_parse_contain(const char *str, unsigned int *out_flags);

/**
 * \brief Parses the CSS 'content-visibility' property.
 *
 * \param str The string to parse.
 * \param out_visibility Pointer to receive the parsed visibility.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_parse_content_visibility(const char *str,
                                enum ui_css_content_visibility *out_visibility);

/**
 * \brief Parses the CSS 'contain-intrinsic-size' property.
 *
 * \param str The string to parse.
 * \param out_size Pointer to receive the parsed intrinsic size.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_parse_contain_intrinsic_size(
    const char *str, struct ui_css_contain_intrinsic_size *out_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** \} */

#endif /* UI_CSS_CONTAIN_H */
