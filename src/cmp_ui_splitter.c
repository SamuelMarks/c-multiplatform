/* clang-format off */
#include "cmp_ui_splitter.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_splitter {
  cmp_ui_node_t *node_root;
  int is_vertical;
  float position;
};

/**
 * @brief cmp_ui_splitter_create
 *
 * @param out_splitter Parameter description.
 * @param is_vertical Parameter description.
 * @param color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_splitter_create(cmp_ui_splitter_t **out_splitter, int is_vertical,
                           uint32_t color) {
  cmp_ui_splitter_t *splitter;
  int rc = CMP_SUCCESS;

  if (!out_splitter) {
    LOG_DEBUG("cmp_ui_splitter_create: out_splitter is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_splitter_t), (void **)&splitter);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_splitter_create: OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(splitter, 0, sizeof(cmp_ui_splitter_t));

  splitter->is_vertical = is_vertical;
  splitter->position = CMP_MATH_HALF;

  rc = cmp_ui_box_create(&splitter->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_splitter_create: cmp_ui_box_create failed\n");
    CMP_FREE(splitter);
    return CMP_ERROR_GENERAL;
  }

  splitter->node_root->bg_color = color;

  *out_splitter = splitter;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_splitter_destroy
 *
 * @param splitter Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_splitter_destroy(cmp_ui_splitter_t *splitter) {
  int rc = CMP_SUCCESS;

  if (!splitter) {
    LOG_DEBUG("cmp_ui_splitter_destroy: splitter is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (splitter->node_root) {
    rc = cmp_ui_node_destroy(splitter->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_splitter_destroy: cmp_ui_node_destroy failed\n");
  }

  rc = CMP_FREE(splitter);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_splitter_destroy: CMP_FREE failed\n");

    return rc;
  }

  return rc;
}

/**
 * @brief cmp_ui_splitter_get_node
 *
 * @param splitter Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_splitter_get_node(cmp_ui_splitter_t *splitter,
                             cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!splitter || !out_node) {
    LOG_DEBUG("cmp_ui_splitter_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = splitter->node_root;

  return rc;
}

/**
 * @brief cmp_ui_splitter_set_position
 *
 * @param splitter Parameter description.
 * @param position Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_splitter_set_position(cmp_ui_splitter_t *splitter, float position) {
  int rc = CMP_SUCCESS;
  if (!splitter) {
    LOG_DEBUG("cmp_ui_splitter_set_position: splitter is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (position < CMP_MATH_ZERO) {
    position = CMP_MATH_ZERO;
  } else if (position > CMP_MATH_ONE) {
    position = CMP_MATH_ONE;
  }

  splitter->position = position;

  return rc;
}
