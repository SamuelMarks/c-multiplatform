#ifndef CMP_CSS_MULTI_COLUMN_H
#define CMP_CSS_MULTI_COLUMN_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_multi_column.h
 * @brief CSS Multi-column Layout implementation.
 */

/**
 * @brief Represents column-width, column-count, and columns properties.
 */
typedef struct cmp_prop_column_def_group {
  char *width; /**< column-width (e.g., "auto", "120px") */
  char *count; /**< column-count (e.g., "auto", "3") */
} cmp_prop_column_def_group_t;

/**
 * @brief Represents column-rule-width, column-rule-style, column-rule-color,
 * and column-rule properties.
 */
typedef struct cmp_prop_column_rule_group {
  char *width; /**< column-rule-width (e.g., "thin", "1px") */
  char *style; /**< column-rule-style (e.g., "solid", "dashed") */
  char *color; /**< column-rule-color (e.g., "red", "transparent") */
} cmp_prop_column_rule_group_t;

/**
 * @brief Values for the column-span property.
 */
typedef enum cmp_column_span {
  CMP_COLUMN_SPAN_NONE = 0,
  CMP_COLUMN_SPAN_ALL
} cmp_column_span_t;

/**
 * @brief Values for the column-fill property.
 */
typedef enum cmp_column_fill {
  CMP_COLUMN_FILL_BALANCE = 0,
  CMP_COLUMN_FILL_AUTO
} cmp_column_fill_t;

/**
 * @brief Represents column-span and column-fill properties.
 */
typedef struct cmp_prop_column_span_fill {
  cmp_column_span_t span;
  cmp_column_fill_t fill;
} cmp_prop_column_span_fill_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a column definition group (width and count).
 * @param group The definition group to initialize.
 * @param width The column-width value (can be NULL).
 * @param count The column-count value (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_column_def_group_init(cmp_prop_column_def_group_t *group,
                                   const char *width, const char *count);

/**
 * @brief Frees resources in a column definition group.
 * @param group The definition group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_column_def_group_free(cmp_prop_column_def_group_t *group);

/**
 * @brief Initializes a column rule group (width, style, and color).
 * @param group The rule group to initialize.
 * @param width The column-rule-width value (can be NULL).
 * @param style The column-rule-style value (can be NULL).
 * @param color The column-rule-color value (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_column_rule_group_init(cmp_prop_column_rule_group_t *group,
                                    const char *width, const char *style,
                                    const char *color);

/**
 * @brief Frees resources in a column rule group.
 * @param group The rule group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_column_rule_group_free(cmp_prop_column_rule_group_t *group);

/**
 * @brief Initializes a column span and fill property group.
 * @param group The span/fill group to initialize.
 * @param span The column-span value.
 * @param fill The column-fill value.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_column_span_fill_init(cmp_prop_column_span_fill_t *group,
                                   cmp_column_span_t span,
                                   cmp_column_fill_t fill);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_MULTI_COLUMN_H */