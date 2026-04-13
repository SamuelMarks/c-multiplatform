/* clang-format off */
#include "cmp_compositor_thread.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_compositor_thread {
  int is_running;
  void *current_tree;
};

int cmp_compositor_thread_create(cmp_compositor_thread_t **out_thread) {
  struct cmp_compositor_thread *ctx;
  if (!out_thread)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_compositor_thread), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(ctx, 0, sizeof(struct cmp_compositor_thread));
  ctx->is_running = 1;
  *out_thread = (cmp_compositor_thread_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_compositor_thread_destroy(cmp_compositor_thread_t *thread) {
  struct cmp_compositor_thread *ctx = (struct cmp_compositor_thread *)thread;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->is_running = 0;
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_compositor_thread_push_tree(cmp_compositor_thread_t *thread,
                                    void *layer_tree_opaque) {
  struct cmp_compositor_thread *ctx = (struct cmp_compositor_thread *)thread;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  ctx->current_tree = layer_tree_opaque;
  return CMP_SUCCESS;
}