#ifndef CMP_CSS_POSITIONING_H
#define CMP_CSS_POSITIONING_H

/* clang-format off */
#include "cmp_css_box_model.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_positioning.h
 * @brief CSS Positioning & Z-Index (Level 3) implementation.
 */

/**
 * @brief Represents the CSS position property.
 */
typedef enum cmp_prop_position {
  CMP_POSITION_STATIC = 0,
  CMP_POSITION_RELATIVE,
  CMP_POSITION_ABSOLUTE,
  CMP_POSITION_FIXED,
  CMP_POSITION_STICKY
} cmp_prop_position_t;

/**
 * @brief Represents the CSS z-index property type.
 */
typedef enum cmp_prop_z_index_type {
  CMP_Z_INDEX_AUTO = 0,
  CMP_Z_INDEX_INTEGER
} cmp_prop_z_index_type_t;

/**
 * @brief Represents the CSS z-index property.
 */
typedef struct cmp_prop_z_index {
  cmp_prop_z_index_type_t type;
  int value; /**< Integer value if type is CMP_Z_INDEX_INTEGER */
} cmp_prop_z_index_t;

/**
 * @brief Represents the CSS isolation property.
 */
typedef enum cmp_prop_isolation {
  CMP_ISOLATION_AUTO = 0,
  CMP_ISOLATION_ISOLATE
} cmp_prop_isolation_t;

/**
 * @brief Type of an inset property (top, right, bottom, left).
 */
typedef enum cmp_prop_inset_type {
  CMP_PROP_INSET_AUTO = 0,
  CMP_PROP_INSET_LENGTH_PHYS,
  CMP_PROP_INSET_LENGTH_REL,
  CMP_PROP_INSET_PERCENT
} cmp_prop_inset_type_t;

/**
 * @brief Represents a single inset property (e.g., top).
 */
typedef struct cmp_prop_inset {
  cmp_prop_inset_type_t type; /**< Type of the inset */
  union {
    cmp_length_phys_t phys;    /**< Physical length */
    cmp_length_rel_t rel;      /**< Relative length */
    cmp_val_percent_t percent; /**< Percentage */
  } value;                     /**< The value of the inset */
} cmp_prop_inset_t;

/**
 * @brief Group of inset properties (top, right, bottom, left).
 */
typedef struct cmp_prop_inset_group {
  cmp_prop_inset_t top;
  cmp_prop_inset_t right;
  cmp_prop_inset_t bottom;
  cmp_prop_inset_t left;
} cmp_prop_inset_group_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a z-index property to auto.
 * @param z_index The property to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_z_index_init_auto(cmp_prop_z_index_t *z_index);

/**
 * @brief Initializes a z-index property to an integer value.
 * @param z_index The property to initialize.
 * @param val The integer value.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_z_index_init_integer(cmp_prop_z_index_t *z_index, int val);

/**
 * @brief Initializes an inset property to auto.
 * @param inset The inset property to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_inset_init_auto(cmp_prop_inset_t *inset);

/**
 * @brief Initializes an inset property to a physical length.
 * @param inset The inset property to initialize.
 * @param val The length value.
 * @param unit The physical unit.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_inset_init_phys(cmp_prop_inset_t *inset, float val,
                             cmp_length_phys_unit_t unit);

/**
 * @brief Initializes an inset property to a percentage.
 * @param inset The inset property to initialize.
 * @param val The percentage value.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_inset_init_percent(cmp_prop_inset_t *inset, float val);

/**
 * @brief Initializes an inset group to all auto values.
 * @param group The group to initialize.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_inset_group_init_auto(cmp_prop_inset_group_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_POSITIONING_H */