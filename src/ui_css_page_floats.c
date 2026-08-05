/* clang-format off */
#include "../include/ui_css_page_floats.h"
#include "../include/ui_cssom.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

ui_error_t
ui_css_page_floats_parse(const struct ui_css_computed_style *style,
                         struct ui_css_page_floats_properties *out_props) {
  const char *val = NULL;
  ui_error_t rc;

  if (!style || !out_props) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_props->float_reference = UI_CSS_FLOAT_REFERENCE_INLINE;
  out_props->float_defer_type = UI_CSS_FLOAT_DEFER_NONE;
  out_props->float_defer_value = 0;
  out_props->clear_after = UI_CSS_CLEAR_AFTER_NONE;

  rc = ui_css_computed_style_get_property(style, "float-reference", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "column") == 0)
      out_props->float_reference = UI_CSS_FLOAT_REFERENCE_COLUMN;
    else if (strcmp(val, "region") == 0)
      out_props->float_reference = UI_CSS_FLOAT_REFERENCE_REGION;
    else if (strcmp(val, "page") == 0)
      out_props->float_reference = UI_CSS_FLOAT_REFERENCE_PAGE;
    else
      out_props->float_reference = UI_CSS_FLOAT_REFERENCE_INLINE;
  }

  rc = ui_css_computed_style_get_property(style, "float-defer", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "none") == 0) {
      out_props->float_defer_type = UI_CSS_FLOAT_DEFER_NONE;
    } else {
      out_props->float_defer_type = UI_CSS_FLOAT_DEFER_INTEGER;
      out_props->float_defer_value = atoi(val);
    }
  }

  rc = ui_css_computed_style_get_property(style, "clear-after", &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "left") == 0)
      out_props->clear_after = UI_CSS_CLEAR_AFTER_LEFT;
    else if (strcmp(val, "right") == 0)
      out_props->clear_after = UI_CSS_CLEAR_AFTER_RIGHT;
    else if (strcmp(val, "both") == 0)
      out_props->clear_after = UI_CSS_CLEAR_AFTER_BOTH;
    else if (strcmp(val, "start") == 0)
      out_props->clear_after = UI_CSS_CLEAR_AFTER_START;
    else if (strcmp(val, "end") == 0)
      out_props->clear_after = UI_CSS_CLEAR_AFTER_END;
    else if (strcmp(val, "descendants") == 0)
      out_props->clear_after = UI_CSS_CLEAR_AFTER_DESCENDANTS;
    else
      out_props->clear_after = UI_CSS_CLEAR_AFTER_NONE;
  }

  return UI_ERROR_NONE;
}
