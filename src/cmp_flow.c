/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_bfc_calculate(cmp_layout_node_t *node, float available_width) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Simplified BFC logic: block takes full available width */
  node->computed_rect.width = available_width;
  return CMP_SUCCESS;
}

int cmp_ifc_calculate(cmp_layout_node_t *node, float available_width) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Inline shrinks to content, wrapped within available width */
  if (node->computed_rect.width > available_width) {
    node->computed_rect.width = available_width;
  }
  return CMP_SUCCESS;
}

int cmp_float_evaluate(cmp_layout_node_t *node, int is_float, int clear,
                       float *out_x, float *out_y) {
  if (!node || !out_x || !out_y)
    return CMP_ERROR_INVALID_ARG;

  if (is_float) {
    /* Basic float displacement */
    *out_x = node->computed_rect.width;
    *out_y = 0.0f;
  } else if (clear) {
    /* Drop below previous floats */
    *out_x = 0.0f;
    *out_y += node->computed_rect.height;
  }
  return CMP_SUCCESS;
}

int cmp_shape_outside_evaluate(cmp_layout_node_t *node, cmp_rect_t float_rect,
                               float shape_radius, float margin) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Wrap content logic placeholder */
  if (shape_radius > 0) {
    /* Circular wrapping adjustment */
    node->computed_rect.width -= (shape_radius + margin);
  } else {
    /* Rectangular wrapping */
    node->computed_rect.width -= (float_rect.width + margin);
  }
  return CMP_SUCCESS;
}

int cmp_multicolumn_evaluate(cmp_layout_node_t *node,
                             cmp_column_fill_t fill_mode) {
  float total_gap;
  float content_width;
  if (!node)
    return CMP_ERROR_INVALID_ARG;

  if (node->column_count <= 1)
    return CMP_SUCCESS;

  total_gap = node->column_gap * (node->column_count - 1);
  content_width = node->computed_rect.width - total_gap;

  if (content_width > 0) {
    node->column_width = content_width / node->column_count;
  } else {
    node->column_width = 0.0f;
  }

  if (fill_mode == CMP_COLUMN_FILL_BALANCE) {
    /* Balance content height evenly */
  }

  return CMP_SUCCESS;
}

int cmp_table_evaluate(cmp_layout_node_t *node, int is_fixed) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  if (is_fixed) {
    /* Fixed layout algorithm */
  } else {
    /* Auto layout algorithm */
  }
  return CMP_SUCCESS;
}

int cmp_table_border_collapse(cmp_layout_node_t *table) {
  if (!table)
    return CMP_ERROR_INVALID_ARG;
  /* Conflict resolution logic */
  return CMP_SUCCESS;
}
