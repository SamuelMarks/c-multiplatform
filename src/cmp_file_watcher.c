/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_file_watcher {
  cmp_vfs_watch_t *vfs_watch;
  cmp_file_watch_cb_t user_cb;
  void *user_data;
};

static void vfs_watch_proxy(const char *path, int event_type, void *user_data) {
  cmp_file_watcher_t *w = (cmp_file_watcher_t *)user_data;
  if (w && w->user_cb) {
    w->user_cb(path, event_type, w->user_data);
  }
}

int cmp_file_watcher_create(cmp_file_watcher_t **out_watcher) {
  cmp_file_watcher_t *w;
  if (!out_watcher)
    return CMP_ERROR_INVALID_ARG;

  w = (cmp_file_watcher_t *)malloc(sizeof(cmp_file_watcher_t));
  if (!w)
    return CMP_ERROR_OOM;

  w->vfs_watch = NULL;
  w->user_cb = NULL;
  w->user_data = NULL;

  *out_watcher = w;
  return CMP_SUCCESS;
}

int cmp_file_watcher_destroy(cmp_file_watcher_t *watcher) {
  if (!watcher)
    return CMP_ERROR_INVALID_ARG;
  cmp_file_watcher_stop(watcher);
  free(watcher);
  return CMP_SUCCESS;
}

int cmp_file_watcher_start(cmp_file_watcher_t *watcher, const char *dir_path,
                           cmp_file_watch_cb_t cb, void *user_data) {
  int result;

  if (!watcher || !dir_path || !cb)
    return CMP_ERROR_INVALID_ARG;

  /* Clean up existing watch if re-started */
  cmp_file_watcher_stop(watcher);

  watcher->user_cb = cb;
  watcher->user_data = user_data;

  result = cmp_vfs_watch_path(dir_path, vfs_watch_proxy, watcher,
                              &watcher->vfs_watch);
  return result;
}

int cmp_file_watcher_stop(cmp_file_watcher_t *watcher) {
  if (!watcher)
    return CMP_ERROR_INVALID_ARG;

  if (watcher->vfs_watch) {
    cmp_vfs_unwatch(watcher->vfs_watch);
    watcher->vfs_watch = NULL;
  }
  return CMP_SUCCESS;
}
