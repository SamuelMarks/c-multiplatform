#ifndef CMP_CSS_GRID_H
#define CMP_CSS_GRID_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_grid.h
 * @brief CSS Grid Layout (Level 1 & 2) implementation.
 */

/**
 * @brief Represents grid-template-columns, grid-template-rows, and
 * grid-template-areas.
 */
typedef struct cmp_prop_grid_template_group {
  char *columns; /**< String representation of grid-template-columns */
  char *rows;    /**< String representation of grid-template-rows */
  char *areas;   /**< String representation of grid-template-areas */
} cmp_prop_grid_template_group_t;

/**
 * @brief Represents grid-auto-flow directions.
 */
typedef enum cmp_grid_auto_flow {
  CMP_GRID_AUTO_FLOW_ROW = 0,
  CMP_GRID_AUTO_FLOW_COLUMN,
  CMP_GRID_AUTO_FLOW_ROW_DENSE,
  CMP_GRID_AUTO_FLOW_COLUMN_DENSE
} cmp_grid_auto_flow_t;

/**
 * @brief Represents grid-auto-columns, grid-auto-rows, and grid-auto-flow.
 */
typedef struct cmp_prop_grid_auto_group {
  char *columns;             /**< String representation of grid-auto-columns */
  char *rows;                /**< String representation of grid-auto-rows */
  cmp_grid_auto_flow_t flow; /**< grid-auto-flow value */
} cmp_prop_grid_auto_group_t;

/**
 * @brief Represents grid line placement properties.
 */
typedef struct cmp_prop_grid_placement_group {
  char *row_start;    /**< grid-row-start */
  char *row_end;      /**< grid-row-end */
  char *column_start; /**< grid-column-start */
  char *column_end;   /**< grid-column-end */
} cmp_prop_grid_placement_group_t;

/**
 * @brief Types of track sizing functions.
 */
typedef enum cmp_grid_track_func_type {
  CMP_GRID_TRACK_FUNC_MINMAX = 0,
  CMP_GRID_TRACK_FUNC_FIT_CONTENT,
  CMP_GRID_TRACK_FUNC_REPEAT
} cmp_grid_track_func_type_t;

/**
 * @brief Represents minmax(), fit-content(), or repeat() functions.
 */
typedef struct cmp_grid_track_funcs {
  cmp_grid_track_func_type_t type;
  char *args; /**< String representation of the arguments */
} cmp_grid_track_funcs_t;

/**
 * @brief Represents the subgrid value for a grid track list.
 */
typedef struct cmp_prop_subgrid {
  int is_subgrid; /**< Non-zero if this is a subgrid */
} cmp_prop_subgrid_t;

/**
 * @brief Represents the masonry layout (Grid Level 3/Experimental).
 */
typedef struct cmp_masonry_layout {
  int is_masonry; /**< Non-zero if masonry is used */
} cmp_masonry_layout_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a grid template group.
 * @param group The template group to initialize.
 * @param columns The columns value (can be NULL).
 * @param rows The rows value (can be NULL).
 * @param areas The areas value (can be NULL).
 * @return 0 on success.
 */
int cmp_prop_grid_template_group_init(cmp_prop_grid_template_group_t *group,
                                      const char *columns, const char *rows,
                                      const char *areas);

/**
 * @brief Frees resources in a grid template group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_grid_template_group_free(cmp_prop_grid_template_group_t *group);

/**
 * @brief Initializes a grid auto group.
 * @param group The auto group to initialize.
 * @param columns The auto columns value (can be NULL).
 * @param rows The auto rows value (can be NULL).
 * @param flow The auto flow value.
 * @return 0 on success.
 */
int cmp_prop_grid_auto_group_init(cmp_prop_grid_auto_group_t *group,
                                  const char *columns, const char *rows,
                                  cmp_grid_auto_flow_t flow);

/**
 * @brief Frees resources in a grid auto group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_grid_auto_group_free(cmp_prop_grid_auto_group_t *group);

/**
 * @brief Initializes a grid placement group.
 * @param group The placement group to initialize.
 * @param rs Row start.
 * @param re Row end.
 * @param cs Column start.
 * @param ce Column end.
 * @return 0 on success.
 */
int cmp_prop_grid_placement_group_init(cmp_prop_grid_placement_group_t *group,
                                       const char *rs, const char *re,
                                       const char *cs, const char *ce);

/**
 * @brief Frees resources in a grid placement group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_grid_placement_group_free(cmp_prop_grid_placement_group_t *group);

/**
 * @brief Initializes a grid track function representation.
 * @param func The function structure.
 * @param type The type of function (minmax, fit-content, repeat).
 * @param args The arguments inside the function.
 * @return 0 on success.
 */
int cmp_grid_track_funcs_init(cmp_grid_track_funcs_t *func,
                              cmp_grid_track_func_type_t type,
                              const char *args);

/**
 * @brief Frees resources in a grid track function.
 * @param func The function to free.
 * @return 0 on success.
 */
int cmp_grid_track_funcs_free(cmp_grid_track_funcs_t *func);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_GRID_H */