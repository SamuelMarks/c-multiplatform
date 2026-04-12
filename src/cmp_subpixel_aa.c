/* clang-format off */
#include "cmp.h"
#include <string.h>
/* clang-format on */

typedef struct cmp_subpixel_aa {
  int enabled;
} cmp_subpixel_aa_t;

int cmp_subpixel_aa_create(cmp_subpixel_aa_t **out_ctx) {
  cmp_subpixel_aa_t *ctx;

  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_subpixel_aa_t), (void **)&ctx) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  ctx->enabled = 0;
  *out_ctx = ctx;
  return CMP_SUCCESS;
}

int cmp_subpixel_aa_destroy(cmp_subpixel_aa_t *ctx) {
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_subpixel_aa_enable(cmp_subpixel_aa_t *ctx, int enable) {
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->enabled = enable;
  return CMP_SUCCESS;
}

int cmp_subpixel_aa_is_enabled(cmp_subpixel_aa_t *ctx, int *out_enabled) {
  if (!ctx || !out_enabled)
    return CMP_ERROR_INVALID_ARG;
  *out_enabled = ctx->enabled;
  return CMP_SUCCESS;
}
