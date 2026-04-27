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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_compositor_thread *ctx = NULL;

  if (out_thread == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_thread_create: Invalid argument "
                  "(out_thread=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_compositor_thread), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_thread_create: Out of memory: %s\n", err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  memset(ctx, 0, sizeof(struct cmp_compositor_thread));
  ctx->is_running = 1;
  *out_thread = (cmp_compositor_thread_t *)ctx;
  cmp_log_debug("cmp_compositor_thread_create: Successfully created compositor "
                "thread context\n");
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_compositor_thread_destroy
 *
 * @param thread Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_compositor_thread_destroy(cmp_compositor_thread_t *thread) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_compositor_thread *ctx = (struct cmp_compositor_thread *)thread;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_thread_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->is_running = 0;
  rc = CMP_FREE(ctx);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_compositor_thread_destroy: CMP_FREE failed\n");
  }

  cmp_log_debug("cmp_compositor_thread_destroy: Successfully destroyed "
                "compositor thread context\n");
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  struct cmp_compositor_thread *ctx = (struct cmp_compositor_thread *)thread;

  if (ctx == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_compositor_thread_push_tree: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->current_tree = layer_tree_opaque;
  cmp_log_debug("cmp_compositor_thread_push_tree: Mock pushed layout tree onto "
                "compositor thread\n");
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
