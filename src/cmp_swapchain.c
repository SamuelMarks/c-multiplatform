/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_swapchain {
  cmp_window_t *window;
  cmp_swapchain_mode_t mode;
  cmp_texture_t *current_frontbuffer;
  cmp_texture_t *current_backbuffer;
  int is_active;
};

int cmp_swapchain_create(cmp_window_t *window, cmp_swapchain_mode_t mode,
                         cmp_swapchain_t **out_swapchain) {
  struct cmp_swapchain *ctx;

  if (!out_swapchain)
    return CMP_ERROR_INVALID_ARG;
  if (!window)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(struct cmp_swapchain), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(struct cmp_swapchain));
  ctx->window = window;
  ctx->mode = mode;
  ctx->is_active = 1;

  /* We allocate dummy mock textures to simulate the swapchain buffers */
  if (CMP_MALLOC(sizeof(cmp_texture_t), (void **)&ctx->current_frontbuffer) !=
      CMP_SUCCESS) {
    CMP_FREE(ctx);
    return CMP_ERROR_OOM;
  }
  memset(ctx->current_frontbuffer, 0, sizeof(cmp_texture_t));
  ctx->current_frontbuffer->internal_handle = (void *)(size_t)0x11111111;

  if (CMP_MALLOC(sizeof(cmp_texture_t), (void **)&ctx->current_backbuffer) !=
      CMP_SUCCESS) {
    CMP_FREE(ctx->current_frontbuffer);
    CMP_FREE(ctx);
    return CMP_ERROR_OOM;
  }
  memset(ctx->current_backbuffer, 0, sizeof(cmp_texture_t));
  ctx->current_backbuffer->internal_handle = (void *)(size_t)0x22222222;

  *out_swapchain = (cmp_swapchain_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_swapchain_destroy(cmp_swapchain_t *swapchain) {
  struct cmp_swapchain *ctx = (struct cmp_swapchain *)swapchain;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (ctx->current_frontbuffer)
    CMP_FREE(ctx->current_frontbuffer);

  if (ctx->current_backbuffer)
    CMP_FREE(ctx->current_backbuffer);

  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_swapchain_acquire_next_image(cmp_swapchain_t *swapchain,
                                     cmp_texture_t **out_texture) {
  struct cmp_swapchain *ctx = (struct cmp_swapchain *)swapchain;
  if (!ctx || !out_texture)
    return CMP_ERROR_INVALID_ARG;

  if (!ctx->is_active)
    return CMP_ERROR_IO; /* Hardware failure simulation */

  /* The renderer draws to the backbuffer */
  *out_texture = ctx->current_backbuffer;

  return CMP_SUCCESS;
}

int cmp_swapchain_present(cmp_swapchain_t *swapchain) {
  struct cmp_swapchain *ctx = (struct cmp_swapchain *)swapchain;
  cmp_texture_t *temp;

  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (!ctx->is_active)
    return CMP_ERROR_IO;

  /* In a real engine, this calls vkQueuePresentKHR, [[MTLCommandBuffer
     presentDrawable:]], or wglSwapBuffers. Here, we simulate the swap. */

  temp = ctx->current_frontbuffer;
  ctx->current_frontbuffer = ctx->current_backbuffer;
  ctx->current_backbuffer = temp;

  return CMP_SUCCESS;
}
