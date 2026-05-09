/* clang-format off */
#include "cmp_ui_skeleton.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_skeleton {
  cmp_ui_node_t *node_root;
  float width;
  float height;
  float shimmer_phase;
};

/**
 * @brief Creates a new UI skeleton loading component.
 *
 * @param out_skeleton Pointer to store the newly created skeleton context.
 * @param width The layout width of the skeleton.
 * @param height The layout height of the skeleton.
 * @param base_color The background base color (ARGB).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_create(cmp_ui_skeleton_t **out_skeleton, float width,
                           float height, uint32_t base_color) {
  cmp_ui_skeleton_t *skeleton;
  int rc = CMP_SUCCESS;

  if (!out_skeleton) {
    LOG_DEBUG("cmp_ui_skeleton_create: out_skeleton is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_skeleton_t), (void **)&skeleton);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_skeleton_create: OOM\n");

    return rc;
  }

  skeleton->width = width;
  skeleton->height = height;
  skeleton->shimmer_phase = 0.0f;

  rc = cmp_ui_box_create(&skeleton->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_skeleton_create: cmp_ui_box_create failed\n");
    CMP_FREE(skeleton);

    return rc;
  }

  skeleton->node_root->bg_color = base_color;

  *out_skeleton = skeleton;
  return rc;
}

/**
 * @brief Destroys a UI skeleton component and frees its resources.
 *
 * @param skeleton Pointer to the skeleton context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_destroy(cmp_ui_skeleton_t *skeleton) {
  int rc = CMP_SUCCESS;

  if (!skeleton) {
    LOG_DEBUG("cmp_ui_skeleton_destroy: skeleton is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (skeleton->node_root) {
    rc = cmp_ui_node_destroy(skeleton->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_skeleton_destroy: cmp_ui_node_destroy failed\n");
    }
  }

  rc = CMP_FREE(skeleton);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_skeleton_destroy: CMP_FREE failed\n");

    return rc;
  }

  return rc;
}

/**
 * @brief Retrieves the root UI node of the skeleton component.
 *
 * @param skeleton Pointer to the skeleton context.
 * @param out_node Pointer to store the root UI node pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_get_node(cmp_ui_skeleton_t *skeleton,
                             cmp_ui_node_t **out_node) {
  int rc = CMP_SUCCESS;
  if (!skeleton || !out_node) {
    LOG_DEBUG("cmp_ui_skeleton_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = skeleton->node_root;

  return rc;
}

/**
 * @brief Updates the shimmer animation phase of the skeleton.
 *
 * @param skeleton Pointer to the skeleton context.
 * @param dt_ms Delta time in milliseconds since the last update.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_update(cmp_ui_skeleton_t *skeleton, float dt_ms) {
  int rc = CMP_SUCCESS;
  if (!skeleton) {
    LOG_DEBUG("cmp_ui_skeleton_update: skeleton is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  skeleton->shimmer_phase += (dt_ms / 1000.0f);
  if (skeleton->shimmer_phase >= 1.0f) {
    skeleton->shimmer_phase -= 1.0f;
  }

  return rc;
}
