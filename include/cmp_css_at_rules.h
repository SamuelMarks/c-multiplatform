/* clang-format off */
#ifndef CMP_CSS_AT_RULES_H
#define CMP_CSS_AT_RULES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents @media rule structure.
 */
typedef struct cmp_at_rule_media {
  char *condition_text; /**< Dynamically allocated string representing the query */
} cmp_at_rule_media_t;

/**
 * @brief Represents @container rule structure.
 */
typedef struct cmp_at_rule_container {
  char *name;           /**< Container name (optional) */
  char *condition_text; /**< Container query condition */
} cmp_at_rule_container_t;

/**
 * @brief Represents @supports rule structure.
 */
typedef struct cmp_at_rule_supports {
  char *condition_text; /**< Dynamically allocated string for supports condition */
} cmp_at_rule_supports_t;

/**
 * @brief Represents @import rule structure.
 */
typedef struct cmp_at_rule_import {
  char *url;            /**< URL to import */
  char *layer_name;     /**< Layer name (optional) */
  char *supports_cond;  /**< Supports condition (optional) */
  char *media_query;    /**< Media query (optional) */
} cmp_at_rule_import_t;

/**
 * @brief Represents @viewport rule structure (legacy).
 */
typedef struct cmp_at_rule_viewport {
  char *declarations; /**< Viewport rule declarations text */
} cmp_at_rule_viewport_t;

/**
 * @brief Represents @charset rule structure.
 */
typedef struct cmp_at_rule_charset {
  char *charset; /**< Character set encoding */
} cmp_at_rule_charset_t;

/* --- Initialization APIs --- */

/**
 * @brief Initialize @media rule.
 */
int cmp_at_rule_media_init(cmp_at_rule_media_t *rule, const char *condition);

/**
 * @brief Free @media rule.
 */
int cmp_at_rule_media_free(cmp_at_rule_media_t *rule);

/**
 * @brief Initialize @container rule.
 */
int cmp_at_rule_container_init(cmp_at_rule_container_t *rule, const char *name,
                               const char *condition);

/**
 * @brief Free @container rule.
 */
int cmp_at_rule_container_free(cmp_at_rule_container_t *rule);

/**
 * @brief Initialize @supports rule.
 */
int cmp_at_rule_supports_init(cmp_at_rule_supports_t *rule, const char *condition);

/**
 * @brief Free @supports rule.
 */
int cmp_at_rule_supports_free(cmp_at_rule_supports_t *rule);

/**
 * @brief Initialize @import rule.
 */
int cmp_at_rule_import_init(cmp_at_rule_import_t *rule, const char *url,
                            const char *layer, const char *supports,
                            const char *media);

/**
 * @brief Free @import rule.
 */
int cmp_at_rule_import_free(cmp_at_rule_import_t *rule);

/**
 * @brief Initialize @viewport rule.
 */
int cmp_at_rule_viewport_init(cmp_at_rule_viewport_t *rule, const char *declarations);

/**
 * @brief Free @viewport rule.
 */
int cmp_at_rule_viewport_free(cmp_at_rule_viewport_t *rule);

/**
 * @brief Initialize @charset rule.
 */
int cmp_at_rule_charset_init(cmp_at_rule_charset_t *rule, const char *charset);

/**
 * @brief Free @charset rule.
 */
int cmp_at_rule_charset_free(cmp_at_rule_charset_t *rule);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_AT_RULES_H */
/* clang-format on */
