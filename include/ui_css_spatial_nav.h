#ifndef UI_CSS_SPATIAL_NAV_H
#define UI_CSS_SPATIAL_NAV_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Action for spatial navigation.
 */
enum ui_css_spatial_nav_action {
  UI_CSS_SPATIAL_NAV_ACTION_AUTO,
  UI_CSS_SPATIAL_NAV_ACTION_FOCUS,
  UI_CSS_SPATIAL_NAV_ACTION_SCROLL
};

/**
 * @brief Containment behavior for spatial navigation.
 */
enum ui_css_spatial_nav_contain {
  UI_CSS_SPATIAL_NAV_CONTAIN_AUTO,
  UI_CSS_SPATIAL_NAV_CONTAIN_CONTAIN
};

/**
 * @brief Functionality modifier for spatial navigation.
 */
enum ui_css_spatial_nav_function {
  UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL,
  UI_CSS_SPATIAL_NAV_FUNCTION_GRID
};

/**
 * @brief Spatial navigation properties for a layout node.
 */
struct ui_css_spatial_nav_properties {
  enum ui_css_spatial_nav_action action;
  enum ui_css_spatial_nav_contain contain;
  enum ui_css_spatial_nav_function function;
};

struct ui_css_computed_style; /* Forward declare */

/**
 * @brief Parses the CSS spatial navigation properties from a computed style.
 *
 * @param style The computed style.
 * @param out_props Pointer to the struct to populate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_spatial_nav_parse(const struct ui_css_computed_style *style,
                         struct ui_css_spatial_nav_properties *out_props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SPATIAL_NAV_H */
