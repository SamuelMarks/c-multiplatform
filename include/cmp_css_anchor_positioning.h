#ifndef CMP_CSS_ANCHOR_POSITIONING_H
#define CMP_CSS_ANCHOR_POSITIONING_H

/* clang-format off */
#include "cmp_css_box_model.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_anchor_positioning.h
 * @brief CSS Anchor Positioning API module implementation.
 */

/**
 * @brief Represents the anchor-name property.
 */
typedef struct cmp_prop_anchor_name {
  char *name; /**< The dashed-ident name of the anchor */
} cmp_prop_anchor_name_t;

/**
 * @brief Represents the position-anchor property.
 */
typedef struct cmp_prop_position_anchor {
  char *name; /**< The dashed-ident name of the default anchor */
} cmp_prop_position_anchor_t;

/**
 * @brief CSS position-visibility property.
 */
typedef enum cmp_prop_position_visibility {
  CMP_POSITION_VISIBILITY_ALWAYS = 0,
  CMP_POSITION_VISIBILITY_ANCHORS_VISIBLE,
  CMP_POSITION_VISIBILITY_NO_OVERFLOW
} cmp_prop_position_visibility_t;

/**
 * @brief CSS anchor() function side.
 */
typedef enum cmp_anchor_side {
  CMP_ANCHOR_SIDE_AUTO = 0,
  CMP_ANCHOR_SIDE_TOP,
  CMP_ANCHOR_SIDE_LEFT,
  CMP_ANCHOR_SIDE_RIGHT,
  CMP_ANCHOR_SIDE_BOTTOM,
  CMP_ANCHOR_SIDE_START,
  CMP_ANCHOR_SIDE_END,
  CMP_ANCHOR_SIDE_SELF_START,
  CMP_ANCHOR_SIDE_SELF_END,
  CMP_ANCHOR_SIDE_CENTER
} cmp_anchor_side_t;

/**
 * @brief Represents an anchor() function call.
 */
typedef struct cmp_anchor_func {
  char *anchor_name;        /**< Optional name of the anchor */
  cmp_anchor_side_t side;   /**< Which side of the anchor to align with */
  cmp_prop_size_t fallback; /**< Fallback length if anchor is invalid */
} cmp_anchor_func_t;

/**
 * @brief CSS anchor-size() function dimension.
 */
typedef enum cmp_anchor_size_dim {
  CMP_ANCHOR_SIZE_WIDTH = 0,
  CMP_ANCHOR_SIZE_HEIGHT,
  CMP_ANCHOR_SIZE_BLOCK,
  CMP_ANCHOR_SIZE_INLINE,
  CMP_ANCHOR_SIZE_SELF_BLOCK,
  CMP_ANCHOR_SIZE_SELF_INLINE
} cmp_anchor_size_dim_t;

/**
 * @brief Represents an anchor-size() function call.
 */
typedef struct cmp_anchor_size_func {
  char *anchor_name;               /**< Optional name of the anchor */
  cmp_anchor_size_dim_t dimension; /**< Which dimension to use */
  cmp_prop_size_t fallback;        /**< Fallback size */
} cmp_anchor_size_func_t;

/**
 * @brief Grouping for anchor functions.
 */
typedef struct cmp_anchor_funcs {
  int is_size; /* Non-zero if this is an anchor-size() function */
  union {
    cmp_anchor_func_t anchor;
    cmp_anchor_size_func_t anchor_size;
  } func;
} cmp_anchor_funcs_t;

/**
 * @brief CSS position-try-order property.
 */
typedef enum cmp_position_try_order {
  CMP_POSITION_TRY_ORDER_NORMAL = 0,
  CMP_POSITION_TRY_ORDER_MOST_WIDTH,
  CMP_POSITION_TRY_ORDER_MOST_HEIGHT,
  CMP_POSITION_TRY_ORDER_MOST_BLOCK_SIZE,
  CMP_POSITION_TRY_ORDER_MOST_INLINE_SIZE
} cmp_position_try_order_t;

/**
 * @brief Represents the position-try property (shorthand for options + order).
 */
typedef struct cmp_prop_position_try {
  char *options; /**< Space-separated string of try-options (e.g. "flip-block,
                    --custom-try") */
  cmp_position_try_order_t order;
} cmp_prop_position_try_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes an anchor-name property.
 * @param anchor The property.
 * @param name The dashed-ident name.
 * @return 0 on success.
 */
int cmp_prop_anchor_name_init(cmp_prop_anchor_name_t *anchor, const char *name);

/**
 * @brief Frees resources in an anchor-name property.
 * @param anchor The property.
 * @return 0 on success.
 */
int cmp_prop_anchor_name_free(cmp_prop_anchor_name_t *anchor);

/**
 * @brief Initializes a position-anchor property.
 * @param anchor The property.
 * @param name The dashed-ident name.
 * @return 0 on success.
 */
int cmp_prop_position_anchor_init(cmp_prop_position_anchor_t *anchor,
                                  const char *name);

/**
 * @brief Frees resources in a position-anchor property.
 * @param anchor The property.
 * @return 0 on success.
 */
int cmp_prop_position_anchor_free(cmp_prop_position_anchor_t *anchor);

/**
 * @brief Initializes an anchor() function.
 * @param func The function structure.
 * @param name Optional anchor name.
 * @param side The anchor side.
 * @return 0 on success.
 */
int cmp_anchor_func_init(cmp_anchor_funcs_t *func, const char *name,
                         cmp_anchor_side_t side);

/**
 * @brief Initializes an anchor-size() function.
 * @param func The function structure.
 * @param name Optional anchor name.
 * @param dim The dimension to capture.
 * @return 0 on success.
 */
int cmp_anchor_size_func_init(cmp_anchor_funcs_t *func, const char *name,
                              cmp_anchor_size_dim_t dim);

/**
 * @brief Frees resources in an anchor function structure.
 * @param func The function structure.
 * @return 0 on success.
 */
int cmp_anchor_funcs_free(cmp_anchor_funcs_t *func);

/**
 * @brief Initializes a position-try property.
 * @param prop The property.
 * @param options Options string.
 * @param order Try order.
 * @return 0 on success.
 */
int cmp_prop_position_try_init(cmp_prop_position_try_t *prop,
                               const char *options,
                               cmp_position_try_order_t order);

/**
 * @brief Frees resources in a position-try property.
 * @param prop The property.
 * @return 0 on success.
 */
int cmp_prop_position_try_free(cmp_prop_position_try_t *prop);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_ANCHOR_POSITIONING_H */