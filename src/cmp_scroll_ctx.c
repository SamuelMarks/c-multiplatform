/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_scroll_ctx {
  float scroll_top;
  float scroll_left;
};

int cmp_scroll_ctx_create(cmp_scroll_ctx_t **out_ctx) {
  struct cmp_scroll_ctx *ctx;

  if (!out_ctx)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_scroll_ctx), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_scroll_ctx));

  *out_ctx = (cmp_scroll_ctx_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_scroll_ctx_destroy(cmp_scroll_ctx_t *ctx) {
  struct cmp_scroll_ctx *internal_ctx = (struct cmp_scroll_ctx *)ctx;

  if (!internal_ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(internal_ctx);
  return CMP_SUCCESS;
}

int cmp_scroll_ctx_get_offset(const cmp_scroll_ctx_t *ctx, float *out_x,
                              float *out_y) {
  const struct cmp_scroll_ctx *internal_ctx =
      (const struct cmp_scroll_ctx *)ctx;

  if (!internal_ctx)
    return CMP_ERROR_INVALID_ARG;

  if (out_x)
    *out_x = internal_ctx->scroll_left;

  if (out_y)
    *out_y = internal_ctx->scroll_top;

  return CMP_SUCCESS;
}

int cmp_scroll_ctx_set_offset(cmp_scroll_ctx_t *ctx, float x, float y) {
  struct cmp_scroll_ctx *internal_ctx = (struct cmp_scroll_ctx *)ctx;

  if (!internal_ctx)
    return CMP_ERROR_INVALID_ARG;

  internal_ctx->scroll_left = x;
  internal_ctx->scroll_top = y;

  return CMP_SUCCESS;
}

int cmp_scroll_ctx_inject_to_gpu(const cmp_scroll_ctx_t *ctx, cmp_ubo_t *ubo) {
  const struct cmp_scroll_ctx *internal_ctx =
      (const struct cmp_scroll_ctx *)ctx;
  float data[2];

  if (!internal_ctx || !ubo)
    return CMP_ERROR_INVALID_ARG;

  data[0] = internal_ctx->scroll_left;
  data[1] = internal_ctx->scroll_top;

  return cmp_ubo_update(ubo, data, sizeof(data));
}
