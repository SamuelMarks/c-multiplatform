/* clang-format off */
#include "cmp_compositor_thread.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_compositor_thread {
  int is_running;
  void *current_tree;
};

/**
 * @brief cmp_compositor_thread_create
 *
 * @param out_thread Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_thread_create(cmp_compositor_thread_t **out_thread) {
  int rc = CMP_SUCCESS;
  struct cmp_compositor_thread *ctx = NULL;

  if (!out_thread) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_thread_create: Invalid argument "
              "(out_thread=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_compositor_thread), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_compositor_thread_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_compositor_thread));
  ctx->is_running = 1;
  *out_thread = (cmp_compositor_thread_t *)ctx;
  return rc;
}

/**
 * @brief cmp_compositor_thread_destroy
 *
 * @param thread Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_thread_destroy(cmp_compositor_thread_t *thread) {
  int rc = CMP_SUCCESS;
  struct cmp_compositor_thread *ctx = (struct cmp_compositor_thread *)thread;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_thread_destroy: Invalid argument "
              "(thread=NULL)\n");
    return rc;
  }

  ctx->is_running = 0;
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_compositor_thread_push_tree
 *
 * @param thread Parameter description.
 * @param layer_tree_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_thread_push_tree(cmp_compositor_thread_t *thread,
                                    void *layer_tree_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_compositor_thread *ctx = (struct cmp_compositor_thread *)thread;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_compositor_thread_push_tree: Invalid argument "
              "(thread=NULL)\n");
    return rc;
  }

  ctx->current_tree = layer_tree_opaque;
  return rc;
}