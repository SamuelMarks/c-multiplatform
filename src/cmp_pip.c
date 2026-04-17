/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_pip {
  int is_active;
  void *active_target;
};

int cmp_pip_create(cmp_pip_t **out_pip) {
  int rc = CMP_SUCCESS;
  cmp_pip_t *pip = NULL;

  if (!out_pip) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pip_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_pip_t), (void **)&pip);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_pip_create: Out of memory\n");
    return rc;
  }

  memset(pip, 0, sizeof(cmp_pip_t));
  *out_pip = pip;
  return rc;
}

int cmp_pip_destroy(cmp_pip_t *pip) {
  int rc = CMP_SUCCESS;

  if (!pip) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pip_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(pip);
  return rc;
}

int cmp_pip_enable(cmp_pip_t *pip, void *ui_node_or_video_stream) {
  int rc = CMP_SUCCESS;

  if (!pip || !ui_node_or_video_stream) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pip_enable: Invalid argument\n");
    return rc;
  }

  pip->is_active = 1;
  pip->active_target = ui_node_or_video_stream;
  return rc;
}

int cmp_pip_disable(cmp_pip_t *pip) {
  int rc = CMP_SUCCESS;

  if (!pip) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pip_disable: Invalid argument\n");
    return rc;
  }

  pip->is_active = 0;
  pip->active_target = NULL;
  return rc;
}

int cmp_pip_is_active(const cmp_pip_t *pip, int *out_is_active) {
  int rc = CMP_SUCCESS;

  if (!pip || !out_is_active) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pip_is_active: Invalid argument\n");
    return rc;
  }

  *out_is_active = pip->is_active;
  return rc;
}
