/* clang-format off */
#include "cmp_ui_spinner.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_spinner {
  cmp_ui_node_t *node_root;
  float rotation_angle;
  float size;
};

/**
 * @brief cmp_ui_spinner_create
 *
 * @param out_spinner Parameter description.
 * @param size Parameter description.
 * @param color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_spinner_create(cmp_ui_spinner_t **out_spinner, float size,
                          uint32_t color) {
  cmp_ui_spinner_t *spinner;
  int rc = CMP_SUCCESS;

  if (!out_spinner) {
    LOG_DEBUG("cmp_ui_spinner_create: out_spinner is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_spinner_t), (void **)&spinner);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_spinner_create: OOM\n");

    return rc;
  }

  spinner->rotation_angle = 0.0f;
  spinner->size = size;

  rc = cmp_ui_box_create(&spinner->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_spinner_create: cmp_ui_box_create failed\n");
    if (CMP_FREE(spinner) != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_spinner_create: CMP_FREE failed\n");
    }

    return rc;
  }

  spinner->node_root->bg_color = color;

  *out_spinner = spinner;
  return rc;
}

/**
 * @brief cmp_ui_spinner_destroy
 *
 * @param spinner Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_spinner_destroy(cmp_ui_spinner_t *spinner) {
  int rc = CMP_SUCCESS;

  if (!spinner) {
    LOG_DEBUG("cmp_ui_spinner_destroy: spinner is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (spinner->node_root) {
    rc = cmp_ui_node_destroy(spinner->node_root);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_ui_spinner_destroy: cmp_ui_node_destroy failed\n");
  }

  rc = CMP_FREE(spinner);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_spinner_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_spinner_get_node
 *
 * @param spinner Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_spinner_get_node(cmp_ui_spinner_t *spinner,
                            cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!spinner || !out_node) {
    LOG_DEBUG("cmp_ui_spinner_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = spinner->node_root;

  return rc;
}

/**
 * @brief cmp_ui_spinner_update
 *
 * @param spinner Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_spinner_update(cmp_ui_spinner_t *spinner, float dt_ms) {
  int rc = CMP_SUCCESS;
  if (!spinner) {
    LOG_DEBUG("cmp_ui_spinner_update: spinner is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  spinner->rotation_angle += (dt_ms / 1000.0f) * 360.0f;
  if (spinner->rotation_angle >= 360.0f) {
    spinner->rotation_angle -= 360.0f;
  }

  return rc;
}