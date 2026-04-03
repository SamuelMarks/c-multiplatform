#ifndef CMP_F2_LAYOUT_H
#define CMP_F2_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

/**
 * @brief Initialize a Fluent 2 Stack Layout component.
 *        Stacks items horizontally or vertically with uniform spacing tokens.
 * @param out_node Pointer to receive the allocated node.
 * @param horizontal Non-zero for horizontal stacking (row), zero for vertical
 * (column).
 * @param spacing_token The gap between items.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 stack_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_stack_create(cmp_ui_node_t **out_node, int horizontal,
                                float spacing_token);

/**
 * @brief Initialize a Fluent 2 Wrap Layout component.
 *        Flows items horizontally and wraps to the next line with specified gap
 * tokens.
 * @param out_node Pointer to receive the allocated node.
 * @param horizontal_gap The gap between items on the same line.
 * @param vertical_gap The gap between lines.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 wrap_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_wrap_create(cmp_ui_node_t **out_node, float horizontal_gap,
                               float vertical_gap);

/**
 * @brief Initialize a Fluent 2 Grid Layout component.
 *        2D grid component with definable row/column tracks, gaps, and
 * alignments.
 * @param out_node Pointer to receive the allocated node.
 * @param columns Number of columns in the grid.
 * @param column_gap Spacing between columns.
 * @param row_gap Spacing between rows.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 grid_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_grid_create(cmp_ui_node_t **out_node, int columns,
                               float column_gap, float row_gap);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_LAYOUT_H */
