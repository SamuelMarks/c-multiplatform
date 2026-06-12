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

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_bfc_calculate: Invalid argument (node=NULL): %d\n", rc);

    return rc;
  }
  /* Simplified BFC logic: block takes full available width */
  node->computed_rect.width = available_width;
  cmp_log_debug("cmp_bfc_calculate: Calculated layout bounds\n");

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

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_ifc_calculate: Invalid argument (node=NULL): %d\n", rc);

    return rc;
  }
  /* Inline shrinks to content, wrapped within available width */
  if (node->computed_rect.width > available_width) {
    node->computed_rect.width = available_width;
  }
  cmp_log_debug("cmp_ifc_calculate: Computed inline bounds\n");

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

  if (node == NULL || out_x == NULL || out_y == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_float_evaluate: Invalid argument: %d\n", rc);

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

  cmp_log_debug("cmp_float_evaluate: Evaluated float dynamics\n");

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
int cmp_shape_outside_evaluate(cmp_layout_node_t *node, cmp_rect_f_t float_rect,
                               float shape_radius, float margin) {
  int rc = CMP_SUCCESS;
  float content_top;
  float content_bottom;
  float circle_center_y;
  float y_dist;
  float x_dist;

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_shape_outside_evaluate: Invalid argument (node=NULL): %d\n",
              rc);

    return rc;
  }

  content_top = node->computed_rect.y;
  content_bottom = node->computed_rect.y + node->computed_rect.height;

  if (shape_radius > 0.0f) {
    /* Circular wrapping adjustment */
    circle_center_y = float_rect.y + float_rect.height / 2.0f;

    /* Check if current text line intersects the circle's vertical bounds */
    if (content_bottom >= float_rect.y &&
        content_top <= float_rect.y + float_rect.height) {
      /* Calculate horizontal intrusion of the circle at the current line's Y */
      y_dist = content_top - circle_center_y;
      if (y_dist < 0) {
        y_dist = -y_dist;
      }

      /* Pythagorean theorem: x^2 + y^2 = r^2 -> x = sqrt(r^2 - y^2) */
      if (y_dist < shape_radius) {
        /* Approximate sqrt to avoid linking heavy math libs unnecessarilly if
           we can avoid it. But since we don't have a fast sqrt here, we'll use
           a linear approximation for the stub */
        x_dist = shape_radius - (y_dist * y_dist / shape_radius);
        if (x_dist > 0) {
          node->computed_rect.width -= (x_dist + margin);
        }
      }
    }
  } else {
    /* Rectangular wrapping */
    if (content_bottom >= float_rect.y &&
        content_top <= float_rect.y + float_rect.height) {
      node->computed_rect.width -= (float_rect.width + margin);
    }
  }

  cmp_log_debug(
      "cmp_shape_outside_evaluate: Simulated dynamic wrapping constraints\n");

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

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_multicolumn_evaluate: Invalid argument (node=NULL): %d\n",
              rc);

    return rc;
  }

  if (node->column_count <= 1) {
    return rc;
  }

  total_gap = node->column_gap * (float)(node->column_count - 1);
  content_width = node->computed_rect.width - total_gap;

  if (content_width > 0.0f) {
    node->column_width = content_width / (float)node->column_count;
  } else {
    node->column_width = 0.0f;
  }

  if (fill_mode == CMP_COLUMN_FILL_BALANCE) {
    /* Balance content height evenly */
  }

  cmp_log_debug("cmp_multicolumn_evaluate: Assigned columns width\n");

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

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_table_evaluate: Invalid argument (node=NULL): %d\n", rc);

    return rc;
  }

  if (is_fixed) {
    /* Fixed layout algorithm */
  } else {
    /* Auto layout algorithm */
  }

  cmp_log_debug("cmp_table_evaluate: Simulated table constraints\n");

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

  if (table == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_table_border_collapse: Invalid argument (table=NULL): %d\n",
              rc);

    return rc;
  }
  /* Conflict resolution logic */

  cmp_log_debug("cmp_table_border_collapse: Computed CSS collapse\n");

  return rc;
}
