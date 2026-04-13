/* clang-format off */
#include "cmp_fluent.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_fluent_reveal {
  float pointer_x;
  float pointer_y;
};

int cmp_fluent_reveal_create(cmp_fluent_reveal_t **out_reveal) {
  struct cmp_fluent_reveal *ctx;
  if (!out_reveal)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_fluent_reveal), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ctx, 0, sizeof(struct cmp_fluent_reveal));
  *out_reveal = (cmp_fluent_reveal_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_fluent_reveal_destroy(cmp_fluent_reveal_t *reveal) {
  struct cmp_fluent_reveal *ctx = (struct cmp_fluent_reveal *)reveal;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_fluent_reveal_update_pointer(cmp_fluent_reveal_t *reveal,
                                     float pointer_x, float pointer_y) {
  struct cmp_fluent_reveal *ctx = (struct cmp_fluent_reveal *)reveal;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->pointer_x = pointer_x;
  ctx->pointer_y = pointer_y;
  return CMP_SUCCESS;
}

struct cmp_acrylic_noise {
  int width;
  int height;
  unsigned char *pixels;
};

int cmp_acrylic_noise_create(int width, int height,
                             cmp_acrylic_noise_t **out_noise) {
  struct cmp_acrylic_noise *ctx;
  if (!out_noise || width <= 0 || height <= 0)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_acrylic_noise), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ctx, 0, sizeof(struct cmp_acrylic_noise));
  ctx->width = width;
  ctx->height = height;
  if (CMP_MALLOC((size_t)(width * height), (void **)&ctx->pixels) !=
      CMP_SUCCESS) {
    CMP_FREE(ctx);
    return CMP_ERROR_OOM;
  }
  /* Fill with random monochrome noise */
  {
    int i;
    for (i = 0; i < width * height; i++) {
      ctx->pixels[i] = (unsigned char)(rand() % 256);
    }
  }
  *out_noise = (cmp_acrylic_noise_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_acrylic_noise_destroy(cmp_acrylic_noise_t *noise) {
  struct cmp_acrylic_noise *ctx = (struct cmp_acrylic_noise *)noise;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (ctx->pixels)
    CMP_FREE(ctx->pixels);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}