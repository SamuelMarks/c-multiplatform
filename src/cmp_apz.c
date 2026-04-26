/* clang-format off */
#include "cmp_apz.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

struct cmp_apz {
  float current_x;
  float current_y;
  float current_scale;
};

/**
 * @brief cmp_apz_create
 *
 * @param out_apz Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_apz_create(cmp_apz_t **out_apz) {
  int rc = CMP_SUCCESS;
  struct cmp_apz *ctx = NULL;

  if (!out_apz) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apz_create: Invalid argument (out_apz=NULL): %s\n",
                err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_apz), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apz_create: Out of memory: %s\n", err_str);
    }
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_apz));
  ctx->current_scale = 1.0f;
  *out_apz = (cmp_apz_t *)ctx;
  return rc;
}

/**
 * @brief cmp_apz_destroy
 *
 * @param apz Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_apz_destroy(cmp_apz_t *apz) {
  int rc = CMP_SUCCESS;
  struct cmp_apz *ctx = (struct cmp_apz *)apz;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apz_destroy: Invalid argument (apz=NULL): %s\n", err_str);
    }
    return rc;
  }

  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_apz_inject_gesture
 *
 * @param apz Parameter description.
 * @param delta_x Parameter description.
 * @param delta_y Parameter description.
 * @param scale Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_apz_inject_gesture(cmp_apz_t *apz, float delta_x, float delta_y,
                           float scale) {
  int rc = CMP_SUCCESS;
  struct cmp_apz *ctx = (struct cmp_apz *)apz;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apz_inject_gesture: Invalid argument (apz=NULL): %s\n",
                err_str);
    }
    return rc;
  }

  ctx->current_x += delta_x;
  ctx->current_y += delta_y;
  ctx->current_scale *= scale;
  return rc;
}

/**
 * @brief cmp_apz_get_transform
 *
 * @param apz Parameter description.
 * @param out_matrix Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_apz_get_transform(const cmp_apz_t *apz, float *out_matrix) {
  int rc = CMP_SUCCESS;
  const struct cmp_apz *ctx = (const struct cmp_apz *)apz;
  int i;

  if (!ctx || !out_matrix) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      cmp_strerror(rc, &err_str);
      LOG_DEBUG("cmp_apz_get_transform: Invalid argument: %s\n", err_str);
    }
    return rc;
  }

  for (i = 0; i < 16; i++) {
    out_matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f; /* Identity */
  }
  out_matrix[0] = ctx->current_scale;
  out_matrix[5] = ctx->current_scale;
  out_matrix[12] = ctx->current_x;
  out_matrix[13] = ctx->current_y;
  return rc;
}