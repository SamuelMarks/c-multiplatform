/* clang-format off */
#ifndef CMP_CSSOM_H
#define CMP_CSSOM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Types of CSS rules.
 */
typedef enum cmp_cssom_rule_type {
  CMP_CSSOM_RULE_UNKNOWN = 0,
  CMP_CSSOM_RULE_STYLE,
  CMP_CSSOM_RULE_MEDIA,
  CMP_CSSOM_RULE_FONT_FACE,
  CMP_CSSOM_RULE_KEYFRAMES
} cmp_cssom_rule_type_t;

/**
 * @brief Represents a CSSStyleDeclaration.
 */
typedef struct cmp_cssom_decl {
  char *css_text; /**< The full text of the declaration block */
  int length;     /**< Number of properties */
} cmp_cssom_decl_t;

/**
 * @brief Represents a generic CSSRule.
 */
typedef struct cmp_cssom_rule {
  cmp_cssom_rule_type_t type; /**< Rule type */
  char *css_text;             /**< Full CSS text of the rule */
  struct cmp_cssom_rule *parent_rule;
} cmp_cssom_rule_t;

/**
 * @brief Represents a getComputedStyle result.
 */
typedef struct cmp_cssom_computed_style {
  void *node_ref; /**< Opaque pointer to the DOM node */
  cmp_cssom_decl_t *decl;
} cmp_cssom_computed_style_t;

/**
 * @brief Represents a dynamic stylesheet mutator.
 */
typedef struct cmp_cssom_mutator {
  void *stylesheet_ref; /**< Opaque reference to the stylesheet */
} cmp_cssom_mutator_t;

/**
 * @brief Initialize a CSS rule.
 * @param rule The rule to initialize.
 * @param type The type of the rule.
 * @param css_text The CSS text.
 * @return 0 on success.
 */
int cmp_cssom_rule_init(cmp_cssom_rule_t *rule, cmp_cssom_rule_type_t type,
                        const char *css_text);

/**
 * @brief Free a CSS rule.
 * @param rule The rule to free.
 * @return 0 on success.
 */
int cmp_cssom_rule_free(cmp_cssom_rule_t *rule);

/**
 * @brief Initialize a CSSStyleDeclaration.
 * @param decl The declaration to initialize.
 * @param css_text The CSS text.
 * @return 0 on success.
 */
int cmp_cssom_decl_init(cmp_cssom_decl_t *decl, const char *css_text);

/**
 * @brief Free a CSSStyleDeclaration.
 * @param decl The declaration to free.
 * @return 0 on success.
 */
int cmp_cssom_decl_free(cmp_cssom_decl_t *decl);

/**
 * @brief Initialize a computed style object.
 * @param cs Computed style object.
 * @param node_ref Opaque node reference.
 * @return 0 on success.
 */
int cmp_cssom_computed_style_init(cmp_cssom_computed_style_t *cs, void *node_ref);

/**
 * @brief Free a computed style object.
 * @param cs Computed style object.
 * @return 0 on success.
 */
int cmp_cssom_computed_style_free(cmp_cssom_computed_style_t *cs);

/**
 * @brief Initialize a mutator.
 * @param mutator Mutator object.
 * @param stylesheet_ref Opaque stylesheet reference.
 * @return 0 on success.
 */
int cmp_cssom_mutator_init(cmp_cssom_mutator_t *mutator, void *stylesheet_ref);

/**
 * @brief Insert a rule dynamically.
 * @param mutator Mutator object.
 * @param rule Rule text to insert.
 * @param index Index at which to insert.
 * @return 0 on success.
 */
int cmp_cssom_mutator_insert_rule(cmp_cssom_mutator_t *mutator, const char *rule,
                                  size_t index);

/**
 * @brief Delete a rule dynamically.
 * @param mutator Mutator object.
 * @param index Index of rule to delete.
 * @return 0 on success.
 */
int cmp_cssom_mutator_delete_rule(cmp_cssom_mutator_t *mutator, size_t index);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSSOM_H */
/* clang-format on */
