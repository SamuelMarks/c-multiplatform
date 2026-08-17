/**
 * \file ui_css_highlight_api.h
 * \brief CSS Custom Highlight API definitions.
 * \author UI Framework Team
 * \date 2026
 */

#ifndef UI_CSS_HIGHLIGHT_API_H
#define UI_CSS_HIGHLIGHT_API_H

/**
 * \defgroup ui_css_highlight CSS Highlight API
 * \brief Types and functions for CSS Custom Highlight API properties.
 * \{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * \brief Represents the predefined types of CSS custom highlights.
 */
enum ui_css_highlight_type {
  UI_CSS_HIGHLIGHT_TYPE_SELECTION,      /**< Selection highlight. */
  UI_CSS_HIGHLIGHT_TYPE_TARGET_TEXT,    /**< Target text highlight. */
  UI_CSS_HIGHLIGHT_TYPE_SPELLING_ERROR, /**< Spelling error highlight. */
  UI_CSS_HIGHLIGHT_TYPE_GRAMMAR_ERROR,  /**< Grammar error highlight. */
  UI_CSS_HIGHLIGHT_TYPE_CUSTOM          /**< Custom highlight. */
};

/**
 * \brief Represents a text range for a CSS highlight.
 */
struct ui_css_highlight_range {
  size_t start_offset; /**< The start offset of the range. */
  size_t end_offset;   /**< The end offset of the range. */
};

/**
 * \brief Opaque structure representing a CSS custom highlight context.
 */
struct ui_css_highlight;

/**
 * \brief Creates a CSS custom highlight context.
 *
 * \param type The type of highlight.
 * \param custom_name The custom name if type is CUSTOM (can be NULL otherwise).
 * \param out_highlight Pointer to receive the created highlight.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_highlight_create(enum ui_css_highlight_type type,
                                   const char *custom_name,
                                   struct ui_css_highlight **out_highlight);

/**
 * \brief Adds a text range to the highlight.
 *
 * \param highlight The highlight.
 * \param start_offset Start offset in the text.
 * \param end_offset End offset in the text.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_highlight_add_range(struct ui_css_highlight *highlight,
                                      size_t start_offset, size_t end_offset);

/**
 * \brief Destroys a CSS custom highlight context.
 *
 * \param highlight The highlight to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_css_highlight_destroy(struct ui_css_highlight *highlight);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** \} */

#endif /* UI_CSS_HIGHLIGHT_API_H */
