/* clang-format off */
#include "cmp_lottie.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_lottie {
  float current_time_ms;
  float total_duration_ms;
};

int cmp_lottie_create(const char *json_buffer, size_t size, cmp_lottie_t **out_lottie) {
  struct cmp_lottie *ctx;
  if (!json_buffer || size == 0 || !out_lottie)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_lottie), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ctx, 0, sizeof(struct cmp_lottie));
  ctx->current_time_ms = 0.0f;
  ctx->total_duration_ms = 1000.0f; /* Mock duration */
  *out_lottie = (cmp_lottie_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_lottie_destroy(cmp_lottie_t *lottie) {
  struct cmp_lottie *ctx = (struct cmp_lottie *)lottie;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_lottie_step(cmp_lottie_t *lottie, float dt_ms) {
  struct cmp_lottie *ctx = (struct cmp_lottie *)lottie;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->current_time_ms += dt_ms;
  if (ctx->current_time_ms > ctx->total_duration_ms) {
    ctx->current_time_ms = 0.0f; /* Loop */
  }
  return CMP_SUCCESS;
}