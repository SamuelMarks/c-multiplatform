/* clang-format off */
#include "cmp_lottie.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_lottie {
  float current_time_ms;
  float total_duration_ms;
};

int cmp_lottie_create(const char *json_buffer, size_t size,
                      cmp_lottie_t **out_lottie) {
  int rc = CMP_SUCCESS;
  struct cmp_lottie *ctx = NULL;

  if (!json_buffer || size == 0 || !out_lottie) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_lottie_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_lottie), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_lottie_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_lottie));
  ctx->current_time_ms = 0.0f;
  ctx->total_duration_ms = 1000.0f; /* Mock duration */
  *out_lottie = (cmp_lottie_t *)ctx;

  return rc;
}

int cmp_lottie_destroy(cmp_lottie_t *lottie) {
  int rc = CMP_SUCCESS;
  struct cmp_lottie *ctx = (struct cmp_lottie *)lottie;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_lottie_destroy: Invalid argument\n");
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

int cmp_lottie_step(cmp_lottie_t *lottie, float dt_ms) {
  int rc = CMP_SUCCESS;
  struct cmp_lottie *ctx = (struct cmp_lottie *)lottie;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_lottie_step: Invalid argument\n");
    return rc;
  }

  ctx->current_time_ms += dt_ms;
  if (ctx->current_time_ms > ctx->total_duration_ms) {
    ctx->current_time_ms = 0.0f; /* Loop */
  }
  return rc;
}