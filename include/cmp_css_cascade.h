#ifndef CMP_CSS_CASCADE_H
#define CMP_CSS_CASCADE_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents the !important flag.
 */
typedef struct cmp_cascade_important {
  int is_important; /**< Non-zero if the declaration is !important */
} cmp_cascade_important_t;

/**
 * @brief Represents a CSS specificity score.
 * An array of 3 integers: a (id), b (class/attr/pseudo-class), c
 * (type/pseudo-element).
 */
typedef struct cmp_specificity {
  unsigned int a; /**< Number of ID selectors */
  unsigned int b; /**< Number of class, attribute, and pseudo-class selectors */
  unsigned int c; /**< Number of type selectors and pseudo-elements */
} cmp_specificity_t;

/**
 * @brief Computes the specificity of a selector string (placeholder logic).
 * @param selector_str The selector string to evaluate.
 * @param out_specificity The resulting specificity structure.
 * @return 0 on success, non-zero error code on failure.
 */
int cmp_specificity_compute(const char *selector_str,
                            cmp_specificity_t *out_specificity);

/**
 * @brief Compares two specificity values.
 * @param s1 First specificity.
 * @param s2 Second specificity.
 * @param out_cmp Result of comparison: < 0 if s1 < s2, 0 if s1 == s2, > 0 if s1
 * > s2.
 * @return 0 on success, non-zero on error.
 */
int cmp_specificity_compare(const cmp_specificity_t *s1,
                            const cmp_specificity_t *s2, int *out_cmp);

/**
 * @brief CSS global keywords.
 */
typedef enum cmp_keyword {
  CMP_KEYWORD_NONE = 0,
  CMP_KEYWORD_INITIAL,
  CMP_KEYWORD_INHERIT,
  CMP_KEYWORD_UNSET,
  CMP_KEYWORD_REVERT,
  CMP_KEYWORD_REVERT_LAYER
} cmp_keyword_t;

/**
 * @brief Represents the 'all' property which resets all properties except
 * unicode-bidi, direction, and custom properties.
 */
typedef struct cmp_prop_all {
  cmp_keyword_t keyword; /**< The keyword assigned to 'all' */
} cmp_prop_all_t;

/**
 * @brief Represents an @layer rule.
 */
typedef struct cmp_at_rule_layer {
  char *name; /**< Layer name, can be NULL for anonymous layers */
} cmp_at_rule_layer_t;

/**
 * @brief Represents a custom property (--*).
 */
typedef struct cmp_css_var {
  char *name;  /**< The name of the custom property, including the -- prefix */
  char *value; /**< The value of the custom property */
} cmp_css_var_t;

/**
 * @brief Represents a var() function call.
 */
typedef struct cmp_var_func {
  char *custom_property_name; /**< The name of the custom property to substitute
                               */
  char *
      fallback; /**< Optional fallback value if the property is invalid/unset */
} cmp_var_func_t;

/**
 * @brief Represents an @property rule for custom property registration.
 */
typedef struct cmp_at_rule_property {
  char *name;          /**< Name of the custom property */
  char *syntax;        /**< Syntax string, e.g., "<color>" */
  int inherits;        /**< Non-zero if the property inherits */
  char *initial_value; /**< The initial value string */
} cmp_at_rule_property_t;

/**
 * @brief Initializes an @layer rule.
 * @param layer The layer to initialize.
 * @param name The layer name.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_layer_init(cmp_at_rule_layer_t *layer, const char *name);

/**
 * @brief Frees resources in an @layer rule.
 * @param layer The layer to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_layer_free(cmp_at_rule_layer_t *layer);

/**
 * @brief Initializes a custom property.
 * @param var The custom property to initialize.
 * @param name The name of the custom property.
 * @param value The value of the custom property.
 * @return 0 on success, non-zero on error.
 */
int cmp_css_var_init(cmp_css_var_t *var, const char *name, const char *value);

/**
 * @brief Frees resources in a custom property.
 * @param var The custom property to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_css_var_free(cmp_css_var_t *var);

/**
 * @brief Initializes a var() function.
 * @param func The var() function to initialize.
 * @param name The custom property name.
 * @param fallback The fallback string (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_var_func_init(cmp_var_func_t *func, const char *name,
                      const char *fallback);

/**
 * @brief Frees resources in a var() function.
 * @param func The var() function to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_var_func_free(cmp_var_func_t *func);

/**
 * @brief Initializes an @property rule.
 * @param prop The @property rule to initialize.
 * @param name The name of the custom property.
 * @param syntax The syntax string.
 * @param inherits Non-zero if the property inherits.
 * @param initial_value The initial value string.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_property_init(cmp_at_rule_property_t *prop, const char *name,
                              const char *syntax, int inherits,
                              const char *initial_value);

/**
 * @brief Frees resources in an @property rule.
 * @param prop The @property rule to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_at_rule_property_free(cmp_at_rule_property_t *prop);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_CASCADE_H */