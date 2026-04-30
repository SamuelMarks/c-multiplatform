/* clang-format off */
#include "cmp_ui_progress_indicator.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_progress_indicator {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t
      *node_fill; /* Represents either the line fill or the circular arc */
  cmp_ui_progress_indicator_type_t current_type;
  float progress;
};

/**
 * @brief cmp_ui_progress_indicator_create
 *
 * @param out_indicator Parameter description.
 * @param type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_indicator_create(
    cmp_ui_progress_indicator_t **out_indicator,
    cmp_ui_progress_indicator_type_t type) {
  int rc = CMP_SUCCESS;
  cmp_ui_progress_indicator_t *ind;
  int err;

  if (!out_indicator) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_progress_indicator_t), (void **)&ind);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(ind, 0, sizeof(cmp_ui_progress_indicator_t));

  ind->current_type = type;
  ind->progress = 0.0f;

  err = cmp_ui_box_create(&ind->node_root);
  if (err != CMP_SUCCESS) {
    int free_rc = CMP_FREE(ind);
    if (free_rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_progress_indicator_create: CMP_FREE ind failed\n");
    return err;
  }

  err = cmp_ui_box_create(&ind->node_fill);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(ind->node_root);
    int free_rc = CMP_FREE(ind);
    if (free_rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_progress_indicator_create: CMP_FREE ind failed\n");
    return err;
  }

  err = cmp_ui_node_add_child(ind->node_root, ind->node_fill);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG(
        "cmp_ui_progress_indicator_create: cmp_ui_node_add_child failed\n");
  }

  err = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&ind->node_root->layout);
  if (err == CMP_SUCCESS) {
    memset(ind->node_root->layout, 0, sizeof(cmp_layout_node_t));
    ind->node_root->layout->id = 1;
  }

  /* Configure root and fill style properties */
  ind->node_root->bg_color = 0xFFE0E0E0; /* Default track color */
  ind->node_fill->bg_color = 0xFF2196F3; /* Default fill color */

  /* In a real scenario, this would apply CSS transformations to achieve
   * morphing. */

  *out_indicator = ind;

  return rc;
}

/**
 * @brief cmp_ui_progress_indicator_destroy
 *
 * @param indicator Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_indicator_destroy(cmp_ui_progress_indicator_t *indicator) {
  int rc = CMP_SUCCESS;
  if (!indicator) {
    return CMP_ERROR_INVALID_ARG;
  }
  int err = CMP_FREE(indicator);
  if (err != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_progress_indicator_destroy: CMP_FREE failed\n");
  }

  rc = err;
  return rc;
}

/**
 * @brief cmp_ui_progress_indicator_get_node
 *
 * @param indicator Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_indicator_get_node(cmp_ui_progress_indicator_t *indicator,
                                       cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!indicator || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = indicator->node_root;

  return rc;
}

/**
 * @brief cmp_ui_progress_indicator_set_progress
 *
 * @param indicator Parameter description.
 * @param progress Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_indicator_set_progress(
    cmp_ui_progress_indicator_t *indicator, float progress) {
  int rc = CMP_SUCCESS;
  if (!indicator) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (progress < 0.0f)
    progress = 0.0f;
  if (progress > 1.0f)
    progress = 1.0f;

  indicator->progress = progress;

  /* Here we would update the layout or vector properties (width% for linear,
   * dashoffset for circular) */

  return rc;
}

/**
 * @brief cmp_ui_progress_indicator_set_type
 *
 * @param indicator Parameter description.
 * @param new_type Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_indicator_set_type(
    cmp_ui_progress_indicator_t *indicator,
    cmp_ui_progress_indicator_type_t new_type) {
  int rc = CMP_SUCCESS;
  if (!indicator) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (indicator->current_type != new_type) {
    indicator->current_type = new_type;
    /* Trigger a CSS view-transition or layout update to visually morph the
     * shapes */
  }

  return rc;
}
/**
 * @brief cmp_ui_progress_indicator_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_progress_indicator_bind_a11y(cmp_ui_progress_indicator_t *widget,
                                        cmp_a11y_tree_t *tree) {
  int rc = CMP_SUCCESS;
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  int err = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id,
                                   "progressbar", "Progress Indicator");
  if (err != CMP_SUCCESS) {
    LOG_DEBUG(
        "cmp_ui_progress_indicator_bind_a11y: cmp_a11y_tree_add_node failed\n");
  }

  rc = err;
  return rc;
}
