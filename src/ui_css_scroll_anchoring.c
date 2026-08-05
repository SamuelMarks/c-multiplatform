/* clang-format off */
#include "../include/ui_css_scroll_anchoring.h"
#include "../include/ui_cssom.h"
#include <string.h>
#include <stddef.h>
/* clang-format on */

ui_error_t
ui_css_scroll_anchoring_parse(const struct ui_css_computed_style *style,
                              enum ui_css_overflow_anchor *out_anchor) {
  const char *val = NULL;
  ui_error_t rc;

  if (!style || !out_anchor) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_anchor = UI_CSS_OVERFLOW_ANCHOR_AUTO; /* Default */

  rc = ui_css_computed_style_get_property(style, "overflow-anchor", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (val[0] == 'n') {
      *out_anchor = UI_CSS_OVERFLOW_ANCHOR_NONE;
    } else if (val[0] == 'a') {
      *out_anchor = UI_CSS_OVERFLOW_ANCHOR_AUTO;
    }
  }

  return UI_ERROR_NONE;
}
