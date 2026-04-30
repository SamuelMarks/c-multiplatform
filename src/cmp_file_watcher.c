/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_file_watcher {
  cmp_vfs_watch_t *vfs_watch;
  cmp_file_watch_cb_t user_cb;
  void *user_data;
};

/**
 * @brief vfs_watch_proxy
 *
 * @param path Parameter description.
 * @param event_type Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_EXEMPT(static void vfs_watch_proxy(const char *path, int event_type,
                                       void *user_data)) {
  cmp_file_watcher_t *w = (cmp_file_watcher_t *)user_data;
  if (w && w->user_cb) {
    w->user_cb(path, event_type, w->user_data);
  }
}

/**
 * @brief cmp_file_watcher_create
 *
 * @param out_watcher Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_file_watcher_create(cmp_file_watcher_t **out_watcher) {
  int rc = CMP_SUCCESS;
  cmp_file_watcher_t *w = NULL;

  if (!out_watcher) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_file_watcher_create: Invalid argument "
              "(out_watcher=NULL)\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_file_watcher_t), (void **)&w);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_file_watcher_create: Out of memory\n");

    return rc;
  }

  w->vfs_watch = NULL;
  w->user_cb = NULL;
  w->user_data = NULL;

  *out_watcher = w;

  return rc;
}

/**
 * @brief cmp_file_watcher_destroy
 *
 * @param watcher Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_file_watcher_destroy(cmp_file_watcher_t *watcher) {
  int rc = CMP_SUCCESS;

  if (!watcher) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_file_watcher_destroy: Invalid argument (watcher=NULL)\n");

    return rc;
  }

  rc = cmp_file_watcher_stop(watcher);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_file_watcher_destroy: Failed to stop watcher\n");
  }

  rc = CMP_FREE(watcher);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_file_watcher_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_file_watcher_start
 *
 * @param watcher Parameter description.
 * @param dir_path Parameter description.
 * @param cb Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_file_watcher_start(cmp_file_watcher_t *watcher, const char *dir_path,
                           cmp_file_watch_cb_t cb, void *user_data) {
  int rc = CMP_SUCCESS;

  if (!watcher || !dir_path || !cb) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_file_watcher_start: Invalid argument\n");

    return rc;
  }

  /* Clean up existing watch if re-started */
  rc = cmp_file_watcher_stop(watcher);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_file_watcher_start: Failed to stop existing watcher\n");

    return rc;
  }

  watcher->user_cb = cb;
  watcher->user_data = user_data;

  rc = cmp_vfs_watch_path(dir_path, vfs_watch_proxy, watcher,
                          &watcher->vfs_watch);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_file_watcher_start: Failed to watch path\n");
  }

  return rc;
}

/**
 * @brief cmp_file_watcher_stop
 *
 * @param watcher Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_file_watcher_stop(cmp_file_watcher_t *watcher) {
  int rc = CMP_SUCCESS;

  if (!watcher) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_file_watcher_stop: Invalid argument (watcher=NULL)\n");

    return rc;
  }

  if (watcher->vfs_watch) {
    rc = cmp_vfs_unwatch(watcher->vfs_watch);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_file_watcher_stop: Failed to unwatch VFS\n");
    }
    watcher->vfs_watch = NULL;
  }

  return rc;
}