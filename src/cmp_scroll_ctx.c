/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"
/* clang-format on */

struct cmp_scroll_ctx {
  float scroll_top;
  float scroll_left;
};

/**
 * @brief Create a scroll context.
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_ctx_create(cmp_scroll_ctx_t **out_ctx) {
  int rc = CMP_SUCCESS;
  struct cmp_scroll_ctx *ctx;

  rc = CMP_SUCCESS;

  if (out_ctx == NULL) {
    LOG_DEBUG("Invalid argument: out_ctx is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_scroll_ctx), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(ctx, 0, sizeof(struct cmp_scroll_ctx));

  *out_ctx = (cmp_scroll_ctx_t *)ctx;
  return rc;
}

/**
 * @brief Destroy a scroll context.
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_ctx_destroy(cmp_scroll_ctx_t *ctx) {
  int rc = CMP_SUCCESS;
  struct cmp_scroll_ctx *internal_ctx;

  rc = CMP_SUCCESS;
  internal_ctx = (struct cmp_scroll_ctx *)ctx;

  if (internal_ctx == NULL) {
    LOG_DEBUG("Invalid argument: ctx is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(internal_ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return rc;
}

/**
 * @brief Get the current scroll offset.
 *
 * @param ctx Parameter description.
 * @param out_x Parameter description.
 * @param out_y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_ctx_get_offset(const cmp_scroll_ctx_t *ctx, float *out_x,
                              float *out_y) {
  int rc = CMP_SUCCESS;
  const struct cmp_scroll_ctx *internal_ctx;

  internal_ctx = (const struct cmp_scroll_ctx *)ctx;

  if (internal_ctx == NULL) {
    LOG_DEBUG("Invalid argument: ctx is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (out_x != NULL) {
    *out_x = internal_ctx->scroll_left;
  }

  if (out_y != NULL) {
    *out_y = internal_ctx->scroll_top;
  }

  return rc;
}

/**
 * @brief Set the scroll offset.
 *
 * @param ctx Parameter description.
 * @param x Parameter description.
 * @param y Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_ctx_set_offset(cmp_scroll_ctx_t *ctx, float x, float y) {
  int rc = CMP_SUCCESS;
  struct cmp_scroll_ctx *internal_ctx;

  internal_ctx = (struct cmp_scroll_ctx *)ctx;

  if (internal_ctx == NULL) {
    LOG_DEBUG("Invalid argument: ctx is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  internal_ctx->scroll_left = x;
  internal_ctx->scroll_top = y;

  return rc;
}

/**
 * @brief Inject scroll offset directly to the GPU via UBO.
 *
 * @param ctx Parameter description.
 * @param ubo Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_scroll_ctx_inject_to_gpu(const cmp_scroll_ctx_t *ctx, cmp_ubo_t *ubo) {
  int rc = CMP_SUCCESS;
  const struct cmp_scroll_ctx *internal_ctx;
  float data[2];

  rc = CMP_SUCCESS;
  internal_ctx = (const struct cmp_scroll_ctx *)ctx;

  if (internal_ctx == NULL || ubo == NULL) {
    LOG_DEBUG("Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  data[0] = internal_ctx->scroll_left;
  data[1] = internal_ctx->scroll_top;

  rc = cmp_ubo_update(ubo, data, sizeof(data));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to update UBO\n");
    return rc;
  }

  return rc;
}
