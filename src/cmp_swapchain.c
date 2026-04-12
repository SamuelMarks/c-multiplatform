/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define CMP_MAX_SWAPCHAIN_BUFFERS 3

struct cmp_swapchain {
  cmp_window_t *window;
  cmp_swapchain_mode_t mode;
  cmp_texture_t *buffers[CMP_MAX_SWAPCHAIN_BUFFERS];
  int buffer_count;
  int frames_in_flight;
  int current_frame_index;
  int current_image_index;
  int is_active;
  void *os_surface_handle;
};

int cmp_swapchain_create(cmp_window_t *window, cmp_swapchain_mode_t mode,
                         cmp_swapchain_t **out_swapchain) {
  struct cmp_swapchain *ctx;
  int i;

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
  ctx->os_surface_handle = cmp_window_get_native_handle(window);

  /* Configure buffers based on presentation mode */
  if (mode == CMP_SWAPCHAIN_MAILBOX) {
    ctx->buffer_count = 3;     /* Triple buffering */
    ctx->frames_in_flight = 2; /* 2 frames in flight usually */
  } else if (mode == CMP_SWAPCHAIN_FIFO) {
    ctx->buffer_count = 2; /* Double buffering */
    ctx->frames_in_flight = 1;
  } else {                 /* IMMEDIATE */
    ctx->buffer_count = 2; /* Double buffering */
    ctx->frames_in_flight = 1;
  }

  for (i = 0; i < ctx->buffer_count; ++i) {
    if (CMP_MALLOC(sizeof(cmp_texture_t), (void **)&ctx->buffers[i]) !=
        CMP_SUCCESS) {
      /* Cleanup on failure */
      int j;
      for (j = 0; j < i; ++j) {
        CMP_FREE(ctx->buffers[j]);
      }
      CMP_FREE(ctx);
      return CMP_ERROR_OOM;
    }
    memset(ctx->buffers[i], 0, sizeof(cmp_texture_t));
    /* Mock internal handle */
    ctx->buffers[i]->internal_handle = (void *)(size_t)(0x11111111 * (i + 1));
  }

  ctx->current_frame_index = 0;
  ctx->current_image_index = 0;

  *out_swapchain = (cmp_swapchain_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_swapchain_destroy(cmp_swapchain_t *swapchain) {
  struct cmp_swapchain *ctx = (struct cmp_swapchain *)swapchain;
  int i;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < ctx->buffer_count; ++i) {
    if (ctx->buffers[i]) {
      CMP_FREE(ctx->buffers[i]);
      ctx->buffers[i] = NULL;
    }
  }

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

  /* In a real engine we wait on a fence/semaphore for frames-in-flight
   * pipelining */
  /* Fence wait for ctx->current_frame_index */

  /* Acquire next image index */
  ctx->current_image_index = (ctx->current_image_index + 1) % ctx->buffer_count;
  *out_texture = ctx->buffers[ctx->current_image_index];

  return CMP_SUCCESS;
}

int cmp_swapchain_present(cmp_swapchain_t *swapchain) {
  struct cmp_swapchain *ctx = (struct cmp_swapchain *)swapchain;

  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (!ctx->is_active)
    return CMP_ERROR_IO;

  /* In a real engine, this calls vkQueuePresentKHR, [[MTLCommandBuffer
     presentDrawable:]], or wglSwapBuffers. Here, we simulate the swap. */

  /* Advance the frame index for frames-in-flight tracking */
  ctx->current_frame_index =
      (ctx->current_frame_index + 1) % ctx->frames_in_flight;

  return CMP_SUCCESS;
}

void *cmp_swapchain_get_os_surface_handle(cmp_swapchain_t *swapchain) {
  struct cmp_swapchain *ctx = (struct cmp_swapchain *)swapchain;
  if (!ctx)
    return NULL;
  return ctx->os_surface_handle;
}

int cmp_swapchain_set_msaa(cmp_swapchain_t *swapchain, int sample_count) {
  if (!swapchain || sample_count < 1)
    return CMP_ERROR_INVALID_ARG;
  /* Mock implementation */
  return CMP_SUCCESS;
}
