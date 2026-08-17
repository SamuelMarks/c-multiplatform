/**
 * @file ui_css_spatial_nav.h
 * @brief CSS Spatial Navigation properties and parsing.
 *
 * This header defines structures, enumerations, and functions for handling
 * CSS spatial navigation properties.
 */

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
  UI_CSS_SPATIAL_NAV_ACTION_AUTO,  /**< Automatic spatial navigation action. */
  UI_CSS_SPATIAL_NAV_ACTION_FOCUS, /**< Focus navigation action. */
  UI_CSS_SPATIAL_NAV_ACTION_SCROLL /**< Scroll navigation action. */
};

/**
 * @brief Containment behavior for spatial navigation.
 */
enum ui_css_spatial_nav_contain {
  UI_CSS_SPATIAL_NAV_CONTAIN_AUTO,   /**< Automatic containment. */
  UI_CSS_SPATIAL_NAV_CONTAIN_CONTAIN /**< Strict containment behavior. */
};

/**
 * @brief Functionality modifier for spatial navigation.
 */
enum ui_css_spatial_nav_function {
  UI_CSS_SPATIAL_NAV_FUNCTION_NORMAL, /**< Normal spatial navigation function.
                                       */
  UI_CSS_SPATIAL_NAV_FUNCTION_GRID /**< Grid-based spatial navigation function.
                                    */
};

/**
 * @brief Spatial navigation properties for a layout node.
 */
struct ui_css_spatial_nav_properties {
  enum ui_css_spatial_nav_action action; /**< The spatial navigation action. */
  enum ui_css_spatial_nav_contain contain; /**< The containment rule. */
  enum ui_css_spatial_nav_function
      function; /**< The spatial navigation function. */
};

/**
 * @brief Forward declaration of the computed style structure.
 */
struct ui_css_computed_style;

/**
 * @brief Parses the CSS spatial navigation properties from a computed style.
 *
 * @param style Pointer to the computed style.
 * @param out_props Pointer to the structure to populate with parsed properties.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t
ui_css_spatial_nav_parse(const struct ui_css_computed_style *style,
                         struct ui_css_spatial_nav_properties *out_props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_SPATIAL_NAV_H */
