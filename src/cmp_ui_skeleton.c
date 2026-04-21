/* clang-format off */
#include "cmp_ui_skeleton.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_ui_skeleton {
  cmp_ui_node_t *node_root;
  float width;
  float height;
  float shimmer_phase;
};

int cmp_ui_skeleton_create(cmp_ui_skeleton_t **out_skeleton, float width,
                           float height, uint32_t base_color) {
  cmp_ui_skeleton_t *skeleton;
  int err;

  if (!out_skeleton) {
    return CMP_ERROR_INVALID_ARG;
  }

  skeleton = (cmp_ui_skeleton_t *)malloc(sizeof(cmp_ui_skeleton_t));
  if (!skeleton) {
    return CMP_ERROR_OOM;
  }

  skeleton->width = width;
  skeleton->height = height;
  skeleton->shimmer_phase = 0.0f;

  err = cmp_ui_box_create(&skeleton->node_root);
  if (err != 0) {
    free(skeleton);
    return err;
  }

  skeleton->node_root->bg_color = base_color;

  *out_skeleton = skeleton;
  return 0;
}

int cmp_ui_skeleton_destroy(cmp_ui_skeleton_t *skeleton) {
  if (!skeleton) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(skeleton);
  return 0;
}

int cmp_ui_skeleton_get_node(cmp_ui_skeleton_t *skeleton,
                             cmp_ui_node_t **out_node) {
  if (!skeleton || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = skeleton->node_root;
  return 0;
}

int cmp_ui_skeleton_update(cmp_ui_skeleton_t *skeleton, float dt_ms) {
  if (!skeleton) {
    return CMP_ERROR_INVALID_ARG;
  }

  skeleton->shimmer_phase += (dt_ms / 1000.0f);
  if (skeleton->shimmer_phase >= 1.0f) {
    skeleton->shimmer_phase -= 1.0f;
  }

  return 0;
}
