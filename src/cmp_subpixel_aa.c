/* clang-format off */
#include "cmp.h"
#include <string.h>
/* clang-format on */

typedef struct cmp_subpixel_aa {
  int enabled;
} cmp_subpixel_aa_t;

/**
 * @brief cmp_subpixel_aa_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_subpixel_aa_create(cmp_subpixel_aa_t **out_ctx) {
  int rc = CMP_SUCCESS;
  cmp_subpixel_aa_t *ctx;

  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_subpixel_aa_t), (void **)&ctx) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  ctx->enabled = 0;
  *out_ctx = ctx;

  return rc;
}

/**
 * @brief cmp_subpixel_aa_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_subpixel_aa_destroy(cmp_subpixel_aa_t *ctx) {
  int rc = CMP_SUCCESS;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx);

  return rc;
}

/**
 * @brief cmp_subpixel_aa_enable
 *
 * @param ctx Parameter description.
 * @param enable Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_subpixel_aa_enable(cmp_subpixel_aa_t *ctx, int enable) {
  int rc = CMP_SUCCESS;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->enabled = enable;

  return rc;
}

/**
 * @brief cmp_subpixel_aa_is_enabled
 *
 * @param ctx Parameter description.
 * @param out_enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_subpixel_aa_is_enabled(cmp_subpixel_aa_t *ctx, int *out_enabled) {
  int rc = CMP_SUCCESS;
  if (!ctx || !out_enabled)
    return CMP_ERROR_INVALID_ARG;
  *out_enabled = ctx->enabled;

  return rc;
}
