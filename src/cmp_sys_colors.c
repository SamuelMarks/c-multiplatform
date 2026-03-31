/* clang-format off */
#include "cmp.h"
#include <string.h>
/* clang-format on */

struct cmp_sys_colors {
  cmp_color_t colors[CMP_SYS_COLOR_MAX];
  int is_set[CMP_SYS_COLOR_MAX];
};

int cmp_sys_colors_create(cmp_sys_colors_t **out_ctx) {
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
  return CMP_SUCCESS;
}

int cmp_sys_colors_destroy(cmp_sys_colors_t *ctx) {
  if (!ctx) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_sys_colors_set(cmp_sys_colors_t *ctx, cmp_sys_color_keyword_t keyword,
                       const cmp_color_t *color) {
  if (!ctx || !color || keyword < 0 || keyword >= CMP_SYS_COLOR_MAX) {
    return CMP_ERROR_INVALID_ARG;
  }

  ctx->colors[keyword] = *color;
  ctx->is_set[keyword] = 1;

  return CMP_SUCCESS;
}

int cmp_sys_colors_resolve(const cmp_sys_colors_t *ctx,
                           cmp_sys_color_keyword_t keyword,
                           cmp_color_t *out_color) {
  if (!ctx || !out_color || keyword < 0 || keyword >= CMP_SYS_COLOR_MAX) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!ctx->is_set[keyword]) {
    return CMP_ERROR_NOT_FOUND;
  }

  *out_color = ctx->colors[keyword];
  return CMP_SUCCESS;
}
