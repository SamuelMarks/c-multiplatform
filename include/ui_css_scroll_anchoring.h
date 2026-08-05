#ifndef UI_CSS_SCROLL_ANCHORING_H
#define UI_CSS_SCROLL_ANCHORING_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief CSS overflow-anchor property.
 */
enum ui_css_overflow_anchor {
  UI_CSS_OVERFLOW_ANCHOR_AUTO,
  UI_CSS_OVERFLOW_ANCHOR_NONE
};

struct ui_css_computed_style; /* Forward declare */

/**
 * @brief Parses the CSS overflow-anchor property from a computed style.
 *
 * @param style The computed style.
 * @param out_anchor Pointer to the enum to populate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_scroll_anchoring_parse(const struct ui_css_computed_style *style,
                              enum ui_css_overflow_anchor *out_anchor);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SCROLL_ANCHORING_H */
