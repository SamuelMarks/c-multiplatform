/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_pip {
  int is_active;
  void *active_target;
};

int cmp_pip_create(cmp_pip_t **out_pip) {
  cmp_pip_t *pip;
  if (!out_pip) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_pip_t), (void **)&pip) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(pip, 0, sizeof(cmp_pip_t));
  *out_pip = pip;
  return CMP_SUCCESS;
}

int cmp_pip_destroy(cmp_pip_t *pip) {
  if (!pip) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(pip);
  return CMP_SUCCESS;
}

int cmp_pip_enable(cmp_pip_t *pip, void *ui_node_or_video_stream) {
  if (!pip || !ui_node_or_video_stream) {
    return CMP_ERROR_INVALID_ARG;
  }
  pip->is_active = 1;
  pip->active_target = ui_node_or_video_stream;
  return CMP_SUCCESS;
}

int cmp_pip_disable(cmp_pip_t *pip) {
  if (!pip) {
    return CMP_ERROR_INVALID_ARG;
  }
  pip->is_active = 0;
  pip->active_target = NULL;
  return CMP_SUCCESS;
}

int cmp_pip_is_active(const cmp_pip_t *pip, int *out_is_active) {
  if (!pip || !out_is_active) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_is_active = pip->is_active;
  return CMP_SUCCESS;
}
