#ifndef UI_CSSOM_H
#define UI_CSSOM_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Basic types of CSS selectors for the initial engine implementation.
 */
enum ui_css_selector_type {
  UI_CSS_SELECTOR_TYPE_UNIVERSAL,
  UI_CSS_SELECTOR_TYPE_TAG,
  UI_CSS_SELECTOR_TYPE_CLASS,
  UI_CSS_SELECTOR_TYPE_ID,
  UI_CSS_SELECTOR_TYPE_ATTRIBUTE,
  UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,
  UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT
};

/**
 * @brief Operator used for attribute selectors.
 */
enum ui_css_attr_operator {
  UI_CSS_ATTR_OP_NONE = 0, /* [attr] */
  UI_CSS_ATTR_OP_EQUALS,   /* [attr="val"] */
  UI_CSS_ATTR_OP_INCLUDES, /* [attr~="val"] */
  UI_CSS_ATTR_OP_DASH,     /* [attr|="val"] */
  UI_CSS_ATTR_OP_PREFIX,   /* [attr^="val"] */
  UI_CSS_ATTR_OP_SUFFIX,   /* [attr$="val"] */
  UI_CSS_ATTR_OP_SUBSTRING /* [attr*="val"] */
};

/**
 * @brief Represents a simple CSS selector (e.g., * or div or .btn or #main or
 * [attr="val"]).
 */
struct ui_css_selector {
  enum ui_css_selector_type type;
  char *value; /* NULL for UNIVERSAL. For ATTRIBUTE, this is the attr name. */
  char *attr_value;                  /* Only used for ATTRIBUTE */
  enum ui_css_attr_operator attr_op; /* Only used for ATTRIBUTE */
  struct ui_css_selector
      *nested_selector; /* Used for functional pseudo-classes like :is(),
                           :has(), :not(), :where() */
  struct ui_css_selector *next;
};

/**
 * @brief Represents a CSS declaration (property: value).
 */
struct ui_css_declaration {
  char *property_name;
  char *property_value;
  int is_important;
  struct ui_css_declaration *next;
};

/**
 * @brief Represents the type of a CSS rule.
 */
enum ui_css_rule_type {
  UI_CSS_RULE_TYPE_STYLE,
  UI_CSS_RULE_TYPE_MEDIA,
  UI_CSS_RULE_TYPE_LAYER,
  UI_CSS_RULE_TYPE_SCOPE,
  UI_CSS_RULE_TYPE_PROPERTY,
  UI_CSS_RULE_TYPE_SUPPORTS,
  UI_CSS_RULE_TYPE_CONTAINER
};

/**
 * @brief Represents a CSS rule (selectors { declarations }).
 */
struct ui_css_rule {
  enum ui_css_rule_type type;

  /* For STYLE rules */
  struct ui_css_selector *selectors;
  struct ui_css_declaration *declarations;

  /* For MEDIA rules */
  char *media_condition;
  struct ui_css_rule *nested_rules;

  /* For SUPPORTS rules */
  char *supports_condition;

  /* For CONTAINER rules */
  char *container_condition;

  /* For LAYER rules */
  char *layer_name;

  /* For SCOPE rules */
  char *scope_start;
  char *scope_end;

  /* For PROPERTY rules (@property) */
  char *property_name;
  char *property_syntax;
  int property_inherits;
  char *property_initial_value;

  struct ui_css_rule *next;
};

/**
 * @brief Represents a CSS layer definition (@layer).
 */
struct ui_css_layer {
  char *name;
  int order;
  struct ui_css_layer *next;
};

/**
 * @brief Represents a CSS namespace definition (@namespace).
 */
struct ui_css_namespace {
  char *prefix;
  char *uri;
  struct ui_css_namespace *next;
};

/**
 * @brief Represents a parsed CSS stylesheet.
 */
struct ui_css_stylesheet {
  struct ui_css_rule *rules;
  struct ui_css_layer *layers;
  struct ui_css_namespace *namespaces;
  int next_layer_order;
};

/**
 * @brief Represents a property resolved during the cascade.
 */
struct ui_css_computed_property {
  char *property_name;
  char *property_value;
  int is_important;
  int layer_order;
  int spec_a;
  int spec_b;
  int spec_c;
  int source_order;
  struct ui_css_computed_property *next;
};

/**
 * @brief Represents the resolved, computed style for a single DOM node.
 */
struct ui_css_computed_style {
  struct ui_css_computed_property *properties;
};

/**
 * @brief Creates a new CSS stylesheet.
 *
 * @param out_stylesheet Pointer to receive the new stylesheet.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_stylesheet_create(struct ui_css_stylesheet **out_stylesheet);

/**
 * @brief Destroys a CSS stylesheet and all its rules.
 *
 * @param stylesheet The stylesheet to destroy.
 */
ui_error_t ui_css_stylesheet_destroy(struct ui_css_stylesheet *stylesheet);

/**
 * @brief Appends a rule to the stylesheet.
 *
 * @param stylesheet The stylesheet.
 * @param rule The rule to append.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_stylesheet_append_rule(struct ui_css_stylesheet *stylesheet,
                                         struct ui_css_rule *rule);

/**
 * @brief Registers a layer by name in the stylesheet, or returns its existing
 * order.
 *
 * @param stylesheet The stylesheet.
 * @param name The layer name.
 * @param out_order Pointer to receive the assigned layer order integer.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_stylesheet_register_layer(struct ui_css_stylesheet *stylesheet,
                                 const char *name, int *out_order);

/**
 * @brief Registers a namespace by prefix in the stylesheet.
 *
 * @param stylesheet The stylesheet.
 * @param prefix The namespace prefix.
 * @param uri The namespace URI.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_stylesheet_register_namespace(struct ui_css_stylesheet *stylesheet,
                                     const char *prefix, const char *uri);

/**
 * @brief Creates a new CSS rule.
 *
 * @param type The type of rule to create.
 * @param out_rule Pointer to receive the new rule.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_rule_create(enum ui_css_rule_type type,
                              struct ui_css_rule **out_rule);

/**
 * @brief Destroys a CSS rule, including its selectors and declarations.
 *
 * @param rule The rule to destroy.
 */
ui_error_t ui_css_rule_destroy(struct ui_css_rule *rule);

/**
 * @brief Appends a selector to a rule.
 *
 * @param rule The rule.
 * @param type The type of selector.
 * @param value The string value of the selector.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_rule_append_selector(struct ui_css_rule *rule,
                                       enum ui_css_selector_type type,
                                       const char *value);

/**
 * @brief Appends an attribute selector to a rule.
 *
 * @param rule The rule.
 * @param attr_name The name of the attribute.
 * @param attr_op The operator.
 * @param attr_value The value to match (can be NULL if op is NONE).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_rule_append_selector_attr(struct ui_css_rule *rule,
                                            const char *attr_name,
                                            enum ui_css_attr_operator attr_op,
                                            const char *attr_value);

/**
 * @brief Appends a declaration to a rule.
 *
 * @param rule The rule.
 * @param property_name The name of the property.
 * @param property_value The value of the property.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_rule_append_declaration(struct ui_css_rule *rule,
                                          const char *property_name,
                                          const char *property_value,
                                          int is_important);

/**
 * @brief Resolves the style for a DOM node by matching it against the given
 * stylesheet.
 *
 * @param stylesheet The stylesheet to evaluate.
 * @param node The DOM node to style.
 * @param out_style Pointer to receive the computed style object.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_resolve_style(const struct ui_css_stylesheet *stylesheet,
                                const struct ui_dom_node *node,
                                struct ui_css_computed_style **out_style);

/**
 * @brief Retrieves the value of a property from a computed style.
 *
 * @param style The computed style.
 * @param property_name The property to look for.
 * @param out_value Pointer to receive the string value.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if the property is not
 * set.
 */
ui_error_t
ui_css_computed_style_get_property(const struct ui_css_computed_style *style,
                                   const char *property_name,
                                   const char **out_value);

/**
 * @brief Destroys a computed style object.
 *
 * @param style The computed style to destroy.
 */
ui_error_t ui_css_computed_style_destroy(struct ui_css_computed_style *style);

/**
 * @brief Represents a single CSS custom property (variable).
 */
struct ui_css_variable {
  char *name;
  char *value;
  struct ui_css_variable *next;
};

/**
 * @brief Store for a collection of CSS custom properties (variables).
 */
struct ui_css_variable_store {
  struct ui_css_variable *variables;
};

/**
 * @brief Creates a new CSS variable store.
 *
 * @param out_store Pointer to receive the new store.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_variable_store_create(struct ui_css_variable_store **out_store);

/**
 * @brief Destroys a CSS variable store.
 *
 * @param store The store to destroy.
 */
ui_error_t ui_css_variable_store_destroy(struct ui_css_variable_store *store);

/**
 * @brief Sets a CSS variable (e.g. "--my-color", "#f00") in the store.
 *
 * @param store The store.
 * @param name The variable name.
 * @param value The variable value.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_variable_store_set(struct ui_css_variable_store *store,
                                     const char *name, const char *value);

/**
 * @brief Resolves CSS variables (e.g. "var(--color, red)") within a property
 * value string using the provided store.
 *
 * @param store The variable store containing custom properties.
 * @param property_value The string containing var() functions.
 * @param out_resolved Pointer to receive the allocated resolved string.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_resolve_variables(const struct ui_css_variable_store *store,
                                    const char *property_value,
                                    char **out_resolved);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSSOM_H */
