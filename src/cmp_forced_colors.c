/* clang-format off */
#include "cmp.h"
/* clang-format on */

struct cmp_forced_colors {
  int active;
};

int cmp_forced_colors_create(cmp_forced_colors_t **out_ctx) {
  cmp_forced_colors_t *ctx;

  if (!out_ctx) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_forced_colors_t), (void **)&ctx) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  ctx->active = 0;
  *out_ctx = ctx;
  return CMP_SUCCESS;
}

int cmp_forced_colors_destroy(cmp_forced_colors_t *ctx) {
  if (!ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_forced_colors_set(cmp_forced_colors_t *ctx, int active) {
  if (!ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  ctx->active = active ? 1 : 0;
  return CMP_SUCCESS;
}

int cmp_forced_colors_strip_background(const cmp_forced_colors_t *ctx,
                                       int *out_strip) {
  if (!ctx || !out_strip) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_strip = ctx->active;
  return CMP_SUCCESS;
}

int cmp_forced_colors_strip_box_shadow(const cmp_forced_colors_t *ctx,
                                       int *out_strip) {
  if (!ctx || !out_strip) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_strip = ctx->active;
  return CMP_SUCCESS;
}
