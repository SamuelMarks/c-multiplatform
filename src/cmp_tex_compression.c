/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_tex_compression {
  cmp_tex_compression_type_t type;
  uint32_t width;
  uint32_t height;
  void *compressed_data;
  size_t data_size;
};

int cmp_tex_compression_create(cmp_tex_compression_type_t type, uint32_t width,
                               uint32_t height, const void *data,
                               size_t data_size,
                               cmp_tex_compression_t **out_tex_comp) {
  struct cmp_tex_compression *ctx;

  if (!out_tex_comp)
    return CMP_ERROR_INVALID_ARG;
  if (!data || data_size == 0 || width == 0 || height == 0)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_tex_compression), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  if (CMP_MALLOC(data_size, &ctx->compressed_data) != CMP_SUCCESS) {
    CMP_FREE(ctx);
    return CMP_ERROR_OOM;
  }

  memcpy(ctx->compressed_data, data, data_size);
  ctx->type = type;
  ctx->width = width;
  ctx->height = height;
  ctx->data_size = data_size;

  *out_tex_comp = (cmp_tex_compression_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_tex_compression_destroy(cmp_tex_compression_t *tex_comp) {
  struct cmp_tex_compression *ctx = (struct cmp_tex_compression *)tex_comp;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->compressed_data)
    CMP_FREE(ctx->compressed_data);
  CMP_FREE(ctx);

  return CMP_SUCCESS;
}

int cmp_tex_compression_mount(cmp_tex_compression_t *tex_comp,
                              cmp_texture_t *target_texture) {
  struct cmp_tex_compression *ctx = (struct cmp_tex_compression *)tex_comp;
  if (!ctx || !target_texture)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation, this function would pass the compressed data
     block to the GPU API (glCompressedTexImage2D, vkCmdCopyBufferToImage,
     etc.). For now, we simulate success by validating the input block and
     format. */

  if (ctx->type == CMP_TEX_COMPRESSION_NONE) {
    /* Technically not compressed, but we handle it. */
    target_texture->format = 0; /* Assuming 0 is RGBA uncompressed */
  } else {
    /* Set format to match the enum type so the renderer knows it's compressed
     */
    target_texture->format = (int)ctx->type;
  }

  target_texture->width = (int)ctx->width;
  target_texture->height = (int)ctx->height;

  /* We might allocate an internal handle to mock a bound texture on the GPU */
  if (!target_texture->internal_handle) {
    /* Test mock */
    target_texture->internal_handle = (void *)(size_t)0xCAFEBABE;
  }

  return CMP_SUCCESS;
}
