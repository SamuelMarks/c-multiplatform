/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief cmp_bfc_calculate
 *
 * @param node Parameter description.
 * @param available_width Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_bfc_calculate(cmp_layout_node_t *node, float available_width) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_bfc_calculate: Invalid argument (node=NULL)\n");
    return rc;
  }
  /* Simplified BFC logic: block takes full available width */
  node->computed_rect.width = available_width;
  return rc;
}

/**
 * @brief cmp_ifc_calculate
 *
 * @param node Parameter description.
 * @param available_width Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ifc_calculate(cmp_layout_node_t *node, float available_width) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_ifc_calculate: Invalid argument (node=NULL)\n");
    return rc;
  }
  /* Inline shrinks to content, wrapped within available width */
  if (node->computed_rect.width > available_width) {
    node->computed_rect.width = available_width;
  }
  return rc;
}

/**
 * @brief cmp_float_evaluate
 *
 * @param node Parameter description.
 * @param is_float Parameter description.
 * @param clear Parameter description.
 * @param out_x Parameter description.
 * @param out_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_float_evaluate(cmp_layout_node_t *node, int is_float, int clear,
                       float *out_x, float *out_y) {
  int rc = CMP_SUCCESS;

  if (!node || !out_x || !out_y) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_float_evaluate: Invalid argument\n");
    return rc;
  }

  if (is_float) {
    /* Basic float displacement */
    *out_x = node->computed_rect.width;
    *out_y = 0.0f;
  } else if (clear) {
    /* Drop below previous floats */
    *out_x = 0.0f;
    *out_y += node->computed_rect.height;
  }
  return rc;
}

/**
 * @brief cmp_shape_outside_evaluate
 *
 * @param node Parameter description.
 * @param float_rect Parameter description.
 * @param shape_radius Parameter description.
 * @param margin Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_shape_outside_evaluate(cmp_layout_node_t *node, cmp_rect_t float_rect,
                               float shape_radius, float margin) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_shape_outside_evaluate: Invalid argument (node=NULL)\n");
    return rc;
  }
  /* Wrap content logic placeholder */
  if (shape_radius > 0) {
    /* Circular wrapping adjustment */
    node->computed_rect.width -= (shape_radius + margin);
  } else {
    /* Rectangular wrapping */
    node->computed_rect.width -= (float_rect.width + margin);
  }
  return rc;
}

/**
 * @brief cmp_multicolumn_evaluate
 *
 * @param node Parameter description.
 * @param fill_mode Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_multicolumn_evaluate(cmp_layout_node_t *node,
                             cmp_column_fill_t fill_mode) {
  int rc = CMP_SUCCESS;
  float total_gap;
  float content_width;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_multicolumn_evaluate: Invalid argument (node=NULL)\n");
    return rc;
  }

  if (node->column_count <= 1) {
    return rc;
  }

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

  return rc;
}

/**
 * @brief cmp_table_evaluate
 *
 * @param node Parameter description.
 * @param is_fixed Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_table_evaluate(cmp_layout_node_t *node, int is_fixed) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_table_evaluate: Invalid argument (node=NULL)\n");
    return rc;
  }
  if (is_fixed) {
    /* Fixed layout algorithm */
  } else {
    /* Auto layout algorithm */
  }
  return rc;
}

/**
 * @brief cmp_table_border_collapse
 *
 * @param table Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_table_border_collapse(cmp_layout_node_t *table) {
  int rc = CMP_SUCCESS;

  if (!table) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_table_border_collapse: Invalid argument (table=NULL)\n");
    return rc;
  }
  /* Conflict resolution logic */
  return rc;
}
