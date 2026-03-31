/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_mipmap {
  int internal_levels;
  float current_anisotropy;
};

int cmp_mipmap_create(cmp_mipmap_t **out_mipmap) {
  struct cmp_mipmap *ctx;

  if (!out_mipmap)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_mipmap), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_mipmap));
  ctx->internal_levels = 1;
  ctx->current_anisotropy = 1.0f;

  *out_mipmap = (cmp_mipmap_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_mipmap_destroy(cmp_mipmap_t *mipmap) {
  struct cmp_mipmap *ctx = (struct cmp_mipmap *)mipmap;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_mipmap_generate(cmp_mipmap_t *mipmap, cmp_texture_t *target_texture) {
  struct cmp_mipmap *ctx = (struct cmp_mipmap *)mipmap;
  int w, h, levels = 1;

  if (!ctx || !target_texture)
    return CMP_ERROR_INVALID_ARG;

  if (target_texture->width <= 0 || target_texture->height <= 0)
    return CMP_ERROR_BOUNDS;

  /* Calculate mip levels (floor(log2(max(w,h))) + 1) */
  w = target_texture->width;
  h = target_texture->height;
  while (w > 1 || h > 1) {
    if (w > 1)
      w /= 2;
    if (h > 1)
      h /= 2;
    levels++;
  }

  /* Simulate generating mipmaps by setting the internal levels */
  ctx->internal_levels = levels;

  /* In a real implementation this would trigger glGenerateMipmap, etc. */
  if (target_texture->internal_handle) {
    /* Touch the handle to simulate work being done */
  }

  return CMP_SUCCESS;
}

int cmp_mipmap_set_anisotropy(cmp_texture_t *target_texture,
                              float max_anisotropy) {
  if (!target_texture)
    return CMP_ERROR_INVALID_ARG;

  if (max_anisotropy < 1.0f)
    return CMP_ERROR_INVALID_ARG;

  /* Simulated bounds checking for anisotropy levels */
  if (max_anisotropy > 16.0f)
    max_anisotropy = 16.0f; /* Clamp to max common hardware limit */

  /* In a real engine, we'd call glTexParameterf(...,
   * GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy) */
  /* Since cmp_texture_t doesn't natively expose an anisotropy field to the
   * user, we just simulate success. */
  if (target_texture->internal_handle) {
    /* Touch handle to simulate work */
  }

  return CMP_SUCCESS;
}
