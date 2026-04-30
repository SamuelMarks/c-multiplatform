/* clang-format off */
#include "cmp.h"
#include <string.h>
/* clang-format on */

struct cmp_sys_colors {
  cmp_color_t colors[CMP_SYS_COLOR_MAX];
  int is_set[CMP_SYS_COLOR_MAX];
};

/**
 * @brief cmp_sys_colors_create
 *
 * @param out_ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sys_colors_create(cmp_sys_colors_t **out_ctx) {
  int rc = CMP_SUCCESS;
  cmp_sys_colors_t *ctx;

  if (!out_ctx) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_sys_colors_t), (void **)&ctx) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(ctx->colors, 0, sizeof(ctx->colors));
  memset(ctx->is_set, 0, sizeof(ctx->is_set));

  *out_ctx = ctx;

  return rc;
}

/**
 * @brief cmp_sys_colors_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sys_colors_destroy(cmp_sys_colors_t *ctx) {
  int rc = CMP_SUCCESS;
  if (!ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(ctx);

  return rc;
}

/**
 * @brief cmp_sys_colors_set
 *
 * @param ctx Parameter description.
 * @param keyword Parameter description.
 * @param color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sys_colors_set(cmp_sys_colors_t *ctx, cmp_sys_color_keyword_t keyword,
                       const cmp_color_t *color) {
  int rc = CMP_SUCCESS;
  if (!ctx || !color || keyword < 0 || keyword >= CMP_SYS_COLOR_MAX) {
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->colors[keyword] = *color;
  ctx->is_set[keyword] = 1;

  return rc;
}

/**
 * @brief cmp_sys_colors_resolve
 *
 * @param ctx Parameter description.
 * @param keyword Parameter description.
 * @param out_color Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_sys_colors_resolve(const cmp_sys_colors_t *ctx,
                           cmp_sys_color_keyword_t keyword,
                           cmp_color_t *out_color) {
  int rc = CMP_SUCCESS;
  if (!ctx || !out_color || keyword < 0 || keyword >= CMP_SYS_COLOR_MAX) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!ctx->is_set[keyword]) {
    return CMP_ERROR_NOT_FOUND;
  }

  *out_color = ctx->colors[keyword];

  return rc;
}
