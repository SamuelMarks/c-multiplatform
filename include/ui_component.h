#ifndef UI_COMPONENT_H
#define UI_COMPONENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_cssom.h"
/* clang-format on */

/**
 * @brief Represents an isolated UI component (similar to a Web Component with
 * Shadow DOM).
 */
struct ui_component {
  struct ui_dom_node *shadow_root; /**< The isolated DOM subtree root. */
  struct ui_css_stylesheet
      *internal_style; /**< The isolated internal stylesheet. */
  struct ui_css_stylesheet
      *override_style; /**< User-provided style overrides. */
  struct ui_css_stylesheet
      *bound_properties; /**< Dynamically bound CSS Custom Properties. */
  struct ui_dom_node
      *host_node; /**< The DOM node this component is mounted to. */
  char *scope_id; /**< A unique ID (e.g., "a7f2") to scope CSS
                     classes/attributes. */
};

/**
 * @brief Creates a new opaque component instance.
 *
 * @param out_component Pointer to receive the new component instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_component_create(struct ui_component **out_component);

/**
 * @brief Destroys a component and its isolated DOM/style subtrees.
 *
 * @param component The component to destroy.
 */
ui_error_t ui_component_destroy(struct ui_component *component);

/**
 * @brief Sets the default internal stylesheet for the component.
 *
 * @param component The component.
 * @param stylesheet The stylesheet to apply as the internal default.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_component_set_default_style(struct ui_component *component,
                                          struct ui_css_stylesheet *stylesheet);

/**
 * @brief Injects a user-provided style override via a CSS string payload.
 *
 * @param component The component.
 * @param css_string The CSS string containing overrides.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_component_inject_style_override(struct ui_component *component,
                                              const char *css_string);

/**
 * @brief Maps internal component state to CSS Custom Properties (Variables).
 *
 * @param component The component.
 * @param property_name The name of the CSS custom property (e.g.,
 * "--primary-color").
 * @param property_value The value to assign to the property.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_component_set_property(struct ui_component *component,
                                     const char *property_name,
                                     const char *property_value);

/**
 * @brief Enforces Scoped CSS Encapsulation.
 * Generates/assigns a unique scope ID to the component, updates the shadow DOM
 * nodes, and rewrites the internal and override stylesheets to prevent global
 * OM pollution.
 *
 * @param component The component to scope.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_component_scope_styles(struct ui_component *component);

/**
 * @brief Mounts the component to a host DOM node.
 *
 * @param component The component to mount.
 * @param host_node The DOM node that will act as the host/mount point.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_component_mount(struct ui_component *component,
                              struct ui_dom_node *host_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COMPONENT_H */
