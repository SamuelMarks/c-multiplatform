/**
 * @file ui_cssom.h
 * @brief CSS Object Model structures and parsing.
 *
 * This header defines structures and functions representing the CSS Object
 * Model, covering selectors, declarations, rules, layers, namespaces,
 * stylesheets, and computed styles.
 */

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
  UI_CSS_SELECTOR_TYPE_UNIVERSAL,     /**< Universal selector (*). */
  UI_CSS_SELECTOR_TYPE_TAG,           /**< Tag name selector (div). */
  UI_CSS_SELECTOR_TYPE_CLASS,         /**< Class selector (.class). */
  UI_CSS_SELECTOR_TYPE_ID,            /**< ID selector (#id). */
  UI_CSS_SELECTOR_TYPE_ATTRIBUTE,     /**< Attribute selector ([attr=val]). */
  UI_CSS_SELECTOR_TYPE_PSEUDO_CLASS,  /**< Pseudo-class selector (:hover). */
  UI_CSS_SELECTOR_TYPE_PSEUDO_ELEMENT /**< Pseudo-element selector (::before).
                                       */
};

/**
 * @brief Operator used for attribute selectors.
 */
enum ui_css_attr_operator {
  UI_CSS_ATTR_OP_NONE = 0, /**< No operator, just existence ([attr]). */
  UI_CSS_ATTR_OP_EQUALS,   /**< Exact match ([attr="val"]). */
  UI_CSS_ATTR_OP_INCLUDES, /**< Whitespace-separated includes ([attr~="val"]).
                            */
  UI_CSS_ATTR_OP_DASH,     /**< Hyphen-separated match ([attr|="val"]). */
  UI_CSS_ATTR_OP_PREFIX,   /**< Prefix match ([attr^="val"]). */
  UI_CSS_ATTR_OP_SUFFIX,   /**< Suffix match ([attr$="val"]). */
  UI_CSS_ATTR_OP_SUBSTRING /**< Substring match ([attr*="val"]). */
};

/**
 * @brief Represents a simple CSS selector (e.g., * or div or .btn or #main or
 * [attr="val"]).
 */
struct ui_css_selector {
  enum ui_css_selector_type type; /**< The type of the selector. */
  char *value; /**< NULL for UNIVERSAL. For ATTRIBUTE, this is the attr name. */
  char *attr_value; /**< The attribute value to match against. Only used for
                       ATTRIBUTE. */
  enum ui_css_attr_operator
      attr_op; /**< The attribute match operator. Only used for ATTRIBUTE. */
  struct ui_css_selector
      *nested_selector; /**< Used for functional pseudo-classes like :is(),
                           :has(), :not(), :where(). */
  struct ui_css_selector *next; /**< Pointer to the next selector in a compound
                                   or complex selector chain. */
};

/**
 * @brief Represents a CSS declaration (property: value).
 */
struct ui_css_declaration {
  char *property_name;  /**< The name of the CSS property. */
  char *property_value; /**< The value of the CSS property. */
  int is_important;     /**< 1 if marked with !important, 0 otherwise. */
  struct ui_css_declaration
      *next; /**< Pointer to the next declaration in the block. */
};

/**
 * @brief Represents the type of a CSS rule.
 */
enum ui_css_rule_type {
  UI_CSS_RULE_TYPE_STYLE,    /**< A standard style rule. */
  UI_CSS_RULE_TYPE_MEDIA,    /**< An \@media rule. */
  UI_CSS_RULE_TYPE_LAYER,    /**< An \@layer rule. */
  UI_CSS_RULE_TYPE_SCOPE,    /**< An \@scope rule. */
  UI_CSS_RULE_TYPE_PROPERTY, /**< An \@property rule. */
  UI_CSS_RULE_TYPE_SUPPORTS, /**< An \@supports rule. */
  UI_CSS_RULE_TYPE_CONTAINER /**< An \@container rule. */
};

/**
 * @brief Represents a CSS rule (selectors { declarations }).
 */
struct ui_css_rule {
  enum ui_css_rule_type type; /**< The rule type. */

  /* For STYLE rules */
  struct ui_css_selector
      *selectors; /**< Head of the selector list for STYLE rules. */
  struct ui_css_declaration
      *declarations; /**< Head of the declaration list for STYLE rules. */

  /* For MEDIA rules */
  char *media_condition; /**< The media query condition string. */
  struct ui_css_rule
      *nested_rules; /**< Rules nested inside this conditional rule. */

  /* For SUPPORTS rules */
  char *supports_condition; /**< The supports condition string. */

  /* For CONTAINER rules */
  char *container_condition; /**< The container query condition string. */

  /* For LAYER rules */
  char *layer_name; /**< The name of the layer. */

  /* For SCOPE rules */
  struct ui_css_selector *scope_start; /**< The scoping root selector. */
  struct ui_css_selector *scope_end;   /**< The scoping limit selector. */

  /* For PROPERTY rules (@property) */
  char *property_name;   /**< Custom property name. */
  char *property_syntax; /**< Custom property syntax. */
  int property_inherits; /**< 1 if custom property inherits, 0 otherwise. */
  char *property_initial_value; /**< Custom property initial value string. */

  struct ui_css_rule
      *next; /**< Pointer to the next rule in the stylesheet or block. */
};

/**
 * @brief Represents a CSS layer definition (\@layer).
 */
struct ui_css_layer {
  char *name;                /**< The name of the layer. */
  int order;                 /**< The assigned order of the layer in cascade. */
  struct ui_css_layer *next; /**< Pointer to the next layer definition. */
};

/**
 * @brief Represents a CSS namespace definition (\@namespace).
 */
struct ui_css_namespace {
  char *prefix; /**< The namespace prefix. */
  char *uri;    /**< The namespace URI. */
  struct ui_css_namespace
      *next; /**< Pointer to the next namespace definition. */
};

/**
 * @brief Represents a parsed CSS stylesheet.
 */
struct ui_css_stylesheet {
  struct ui_css_rule *rules;   /**< Linked list of rules in the stylesheet. */
  struct ui_css_layer *layers; /**< Linked list of declared layers. */
  struct ui_css_namespace
      *namespaces;      /**< Linked list of declared namespaces. */
  int next_layer_order; /**< The next available layer order index. */
};

/**
 * @brief Represents a property resolved during the cascade.
 */
struct ui_css_computed_property {
  char *property_name;  /**< The resolved property name. */
  char *property_value; /**< The resolved property value string. */
  int is_important;     /**< 1 if the winning declaration was !important, 0
                           otherwise. */
  int layer_order;  /**< The cascade layer order of the winning declaration. */
  int spec_a;       /**< Selector specificity (ID). */
  int spec_b;       /**< Selector specificity (Class/Attr/Pseudo). */
  int spec_c;       /**< Selector specificity (Tag/Pseudo-element). */
  int source_order; /**< Source document order index. */
  struct ui_css_computed_property
      *next; /**< Pointer to the next computed property. */
};

/**
 * @brief Represents the resolved, computed style for a single DOM node.
 */
struct ui_css_computed_style {
  struct ui_css_computed_property
      *properties; /**< Linked list of computed properties. */
};

/**
 * @brief Creates a new CSS stylesheet.
 *
 * @param out_stylesheet Pointer to receive the new stylesheet.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_stylesheet_create(struct ui_css_stylesheet **out_stylesheet);

/**
 * @brief Destroys a CSS stylesheet and all its rules.
 *
 * @param stylesheet Pointer to the stylesheet to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_stylesheet_destroy(struct ui_css_stylesheet *stylesheet);

/**
 * @brief Appends a rule to the stylesheet.
 *
 * @param stylesheet Pointer to the stylesheet.
 * @param rule Pointer to the rule to append.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_stylesheet_append_rule(struct ui_css_stylesheet *stylesheet,
                                         struct ui_css_rule *rule);

/**
 * @brief Registers a layer by name in the stylesheet, or returns its existing
 * order.
 *
 * @param stylesheet Pointer to the stylesheet.
 * @param name The layer name.
 * @param out_order Pointer to receive the assigned layer order integer.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_css_stylesheet_register_layer(struct ui_css_stylesheet *stylesheet,
                                 const char *name, int *out_order);

/**
 * @brief Registers a namespace by prefix in the stylesheet.
 *
 * @param stylesheet Pointer to the stylesheet.
 * @param prefix The namespace prefix.
 * @param uri The namespace URI.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_css_stylesheet_register_namespace(struct ui_css_stylesheet *stylesheet,
                                     const char *prefix, const char *uri);

/**
 * @brief Creates a new CSS rule.
 *
 * @param type The type of rule to create.
 * @param out_rule Pointer to receive the new rule.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_rule_create(enum ui_css_rule_type type,
                              struct ui_css_rule **out_rule);

/**
 * @brief Destroys a CSS rule, including its selectors and declarations.
 *
 * @param rule Pointer to the rule to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_rule_destroy(struct ui_css_rule *rule);

/**
 * @brief Destroys a CSS selector and its linked elements.
 *
 * @param sel Pointer to the selector to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_selector_destroy(struct ui_css_selector *sel);

/**
 * @brief Appends a selector to a rule.
 *
 * @param rule Pointer to the rule.
 * @param type The type of selector.
 * @param value The string value of the selector.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_rule_append_selector(struct ui_css_rule *rule,
                                       enum ui_css_selector_type type,
                                       const char *value);

/**
 * @brief Appends an attribute selector to a rule.
 *
 * @param rule Pointer to the rule.
 * @param attr_name The name of the attribute.
 * @param attr_op The operator.
 * @param attr_value The value to match (can be NULL if op is NONE).
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_rule_append_selector_attr(struct ui_css_rule *rule,
                                            const char *attr_name,
                                            enum ui_css_attr_operator attr_op,
                                            const char *attr_value);

/**
 * @brief Appends a declaration to a rule.
 *
 * @param rule Pointer to the rule.
 * @param property_name The name of the property.
 * @param property_value The value of the property.
 * @param is_important 1 if important, 0 otherwise.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_rule_append_declaration(struct ui_css_rule *rule,
                                          const char *property_name,
                                          const char *property_value,
                                          int is_important);

/**
 * @brief Resolves the style for a DOM node by matching it against the given
 * stylesheet.
 *
 * @param stylesheet Pointer to the stylesheet to evaluate.
 * @param node Pointer to the DOM node to style.
 * @param out_style Pointer to receive the computed style object.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_resolve_style(const struct ui_css_stylesheet *stylesheet,
                                const struct ui_dom_node *node,
                                struct ui_css_computed_style **out_style);

/**
 * @brief Retrieves the value of a property from a computed style.
 *
 * @param style Pointer to the computed style.
 * @param property_name The property to look for.
 * @param out_value Pointer to receive the string value.
 * @return `UI_ERROR_NONE` on success, `UI_ERROR_NOT_FOUND` if the property is
 * not set.
 */
ui_error_t
ui_css_computed_style_get_property(const struct ui_css_computed_style *style,
                                   const char *property_name,
                                   const char **out_value);

/**
 * @brief Destroys a computed style object.
 *
 * @param style Pointer to the computed style to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_computed_style_destroy(struct ui_css_computed_style *style);

/**
 * @brief Represents a single CSS custom property (variable).
 */
struct ui_css_variable {
  char *name;                   /**< Variable name (e.g. "--color"). */
  char *value;                  /**< Variable value. */
  struct ui_css_variable *next; /**< Pointer to the next custom property. */
};

/**
 * @brief Store for a collection of CSS custom properties (variables).
 */
struct ui_css_variable_store {
  struct ui_css_variable *variables; /**< Linked list of variables. */
};

/**
 * @brief Creates a new CSS variable store.
 *
 * @param out_store Pointer to receive the new store.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_css_variable_store_create(struct ui_css_variable_store **out_store);

/**
 * @brief Destroys a CSS variable store.
 *
 * @param store Pointer to the store to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_variable_store_destroy(struct ui_css_variable_store *store);

/**
 * @brief Sets a CSS variable (e.g. "--my-color", "#f00") in the store.
 *
 * @param store Pointer to the store.
 * @param name The variable name.
 * @param value The variable value.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
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
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_resolve_variables(const struct ui_css_variable_store *store,
                                    const char *property_value,
                                    char **out_resolved);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSSOM_H */
