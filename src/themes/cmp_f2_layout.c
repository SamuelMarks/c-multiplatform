/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_layout.h"
#include <stddef.h>
/* clang-format on */

CMP_API int cmp_f2_stack_create(cmp_ui_node_t **out_node, int horizontal,
                        float spacing_token) {
  int res;
  if (!out_node) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    return res;
  }

  (*out_node)->layout->direction = horizontal ? CMP_FLEX_ROW : CMP_FLEX_COLUMN;
  (*out_node)->layout->flex_wrap = CMP_FLEX_NOWRAP;

  /* Use column_gap as a generic spacing applicator */
  (*out_node)->layout->column_gap = spacing_token;

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_wrap_create(cmp_ui_node_t **out_node, float horizontal_gap,
                       float vertical_gap) {
  int res;
  if (!out_node) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_box_create(out_node);
  if (res != CMP_SUCCESS) {
    return res;
  }

  (*out_node)->layout->direction = CMP_FLEX_ROW;
  (*out_node)->layout->flex_wrap = CMP_FLEX_WRAP;

  /* Use column_gap as a generic spacing applicator */
  (*out_node)->layout->column_gap = horizontal_gap;

  /* If there were a row_gap on layout node, we would set it to vertical_gap */
  (void)vertical_gap;

  return CMP_SUCCESS;
}

CMP_API int cmp_f2_grid_create(cmp_ui_node_t **out_node, int columns, float column_gap,
                       float row_gap) {
  int res;
  if (!out_node) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = cmp_ui_grid_view_create(out_node, columns);
  if (res != CMP_SUCCESS) {
    return res;
  }

  (*out_node)->layout->column_gap = column_gap;
  (void)row_gap;

  return CMP_SUCCESS;
}
