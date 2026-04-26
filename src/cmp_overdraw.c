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
 * @brief cmp_overdraw_create
 *
 * @param out_overdraw Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_overdraw_create(cmp_overdraw_t **out_overdraw) {
  int rc = CMP_SUCCESS;
  struct cmp_overdraw *ctx = NULL;

  if (!out_overdraw) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_overdraw_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_overdraw), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_overdraw_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_overdraw));
  ctx->is_active = 0;

  *out_overdraw = (cmp_overdraw_t *)ctx;
  return rc;
}

/**
 * @brief cmp_overdraw_destroy
 *
 * @param overdraw Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_overdraw_destroy(cmp_overdraw_t *overdraw) {
  int rc = CMP_SUCCESS;
  struct cmp_overdraw *ctx = (struct cmp_overdraw *)overdraw;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_overdraw_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_overdraw_set_enabled
 *
 * @param overdraw Parameter description.
 * @param renderer Parameter description.
 * @param enable Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_overdraw_set_enabled(cmp_overdraw_t *overdraw, cmp_renderer_t *renderer,
                             int enable) {
  int rc = CMP_SUCCESS;
  struct cmp_overdraw *ctx = (struct cmp_overdraw *)overdraw;

  if (!ctx || !renderer) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_overdraw_set_enabled: Invalid argument\n");
    return rc;
  }

  /* In a real implementation this would swap the active fragment shader
     on the renderer to an additive blending heat-map shader */
  ctx->is_active = enable ? 1 : 0;

  return rc;
}
