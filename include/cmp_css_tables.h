/* clang-format off */
#ifndef CMP_CSS_TABLES_H
#define CMP_CSS_TABLES_H

#include "cmp_css_values.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents the table-layout property.
 */
typedef enum cmp_prop_table_layout {
  CMP_TABLE_LAYOUT_AUTO = 0,
  CMP_TABLE_LAYOUT_FIXED
} cmp_prop_table_layout_t;

/**
 * @brief Represents the border-collapse property.
 */
typedef enum cmp_prop_table_border_collapse {
  CMP_TABLE_BORDER_COLLAPSE_SEPARATE = 0,
  CMP_TABLE_BORDER_COLLAPSE_COLLAPSE
} cmp_prop_table_border_collapse_t;

/**
 * @brief Type of a border-spacing value.
 */
typedef enum cmp_prop_table_border_spacing_type {
  CMP_TABLE_BORDER_SPACING_LENGTH_PHYS = 0,
  CMP_TABLE_BORDER_SPACING_LENGTH_REL
} cmp_prop_table_border_spacing_type_t;

/**
 * @brief Represents a single border-spacing value.
 */
typedef struct cmp_prop_table_border_spacing_val {
  cmp_prop_table_border_spacing_type_t type; /**< The type of the length */
  union {
    cmp_length_phys_t phys; /**< Physical length */
    cmp_length_rel_t rel;   /**< Relative length */
  } value;                  /**< The value of the length */
} cmp_prop_table_border_spacing_val_t;

/**
 * @brief Represents the border-spacing property.
 */
typedef struct cmp_prop_table_border_spacing {
  cmp_prop_table_border_spacing_val_t horizontal; /**< Horizontal spacing */
  cmp_prop_table_border_spacing_val_t vertical;   /**< Vertical spacing */
} cmp_prop_table_border_spacing_t;

/**
 * @brief Represents table border properties.
 */
typedef struct cmp_prop_table_border {
  cmp_prop_table_border_collapse_t collapse; /**< border-collapse */
  cmp_prop_table_border_spacing_t spacing;   /**< border-spacing */
} cmp_prop_table_border_t;

/**
 * @brief Represents the caption-side property.
 */
typedef enum cmp_prop_caption_side {
  CMP_CAPTION_SIDE_TOP = 0,
  CMP_CAPTION_SIDE_BOTTOM,
  CMP_CAPTION_SIDE_BLOCK_START,
  CMP_CAPTION_SIDE_BLOCK_END,
  CMP_CAPTION_SIDE_INLINE_START,
  CMP_CAPTION_SIDE_INLINE_END
} cmp_prop_caption_side_t;

/**
 * @brief Represents the empty-cells property.
 */
typedef enum cmp_prop_empty_cells {
  CMP_EMPTY_CELLS_SHOW = 0,
  CMP_EMPTY_CELLS_HIDE
} cmp_prop_empty_cells_t;

/**
 * @brief Represents miscellaneous table properties.
 */
typedef struct cmp_prop_table_misc {
  cmp_prop_caption_side_t caption_side; /**< caption-side */
  cmp_prop_empty_cells_t empty_cells;   /**< empty-cells */
} cmp_prop_table_misc_t;

/**
 * @brief Initialize a single border-spacing value with physical length.
 *
 * @param val The value to initialize.
 * @param len The physical length value.
 * @param unit The physical length unit.
 * @return 0 on success, non-zero on failure.
 */
int cmp_prop_table_border_spacing_val_init_phys(
    cmp_prop_table_border_spacing_val_t *val, float len,
    cmp_length_phys_unit_t unit);

/**
 * @brief Initialize a single border-spacing value with relative length.
 *
 * @param val The value to initialize.
 * @param len The relative length value.
 * @param unit The relative length unit.
 * @return 0 on success, non-zero on failure.
 */
int cmp_prop_table_border_spacing_val_init_rel(
    cmp_prop_table_border_spacing_val_t *val, float len,
    cmp_length_rel_unit_t unit);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_TABLES_H */
/* clang-format on */