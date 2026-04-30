/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_overdraw {
  int is_active;
};

/**
 * @brief Create an overdraw context.
 *
 * @param out_overdraw Pointer to store the created context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_overdraw_create(cmp_overdraw_t **out_overdraw) {
  int rc = CMP_SUCCESS;
  struct cmp_overdraw *ctx;

  rc = CMP_SUCCESS;
  ctx = NULL;

  if (out_overdraw == NULL) {
    LOG_DEBUG("Error in cmp_overdraw_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_overdraw), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_overdraw_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  memset(ctx, 0, sizeof(struct cmp_overdraw));
  ctx->is_active = 0;

  *out_overdraw = (cmp_overdraw_t *)ctx;
  return rc;
}

/**
 * @brief Destroy an overdraw context.
 *
 * @param overdraw The context to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_overdraw_destroy(cmp_overdraw_t *overdraw) {
  int rc = CMP_SUCCESS;
  struct cmp_overdraw *ctx;

  rc = CMP_SUCCESS;

  if (overdraw == NULL) {
    LOG_DEBUG("Error in cmp_overdraw_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_overdraw *)overdraw;

  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_overdraw_destroy: CMP_FREE failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief Set the overdraw visualization enabled state.
 *
 * @param overdraw The overdraw context.
 * @param renderer The renderer context.
 * @param enable 1 to enable overdraw visualization, 0 to disable.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_overdraw_set_enabled(cmp_overdraw_t *overdraw, cmp_renderer_t *renderer,
                             int enable) {
  int rc = CMP_SUCCESS;
  struct cmp_overdraw *ctx;

  if (overdraw == NULL || renderer == NULL) {
    LOG_DEBUG("Error in cmp_overdraw_set_enabled: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  ctx = (struct cmp_overdraw *)overdraw;

  /* In a real implementation this would swap the active fragment shader
     on the renderer to an additive blending heat-map shader */
  ctx->is_active = enable ? 1 : 0;
  return rc;
}
