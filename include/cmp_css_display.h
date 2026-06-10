#ifndef CMP_CSS_DISPLAY_H
#define CMP_CSS_DISPLAY_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_display.h
 * @brief CSS Display & Flow Layout module implementation.
 */

/**
 * @brief Represents the CSS display property.
 */
typedef enum cmp_prop_display {
  CMP_DISPLAY_NONE = 0,
  CMP_DISPLAY_BLOCK,
  CMP_DISPLAY_INLINE,
  CMP_DISPLAY_INLINE_BLOCK,
  CMP_DISPLAY_FLEX,
  CMP_DISPLAY_INLINE_FLEX,
  CMP_DISPLAY_GRID,
  CMP_DISPLAY_INLINE_GRID,
  CMP_DISPLAY_TABLE,
  CMP_DISPLAY_INLINE_TABLE,
  CMP_DISPLAY_TABLE_ROW,
  CMP_DISPLAY_TABLE_CELL,
  CMP_DISPLAY_TABLE_COLUMN,
  CMP_DISPLAY_TABLE_COLUMN_GROUP,
  CMP_DISPLAY_TABLE_HEADER_GROUP,
  CMP_DISPLAY_TABLE_FOOTER_GROUP,
  CMP_DISPLAY_TABLE_ROW_GROUP,
  CMP_DISPLAY_TABLE_CAPTION,
  CMP_DISPLAY_CONTENTS,
  CMP_DISPLAY_FLOW_ROOT,
  CMP_DISPLAY_RUBY,
  CMP_DISPLAY_RUBY_BASE,
  CMP_DISPLAY_RUBY_TEXT,
  CMP_DISPLAY_RUBY_BASE_CONTAINER,
  CMP_DISPLAY_RUBY_TEXT_CONTAINER,
  CMP_DISPLAY_LIST_ITEM
} cmp_prop_display_t;

/**
 * @brief Represents the CSS float property.
 */
typedef enum cmp_prop_float {
  CMP_FLOAT_NONE = 0,
  CMP_FLOAT_LEFT,
  CMP_FLOAT_RIGHT,
  CMP_FLOAT_INLINE_START,
  CMP_FLOAT_INLINE_END
} cmp_prop_float_t;

/**
 * @brief Represents advanced float properties.
 */
typedef struct cmp_prop_float_advanced {
  int defer;       /**< float-defer */
  char *reference; /**< float-reference string */
} cmp_prop_float_advanced_t;

/**
 * @brief Represents the CSS clear property.
 */
typedef enum cmp_prop_clear {
  CMP_CLEAR_NONE = 0,
  CMP_CLEAR_LEFT,
  CMP_CLEAR_RIGHT,
  CMP_CLEAR_BOTH,
  CMP_CLEAR_INLINE_START,
  CMP_CLEAR_INLINE_END
} cmp_prop_clear_t;

/**
 * @brief Represents clear-after property.
 */
typedef struct cmp_prop_clear_after {
  cmp_prop_clear_t clear;
} cmp_prop_clear_after_t;

/**
 * @brief Represents the CSS visibility property.
 */
typedef enum cmp_prop_visibility {
  CMP_VISIBILITY_VISIBLE = 0,
  CMP_VISIBILITY_HIDDEN,
  CMP_VISIBILITY_COLLAPSE
} cmp_prop_visibility_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes an advanced float property.
 * @param adv The advanced float property to initialize.
 * @param defer The float-defer value.
 * @param reference The float-reference string.
 * @return 0 on success.
 */
int cmp_prop_float_advanced_init(cmp_prop_float_advanced_t *adv, int defer,
                                 const char *reference);

/**
 * @brief Frees resources within an advanced float property.
 * @param adv The advanced float property to free.
 * @return 0 on success.
 */
int cmp_prop_float_advanced_free(cmp_prop_float_advanced_t *adv);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_DISPLAY_H */