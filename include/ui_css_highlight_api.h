#ifndef UI_CSS_HIGHLIGHT_API_H
#define UI_CSS_HIGHLIGHT_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents the predefined types of CSS custom highlights.
 */
enum ui_css_highlight_type {
  UI_CSS_HIGHLIGHT_TYPE_SELECTION,
  UI_CSS_HIGHLIGHT_TYPE_TARGET_TEXT,
  UI_CSS_HIGHLIGHT_TYPE_SPELLING_ERROR,
  UI_CSS_HIGHLIGHT_TYPE_GRAMMAR_ERROR,
  UI_CSS_HIGHLIGHT_TYPE_CUSTOM
};

/**
 * @brief Represents a text range for a CSS highlight.
 */
struct ui_css_highlight_range {
  size_t start_offset;
  size_t end_offset;
};

/**
 * @brief Opaque structure representing a CSS custom highlight context.
 */
struct ui_css_highlight;

/**
 * @brief Creates a CSS custom highlight context.
 *
 * @param type The type of highlight.
 * @param custom_name The custom name if type is CUSTOM (can be NULL otherwise).
 * @param out_highlight Pointer to the created highlight.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_highlight_create(enum ui_css_highlight_type type,
                                   const char *custom_name,
                                   struct ui_css_highlight **out_highlight);

/**
 * @brief Adds a text range to the highlight.
 *
 * @param highlight The highlight.
 * @param start_offset Start offset in the text.
 * @param end_offset End offset in the text.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_highlight_add_range(struct ui_css_highlight *highlight,
                                      size_t start_offset, size_t end_offset);

/**
 * @brief Destroys a CSS custom highlight context.
 *
 * @param highlight The highlight to destroy.
 */
ui_error_t ui_css_highlight_destroy(struct ui_css_highlight *highlight);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_HIGHLIGHT_API_H */
