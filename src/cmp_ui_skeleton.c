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
 * @brief cmp_ui_skeleton_create
 *
 * @param out_skeleton Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @param base_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_create(cmp_ui_skeleton_t **out_skeleton, float width,
                           float height, uint32_t base_color) {
  cmp_ui_skeleton_t *skeleton;
  int rc;

  if (!out_skeleton) {
    LOG_DEBUG("cmp_ui_skeleton_create: out_skeleton is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_skeleton_t), (void **)&skeleton);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_skeleton_create: OOM\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  skeleton->width = width;
  skeleton->height = height;
  skeleton->shimmer_phase = 0.0f;

  rc = cmp_ui_box_create(&skeleton->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_skeleton_create: cmp_ui_box_create failed\n");
    CMP_FREE(skeleton);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  skeleton->node_root->bg_color = base_color;

  *out_skeleton = skeleton;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_skeleton_destroy
 *
 * @param skeleton Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_destroy(cmp_ui_skeleton_t *skeleton) {
  int rc;

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
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_skeleton_get_node
 *
 * @param skeleton Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_get_node(cmp_ui_skeleton_t *skeleton,
                             cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  if (!skeleton || !out_node) {
    LOG_DEBUG("cmp_ui_skeleton_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = skeleton->node_root;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_skeleton_update
 *
 * @param skeleton Parameter description.
 * @param dt_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_skeleton_update(cmp_ui_skeleton_t *skeleton, float dt_ms) {
  int rc;
  rc = 0;
  if (!skeleton) {
    LOG_DEBUG("cmp_ui_skeleton_update: skeleton is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  skeleton->shimmer_phase += (dt_ms / 1000.0f);
  if (skeleton->shimmer_phase >= 1.0f) {
    skeleton->shimmer_phase -= 1.0f;
  }

  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}