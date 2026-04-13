/* clang-format off */
#include "cmp_apz.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_apz {
  float current_x;
  float current_y;
  float current_scale;
};

int cmp_apz_create(cmp_apz_t **out_apz) {
  struct cmp_apz *ctx;
  if (!out_apz)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_apz), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ctx, 0, sizeof(struct cmp_apz));
  ctx->current_scale = 1.0f;
  *out_apz = (cmp_apz_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_apz_destroy(cmp_apz_t *apz) {
  struct cmp_apz *ctx = (struct cmp_apz *)apz;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_apz_inject_gesture(cmp_apz_t *apz, float delta_x, float delta_y,
                           float scale) {
  struct cmp_apz *ctx = (struct cmp_apz *)apz;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->current_x += delta_x;
  ctx->current_y += delta_y;
  ctx->current_scale *= scale;
  return CMP_SUCCESS;
}

int cmp_apz_get_transform(const cmp_apz_t *apz, float *out_matrix) {
  const struct cmp_apz *ctx = (const struct cmp_apz *)apz;
  int i;
  if (!ctx || !out_matrix)
    return CMP_ERROR_INVALID_ARG;
  for (i = 0; i < 16; i++) {
    out_matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f; /* Identity */
  }
  out_matrix[0] = ctx->current_scale;
  out_matrix[5] = ctx->current_scale;
  out_matrix[12] = ctx->current_x;
  out_matrix[13] = ctx->current_y;
  return CMP_SUCCESS;
}