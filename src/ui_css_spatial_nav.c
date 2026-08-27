/* clang-format off */
#include "../include/ui_css_spatial_nav.h"
#include "../include/ui_cssom.h"
#include <string.h>
#include <stddef.h>
/* clang-format on */

/*
 * @brief ui_css_spatial_nav_parse.
 * @param style Parameter style.
 * @param out_props Parameter out_props.
 * @return Return value.
 */
ui_error_t
ui_css_spatial_nav_parse(const struct ui_css_computed_style *style,
                         struct ui_css_spatial_nav_properties *out_props) {
  const char *val = NULL;
  ui_error_t rc;

  if (!style || !out_props) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_props->action = UI_CSS_SPATIAL_NAV_ACTION_AUTO;
  out_props->contain = UI_CSS_SPATIAL_NAV_CONTAIN_AUTO;
  out_props->function = UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL;

  rc = ui_css_computed_style_get_property(style, "spatial-navigation-action",
                                          &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "focus") == 0) {
      out_props->action = UI_CSS_SPATIAL_NAV_ACTION_FOCUS;
    } else if (strcmp(val, "scroll") == 0) {
      out_props->action = UI_CSS_SPATIAL_NAV_ACTION_SCROLL;
    } else if (strcmp(val, "auto") == 0) {
      out_props->action = UI_CSS_SPATIAL_NAV_ACTION_AUTO;
    }
  }

  rc = ui_css_computed_style_get_property(style, "spatial-navigation-contain",
                                          &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "contain") == 0) {
      out_props->contain = UI_CSS_SPATIAL_NAV_CONTAIN_CONTAIN;
    } else if (strcmp(val, "auto") == 0) {
      out_props->contain = UI_CSS_SPATIAL_NAV_CONTAIN_AUTO;
    }
  }

  rc = ui_css_computed_style_get_property(style, "spatial-navigation-function",
                                          &val);
  if (rc != UI_ERROR_NONE) {
    if (0)
      return rc;
  }
  if (rc == UI_ERROR_NONE) {
    if (strcmp(val, "grid") == 0) {
      out_props->function = UI_CSS_SPATIAL_NAV_FUNCTION_GRID;
    } else if (strcmp(val, "normal") == 0) {
      out_props->function = UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL;
    }
  }

  return UI_ERROR_NONE;
}
