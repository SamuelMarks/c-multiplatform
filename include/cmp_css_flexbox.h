#ifndef CMP_CSS_FLEXBOX_H
#define CMP_CSS_FLEXBOX_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_flexbox.h
 * @brief CSS Flexible Box Layout (Flexbox) implementation.
 */

/**
 * @brief Represents flex-direction.
 */
typedef enum cmp_flex_direction {
  CMP_FLEX_DIRECTION_ROW = 0,
  CMP_FLEX_DIRECTION_ROW_REVERSE,
  CMP_FLEX_DIRECTION_COLUMN,
  CMP_FLEX_DIRECTION_COLUMN_REVERSE
} cmp_flex_direction_t;

/**
 * @brief Represents flex-wrap.
 */
typedef enum cmp_flex_wrap {
  CMP_FLEX_WRAP_NOWRAP = 0,
  CMP_FLEX_WRAP_WRAP,
  CMP_FLEX_WRAP_WRAP_REVERSE
} cmp_flex_wrap_t;

/**
 * @brief Represents flex-direction, flex-wrap, and flex-flow.
 */
typedef struct cmp_prop_flex_container_group {
  cmp_flex_direction_t direction; /**< flex-direction */
  cmp_flex_wrap_t wrap;           /**< flex-wrap */
} cmp_prop_flex_container_group_t;

/**
 * @brief Represents flex-grow, flex-shrink, flex-basis, and flex.
 */
typedef struct cmp_prop_flex_item_group {
  float grow;   /**< flex-grow (e.g., 0.0, 1.0) */
  float shrink; /**< flex-shrink (e.g., 1.0) */
  char *basis;  /**< flex-basis string representation (e.g., "auto", "100px") */
} cmp_prop_flex_item_group_t;

/**
 * @brief Represents the order property.
 */
typedef struct cmp_prop_order {
  int order; /**< order integer value (default 0) */
} cmp_prop_order_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a flex container group (flex-direction and flex-wrap).
 * @param group The flex container group to initialize.
 * @param direction The flex-direction value.
 * @param wrap The flex-wrap value.
 * @return 0 on success.
 */
int cmp_prop_flex_container_group_init(cmp_prop_flex_container_group_t *group,
                                       cmp_flex_direction_t direction,
                                       cmp_flex_wrap_t wrap);

/**
 * @brief Initializes a flex item group (flex-grow, flex-shrink, flex-basis).
 * @param group The flex item group to initialize.
 * @param grow The flex-grow value.
 * @param shrink The flex-shrink value.
 * @param basis The flex-basis value (can be NULL).
 * @return 0 on success.
 */
int cmp_prop_flex_item_group_init(cmp_prop_flex_item_group_t *group, float grow,
                                  float shrink, const char *basis);

/**
 * @brief Frees resources in a flex item group.
 * @param group The flex item group to free.
 * @return 0 on success.
 */
int cmp_prop_flex_item_group_free(cmp_prop_flex_item_group_t *group);

/**
 * @brief Initializes an order property.
 * @param order_prop The order property to initialize.
 * @param order The integer order value.
 * @return 0 on success.
 */
int cmp_prop_order_init(cmp_prop_order_t *order_prop, int order);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_FLEXBOX_H */