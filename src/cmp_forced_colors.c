/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

struct cmp_forced_colors {
  int active;
};

/**
 * @brief cmp_forced_colors_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_create(cmp_forced_colors_t **out_ctx) {
  int rc = CMP_SUCCESS;
  cmp_forced_colors_t *ctx = NULL;

  if (!out_ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_forced_colors_create: Invalid argument (out_ctx=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_forced_colors_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_forced_colors_create: Out of memory\n");
    return rc;
  }

  ctx->active = 0;
  *out_ctx = ctx;
  return rc;
}

/**
 * @brief cmp_forced_colors_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_destroy(cmp_forced_colors_t *ctx) {
  int rc = CMP_SUCCESS;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_forced_colors_destroy: Invalid argument (ctx=NULL)\n");
    return rc;
  }
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_forced_colors_set
 *
 * @param ctx Parameter description.
 * @param active Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_set(cmp_forced_colors_t *ctx, int active) {
  int rc = CMP_SUCCESS;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_forced_colors_set: Invalid argument (ctx=NULL)\n");
    return rc;
  }
  ctx->active = active ? 1 : 0;
  return rc;
}

/**
 * @brief cmp_forced_colors_strip_background
 *
 * @param ctx Parameter description.
 * @param out_strip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_strip_background(const cmp_forced_colors_t *ctx,
                                       int *out_strip) {
  int rc = CMP_SUCCESS;

  if (!ctx || !out_strip) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_forced_colors_strip_background: Invalid argument\n");
    return rc;
  }

  *out_strip = ctx->active;
  return rc;
}

/**
 * @brief cmp_forced_colors_strip_box_shadow
 *
 * @param ctx Parameter description.
 * @param out_strip Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_forced_colors_strip_box_shadow(const cmp_forced_colors_t *ctx,
                                       int *out_strip) {
  int rc = CMP_SUCCESS;

  if (!ctx || !out_strip) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_forced_colors_strip_box_shadow: Invalid argument\n");
    return rc;
  }

  *out_strip = ctx->active;
  return rc;
}
