#ifndef CMP_CSS_LISTS_CONTENT_H
#define CMP_CSS_LISTS_CONTENT_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_lists_content.h
 * @brief CSS Lists, Counters, and Content properties implementation.
 */

/**
 * @brief Represents list-style-type property.
 */
typedef enum cmp_list_style_type {
  CMP_LIST_STYLE_TYPE_NONE = 0,
  CMP_LIST_STYLE_TYPE_DISC,
  CMP_LIST_STYLE_TYPE_CIRCLE,
  CMP_LIST_STYLE_TYPE_SQUARE,
  CMP_LIST_STYLE_TYPE_DECIMAL,
  CMP_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO,
  CMP_LIST_STYLE_TYPE_LOWER_ROMAN,
  CMP_LIST_STYLE_TYPE_UPPER_ROMAN,
  CMP_LIST_STYLE_TYPE_LOWER_GREEK,
  CMP_LIST_STYLE_TYPE_LOWER_ALPHA,
  CMP_LIST_STYLE_TYPE_LOWER_LATIN,
  CMP_LIST_STYLE_TYPE_UPPER_ALPHA,
  CMP_LIST_STYLE_TYPE_UPPER_LATIN,
  CMP_LIST_STYLE_TYPE_ARMENIAN,
  CMP_LIST_STYLE_TYPE_GEORGIAN,
  CMP_LIST_STYLE_TYPE_HEBREW,
  CMP_LIST_STYLE_TYPE_STRING /* Custom string */
} cmp_list_style_type_t;

/**
 * @brief Represents list-style-position property.
 */
typedef enum cmp_list_style_position {
  CMP_LIST_STYLE_POSITION_OUTSIDE = 0,
  CMP_LIST_STYLE_POSITION_INSIDE
} cmp_list_style_position_t;

/**
 * @brief Represents the list-style group property.
 */
typedef struct cmp_prop_list_style_group {
  cmp_list_style_type_t type;
  char *custom_string; /**< Valid if type is STRING */
  cmp_list_style_position_t position;
  char *image; /**< String representation of list-style-image (URL/none) */
} cmp_prop_list_style_group_t;

/**
 * @brief Represents a single counter action (reset, increment, set).
 */
typedef struct cmp_counter_action {
  char *counter_name;
  int value;
} cmp_counter_action_t;

/**
 * @brief Represents counter-reset, counter-increment, counter-set group.
 */
typedef struct cmp_prop_counter_group {
  cmp_counter_action_t *resets;
  size_t num_resets;
  cmp_counter_action_t *increments;
  size_t num_increments;
  cmp_counter_action_t *sets;
  size_t num_sets;
} cmp_prop_counter_group_t;

/**
 * @brief Represents the content property.
 */
typedef struct cmp_prop_content {
  char *content; /**< Blob string representing content */
} cmp_prop_content_t;

/**
 * @brief Represents the quotes property.
 */
typedef struct cmp_prop_quotes {
  char *quotes; /**< String representation, e.g., "none", "auto", or custom pair
                 */
} cmp_prop_quotes_t;

/**
 * @brief Represents the content and quotes group.
 */
typedef struct cmp_prop_content_group {
  cmp_prop_content_t content_prop;
  cmp_prop_quotes_t quotes_prop;
} cmp_prop_content_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes list-style group.
 * @param group The group to initialize.
 * @param type list-style-type.
 * @param custom_string custom string if type is STRING.
 * @param position list-style-position.
 * @param image list-style-image string.
 * @return 0 on success.
 */
int cmp_prop_list_style_group_init(cmp_prop_list_style_group_t *group,
                                   cmp_list_style_type_t type,
                                   const char *custom_string,
                                   cmp_list_style_position_t position,
                                   const char *image);

/**
 * @brief Frees list-style group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_list_style_group_free(cmp_prop_list_style_group_t *group);

/**
 * @brief Initializes counter group.
 * @param group The group to initialize.
 * @return 0 on success.
 */
int cmp_prop_counter_group_init(cmp_prop_counter_group_t *group);

/**
 * @brief Adds a counter-reset.
 * @param group The group.
 * @param name counter name.
 * @param value reset value.
 * @return 0 on success.
 */
int cmp_prop_counter_group_add_reset(cmp_prop_counter_group_t *group,
                                     const char *name, int value);

/**
 * @brief Adds a counter-increment.
 * @param group The group.
 * @param name counter name.
 * @param value increment value.
 * @return 0 on success.
 */
int cmp_prop_counter_group_add_increment(cmp_prop_counter_group_t *group,
                                         const char *name, int value);

/**
 * @brief Adds a counter-set.
 * @param group The group.
 * @param name counter name.
 * @param value set value.
 * @return 0 on success.
 */
int cmp_prop_counter_group_add_set(cmp_prop_counter_group_t *group,
                                   const char *name, int value);

/**
 * @brief Frees counter group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_counter_group_free(cmp_prop_counter_group_t *group);

/**
 * @brief Initializes content group.
 * @param group The group to initialize.
 * @param content content string.
 * @param quotes quotes string.
 * @return 0 on success.
 */
int cmp_prop_content_group_init(cmp_prop_content_group_t *group,
                                const char *content, const char *quotes);

/**
 * @brief Frees content group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_content_group_free(cmp_prop_content_group_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_LISTS_CONTENT_H */
