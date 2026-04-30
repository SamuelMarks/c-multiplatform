/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <winsock2.h>
#else
#include <unistd.h>
#if !defined(__WATCOMC__) && !defined(__DOS__)
#include <pthread.h>
#endif
#include <fcntl.h>
#if defined(__linux__)
#include <sys/inotify.h>
#include <sys/select.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif
#endif
/* clang-format on */

typedef struct cmp_vfs_mount_entry {
  char *mount_point;
  char *real_path;
  struct cmp_vfs_mount_entry *next;
} cmp_vfs_mount_entry_t;

static int g_vfs_initialized = 0;
static cfs_runtime_t *g_cfs_rt = NULL;
static cmp_vfs_mount_entry_t *g_mounts = NULL;

/**
 * @brief Documented function cmp_vfs_init.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_init(void) {
  int rc = CMP_SUCCESS;
  cfs_runtime_config cfg;
  if (g_vfs_initialized) {
    return rc;
  }

  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = cfs_modality_singlethread;
  if (cfs_runtime_init(&cfg, &g_cfs_rt, NULL) != 0) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_init: cfs_runtime_init failed\n");
    return rc;
  }

  g_mounts = NULL;
  g_vfs_initialized = 1;
  return rc;
}

/**
 * @brief Documented function cmp_vfs_shutdown.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_shutdown(void) {
  int rc = CMP_SUCCESS;
  cmp_vfs_mount_entry_t *curr, *next;
  int free_rc;

  if (!g_vfs_initialized) {
    return rc;
  }

  curr = g_mounts;
  while (curr != NULL) {
    next = curr->next;
    free_rc = CMP_FREE(curr->mount_point);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_shutdown: CMP_FREE mount_point failed\n");
    }
    free_rc = CMP_FREE(curr->real_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_shutdown: CMP_FREE real_path failed\n");
    }
    free_rc = CMP_FREE(curr);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_shutdown: CMP_FREE curr failed\n");
    }
    curr = next;
  }
  g_mounts = NULL;

  if (g_cfs_rt != NULL) {
    cfs_runtime_destroy(g_cfs_rt);
    g_cfs_rt = NULL;
  }

  g_vfs_initialized = 0;
  return rc;
}

/**
 * @brief Documented function cmp_vfs_mount.
 *
 * @param mount_point Parameter description.
 * @param real_path Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_mount(const char *mount_point, const char *real_path) {
  int rc = CMP_SUCCESS;
  cmp_vfs_mount_entry_t *entry;
  size_t mp_len, rp_len;
  int free_rc;

  if (mount_point == NULL || real_path == NULL || !g_vfs_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_mount: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_vfs_mount_entry_t), (void **)&entry);
  if (rc != CMP_SUCCESS) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_mount: CMP_MALLOC entry failed\n");
    return rc;
  }

  mp_len = strlen(mount_point);
  rp_len = strlen(real_path);

  rc = CMP_MALLOC(mp_len + 1, (void **)&entry->mount_point);
  if (rc != CMP_SUCCESS) {
    free_rc = CMP_FREE(entry);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_mount: CMP_FREE entry failed\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_mount: CMP_MALLOC mount_point failed\n");
    return rc;
  }

  rc = CMP_MALLOC(rp_len + 1, (void **)&entry->real_path);
  if (rc != CMP_SUCCESS) {
    free_rc = CMP_FREE(entry->mount_point);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_mount: CMP_FREE mount_point failed\n");
    }
    free_rc = CMP_FREE(entry);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_mount: CMP_FREE entry failed\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_mount: CMP_MALLOC real_path failed\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(entry->mount_point, mp_len + 1, mount_point);
  strcpy_s(entry->real_path, rp_len + 1, real_path);
#else
  strcpy(entry->mount_point, mount_point);
  strcpy(entry->real_path, real_path);
#endif

  entry->next = g_mounts;
  g_mounts = entry;

  return rc;
}

/**
 * @brief Documented function cmp_vfs_resolve_path.
 *
 * @param virtual_path Parameter description.
 * @param out_path Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_resolve_path(const char *virtual_path, cmp_string_t *out_path) {
  int rc = CMP_SUCCESS;
  cmp_vfs_mount_entry_t *curr;
  size_t mp_len;

  if (virtual_path == NULL || out_path == NULL || !g_vfs_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_resolve_path: Invalid argument\n");
    return rc;
  }

  rc = cmp_string_init(out_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_resolve_path: cmp_string_init failed\n");
    return rc;
  }

  curr = g_mounts;
  while (curr != NULL) {
    mp_len = strlen(curr->mount_point);
    if (strncmp(virtual_path, curr->mount_point, mp_len) == 0) {
      rc = cmp_string_append(out_path, curr->real_path);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_vfs_resolve_path: cmp_string_append failed\n");
        return rc;
      }
      /* If virtual path continues after mount point, append it */
      if (virtual_path[mp_len] != '\0') {
        /* Add a slash only if real_path doesn't end with one and virtual_path
         * doesn't start with one */
        size_t rp_len = strlen(curr->real_path);
        int rp_has_slash =
            (rp_len > 0 && (curr->real_path[rp_len - 1] == '/' ||
                            curr->real_path[rp_len - 1] == '\\'));
        int vp_has_slash =
            (virtual_path[mp_len] == '/' || virtual_path[mp_len] == '\\');

        if (!rp_has_slash && !vp_has_slash) {
          rc = cmp_string_append(out_path, "/");
          if (rc != CMP_SUCCESS) {
            LOG_DEBUG("cmp_vfs_resolve_path: cmp_string_append failed\n");
            return rc;
          }
        } else if (rp_has_slash && vp_has_slash) {
          /* Skip the extra slash in virtual_path */
          rc = cmp_string_append(out_path, virtual_path + mp_len + 1);
          if (rc != CMP_SUCCESS) {
            LOG_DEBUG("cmp_vfs_resolve_path: cmp_string_append failed\n");
            return rc;
          }
          return rc;
        }
        rc = cmp_string_append(out_path, virtual_path + mp_len);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_vfs_resolve_path: cmp_string_append failed\n");
          return rc;
        }
      }
      return rc;
    }
    curr = curr->next;
  }

  /* Fallback: treat virtual path as physical path */
  rc = cmp_string_append(out_path, virtual_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_resolve_path: cmp_string_append failed\n");
    return rc;
  }
  return rc;
}

/**
 * @brief Documented function cmp_vfs_read_file_sync.
 *
 * @param virtual_path Parameter description.
 * @param out_buffer Parameter description.
 * @param out_size Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_read_file_sync(const char *virtual_path, void **out_buffer,
                           size_t *out_size) {
  int rc = CMP_SUCCESS;
  cfs_path p;
  cmp_string_t resolved_path;
  const cfs_char_t *native_path;
  FILE *f;
  cfs_uintmax_t file_size = 0;
  void *buf = NULL;
  int res;
  size_t read_bytes;
  int free_rc;

  if (virtual_path == NULL || out_buffer == NULL || out_size == NULL ||
      !g_vfs_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_read_file_sync: Invalid argument\n");
    return rc;
  }

  rc = cmp_vfs_resolve_path(virtual_path, &resolved_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_read_file_sync: cmp_vfs_resolve_path failed\n");
    return rc;
  }

  cfs_path_init(&p);
#if defined(_WIN32)
  {
    wchar_t *wpath = NULL;
    cfs_size_t req_len = 0;
    cfs_utf8_to_utf16(resolved_path.data, NULL, 0, &req_len);
    if (req_len <= 0) {
      cfs_path_destroy(&p);
      cmp_string_destroy(&resolved_path);
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_vfs_read_file_sync: cfs_utf8_to_utf16 failed\n");
      return rc;
    }
    rc = CMP_MALLOC((size_t)req_len * sizeof(wchar_t), (void **)&wpath);
    if (rc != CMP_SUCCESS) {
      cfs_path_destroy(&p);
      cmp_string_destroy(&resolved_path);
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_vfs_read_file_sync: CMP_MALLOC wpath failed\n");
      return rc;
    }
    cfs_utf8_to_utf16(resolved_path.data, wpath, req_len, NULL);
    cfs_path_assign(&p, wpath);
    free_rc = CMP_FREE(wpath);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_read_file_sync: CMP_FREE wpath failed\n");
    }
  }
#else
  cfs_path_assign(&p, resolved_path.data);
#endif

  rc = cmp_string_destroy(&resolved_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_read_file_sync: cmp_string_destroy failed\n");
  }

  cfs_path_c_str(&p, &native_path);
#if defined(_WIN32)
  f = _wfopen((const wchar_t *)native_path, L"rb");
#else
  f = fopen(native_path, "rb");
#endif

  if (!f) {
    cfs_path_destroy(&p);
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_read_file_sync: file not found\n");
    return rc;
  }

  res = cfs_file_size(&p, &file_size, NULL);
  if (res != 0) {
    fclose(f);
    cfs_path_destroy(&p);
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_read_file_sync: cfs_file_size failed\n");
    return rc;
  }

  rc = CMP_MALLOC((size_t)file_size + 1, &buf);
  if (rc != CMP_SUCCESS) {
    fclose(f);
    cfs_path_destroy(&p);
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_read_file_sync: CMP_MALLOC buf failed\n");
    return rc;
  }

  if (file_size > 0) {
    read_bytes = fread(buf, 1, (size_t)file_size, f);
    if (read_bytes != (size_t)file_size) {
      free_rc = CMP_FREE(buf);
      if (free_rc != CMP_SUCCESS) {
        LOG_DEBUG("cmp_vfs_read_file_sync: CMP_FREE buf failed\n");
      }
      fclose(f);
      cfs_path_destroy(&p);
      rc = CMP_ERROR_NOT_FOUND;
      LOG_DEBUG("cmp_vfs_read_file_sync: fread failed\n");
      return rc;
    }
  }
  ((char *)buf)[file_size] = '\0';

  *out_buffer = buf;
  *out_size = (size_t)file_size;

  fclose(f);
  cfs_path_destroy(&p);
  return rc;
}

typedef struct {
  char *virtual_path;
  cmp_vfs_read_cb_t callback;
  void *user_data;
} cmp_vfs_async_read_ctx_t;

/**
 * @brief Documented function cmp_vfs_read_async_worker.
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void cmp_vfs_read_async_worker(void *arg) {
  cmp_vfs_async_read_ctx_t *ctx = (cmp_vfs_async_read_ctx_t *)arg;
  void *buffer = NULL;
  size_t size = 0;
  int res;
  int free_rc;

  if (ctx == NULL) {
    return;
  }

  res = cmp_vfs_read_file_sync(ctx->virtual_path, &buffer, &size);

  if (ctx->callback) {
    ctx->callback(res, buffer, size, ctx->user_data);
  } else if (res == CMP_SUCCESS && buffer != NULL) {
    free_rc = CMP_FREE(buffer);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_read_async_worker: CMP_FREE buffer failed\n");
    }
  }

  if (ctx->virtual_path != NULL) {
    free_rc = CMP_FREE(ctx->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_read_async_worker: CMP_FREE virtual_path failed\n");
    }
  }
  free_rc = CMP_FREE(ctx);
  if (free_rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_read_async_worker: CMP_FREE ctx failed\n");
  }
}

/**
 * @brief Documented function cmp_vfs_read_file_async.
 *
 * @param mod Parameter description.
 * @param virtual_path Parameter description.
 * @param callback Parameter description.
 * @param user_data Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_read_file_async(cmp_modality_t *mod, const char *virtual_path,
                            cmp_vfs_read_cb_t callback, void *user_data) {
  int rc = CMP_SUCCESS;
  cmp_vfs_async_read_ctx_t *ctx;
  size_t path_len;
  int free_rc;

  if (mod == NULL || virtual_path == NULL || !g_vfs_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_read_file_async: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_vfs_async_read_ctx_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_read_file_async: CMP_MALLOC ctx failed\n");
    return rc;
  }

  path_len = strlen(virtual_path);
  rc = CMP_MALLOC(path_len + 1, (void **)&ctx->virtual_path);
  if (rc != CMP_SUCCESS) {
    free_rc = CMP_FREE(ctx);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_read_file_async: CMP_FREE ctx failed\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_read_file_async: CMP_MALLOC virtual_path failed\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(ctx->virtual_path, path_len + 1, virtual_path);
#else
  strcpy(ctx->virtual_path, virtual_path);
#endif

  ctx->callback = callback;
  ctx->user_data = user_data;

  rc = cmp_modality_queue_task(mod, cmp_vfs_read_async_worker, ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_read_file_async: cmp_modality_queue_task failed\n");
    free_rc = CMP_FREE(ctx->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_read_file_async: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(ctx);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_read_file_async: CMP_FREE ctx failed\n");
    }
    return rc;
  }

  return rc;
}

struct cmp_vfs_watch {
  char *virtual_path;
  cmp_vfs_watch_cb_t callback;
  void *user_data;
  int running;
#if defined(_WIN32)
  void *thread_handle;
  void *dir_handle;
  void *stop_event;
#else
  pthread_t thread_handle;
  int stop_pipe[2];
#if defined(__linux__)
  int inotify_fd;
  int watch_wd;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||    \
    defined(__NetBSD__)
  int kq_fd;
  int fd;
#endif
#endif
};
#if defined(_WIN32)
/**
 * @brief Documented function cmp_math_vfs_watch_worker.
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */

static unsigned long __stdcall cmp_math_vfs_watch_worker(void *arg) {
  cmp_vfs_watch_t *watch = (cmp_vfs_watch_t *)arg;
  char buffer[4096];
  OVERLAPPED overlapped;
  unsigned long bytes_returned;
  void *handles[2];
  int result;
  FILE_NOTIFY_INFORMATION *info;

  memset(&overlapped, 0, sizeof(overlapped));
  overlapped.hEvent = CreateEventA(NULL, 1, 0, NULL);
  if (!overlapped.hEvent) {
    return 1;
  }

  handles[0] = watch->stop_event;
  handles[1] = overlapped.hEvent;

  while (watch->running) {
    if (!ReadDirectoryChangesW(watch->dir_handle, buffer, sizeof(buffer), 1,
                               FILE_NOTIFY_CHANGE_FILE_NAME |
                                   FILE_NOTIFY_CHANGE_DIR_NAME |
                                   FILE_NOTIFY_CHANGE_LAST_WRITE,
                               &bytes_returned, &overlapped, NULL)) {
      break;
    }

    result = WaitForMultipleObjects(2, (const HANDLE *)handles, 0, 0xFFFFFFFF);
    if (result == 0) {
      CancelIo(watch->dir_handle);
      break;
    } else if (result == 1) {
      info = (FILE_NOTIFY_INFORMATION *)buffer;
      for (;;) {
        watch->callback(watch->virtual_path, 1, watch->user_data);
        if (info->NextEntryOffset == 0) {
          break;
        }
        info =
            (FILE_NOTIFY_INFORMATION *)((char *)info + info->NextEntryOffset);
      }
      ResetEvent(overlapped.hEvent);
    } else {
      break;
    }
  }

  CloseHandle(overlapped.hEvent);
  return 0;
}
#elif defined(__linux__)
/**
 * @brief cmp_math_vfs_watch_worker
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void *cmp_math_vfs_watch_worker(void *arg) {
  cmp_vfs_watch_t *watch = (cmp_vfs_watch_t *)arg;
  union {
    char bytes[4096];
    struct inotify_event dummy;
  } buffer;
  int nfds, res;
  fd_set rfds;
  struct inotify_event *event;
  char *ptr;

  while (watch->running) {
    FD_ZERO(&rfds);
    FD_SET(watch->inotify_fd, &rfds);
    FD_SET(watch->stop_pipe[0], &rfds);
    nfds = (watch->inotify_fd > watch->stop_pipe[0] ? watch->inotify_fd
                                                    : watch->stop_pipe[0]) +
           1;

    res = select(nfds, &rfds, NULL, NULL, NULL);
    if (res < 0) {
      break;
    }

    if (FD_ISSET(watch->stop_pipe[0], &rfds)) {
      break;
    }

    if (FD_ISSET(watch->inotify_fd, &rfds)) {
      res = read(watch->inotify_fd, buffer.bytes, sizeof(buffer.bytes));
      if (res <= 0) {
        break;
      }
      ptr = buffer.bytes;
      while (ptr < buffer.bytes + res) {
        event = (struct inotify_event *)ptr;
        if (event->mask & (IN_MODIFY | IN_CREATE | IN_DELETE)) {
          watch->callback(watch->virtual_path, 1, watch->user_data);
        }
        ptr += sizeof(struct inotify_event) + event->len;
      }
    }
  }

  return NULL;
}
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||    \
    defined(__NetBSD__)
/**
 * @brief cmp_math_vfs_watch_worker
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void *cmp_math_vfs_watch_worker(void *arg) {
  cmp_vfs_watch_t *watch = (cmp_vfs_watch_t *)arg;
  struct kevent events[2];
  int res;
  int i;

  while (watch->running) {
    res = kevent(watch->kq_fd, NULL, 0, events, 2, NULL);
    if (res <= 0) {
      break;
    }

    for (i = 0; i < res; i++) {
      if (events[i].ident == (uintptr_t)watch->stop_pipe[0]) {
        return NULL;
      }
      if (events[i].ident == (uintptr_t)watch->fd) {
        watch->callback(watch->virtual_path, 1, watch->user_data);
      }
    }
  }

  return NULL;
}
#else
/**
 * @brief cmp_math_vfs_watch_worker
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void *cmp_math_vfs_watch_worker(void *arg) {
  cmp_vfs_watch_t *watch = (cmp_vfs_watch_t *)arg;
  char dummy;
  read(watch->stop_pipe[0], &dummy, 1);
  return NULL;
}
#endif

/**
 * @brief Documented function cmp_vfs_watch_path.
 *
 * @param virtual_path Parameter description.
 * @param callback Parameter description.
 * @param user_data Parameter description.
 * @param out_watch Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_watch_path(const char *virtual_path, cmp_vfs_watch_cb_t callback,
                       void *user_data, cmp_vfs_watch_t **out_watch) {
  int rc = CMP_SUCCESS;
  cmp_vfs_watch_t *watch;
  size_t path_len;
  cmp_string_t resolved_path;
  cfs_path p;
  const cfs_char_t *native_path;
  int free_rc;
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__)
  struct kevent change[2];
#endif

  if (virtual_path == NULL || callback == NULL || out_watch == NULL ||
      !g_vfs_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_watch_path: Invalid argument\n");
    return rc;
  }

  rc = cmp_vfs_resolve_path(virtual_path, &resolved_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_watch_path: cmp_vfs_resolve_path failed\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_vfs_watch_t), (void **)&watch);
  if (rc != CMP_SUCCESS) {
    rc = cmp_string_destroy(&resolved_path);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: cmp_string_destroy failed\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_watch_path: CMP_MALLOC watch failed\n");
    return rc;
  }

  memset(watch, 0, sizeof(*watch));

  path_len = strlen(virtual_path);
  rc = CMP_MALLOC(path_len + 1, (void **)&watch->virtual_path);
  if (rc != CMP_SUCCESS) {
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    free_rc = cmp_string_destroy(&resolved_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: cmp_string_destroy failed\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_vfs_watch_path: CMP_MALLOC virtual_path failed\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(watch->virtual_path, path_len + 1, virtual_path);
#else
  strcpy(watch->virtual_path, virtual_path);
#endif

  watch->callback = callback;
  watch->user_data = user_data;
  watch->running = 1;

  cfs_path_init(&p);
#if defined(_WIN32)
  {
    wchar_t *wpath = NULL;
    cfs_size_t req_len = 0;
    cfs_utf8_to_utf16(resolved_path.data, NULL, 0, &req_len);
    if (req_len > 0) {
      if (CMP_MALLOC((size_t)req_len * sizeof(wchar_t), (void **)&wpath) ==
          CMP_SUCCESS) {
        cfs_utf8_to_utf16(resolved_path.data, wpath, req_len, NULL);
        cfs_path_assign(&p, wpath);
        free_rc = CMP_FREE(wpath);
        if (free_rc != CMP_SUCCESS) {
          LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE wpath failed\n");
        }
      }
    }
  }
#else
  cfs_path_assign(&p, resolved_path.data);
#endif

  free_rc = cmp_string_destroy(&resolved_path);
  if (free_rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_watch_path: cmp_string_destroy failed\n");
  }
  cfs_path_c_str(&p, &native_path);
#if defined(_WIN32)
  watch->dir_handle = CreateFileW(
      (const wchar_t *)native_path, FILE_LIST_DIRECTORY,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

  if (watch->dir_handle == (void *)-1) {
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: CreateFileW failed\n");
    return rc;
  }

  watch->stop_event = CreateEventA(NULL, 1, 0, NULL);
  watch->thread_handle =
      CreateThread(NULL, 0, cmp_math_vfs_watch_worker, watch, 0, NULL);
#elif defined(__linux__)
  if (pipe(watch->stop_pipe) == -1) {
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: pipe failed\n");
    return rc;
  }

  watch->inotify_fd = inotify_init();
  if (watch->inotify_fd < 0) {
    close(watch->stop_pipe[0]);
    close(watch->stop_pipe[1]);
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: inotify_init failed\n");
    return rc;
  }

  watch->watch_wd = inotify_add_watch(watch->inotify_fd, native_path,
                                      IN_MODIFY | IN_CREATE | IN_DELETE);
  if (watch->watch_wd < 0) {
    close(watch->inotify_fd);
    close(watch->stop_pipe[0]);
    close(watch->stop_pipe[1]);
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: inotify_add_watch failed\n");
    return rc;
  }

  pthread_create(&watch->thread_handle, NULL, cmp_math_vfs_watch_worker, watch);
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||    \
    defined(__NetBSD__)
  if (pipe(watch->stop_pipe) == -1) {
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: pipe failed\n");
    return rc;
  }

  watch->kq_fd = kqueue();
  if (watch->kq_fd < 0) {
    close(watch->stop_pipe[0]);
    close(watch->stop_pipe[1]);
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: kqueue failed\n");
    return rc;
  }

  watch->fd = open(native_path, O_EVTONLY);
  if (watch->fd < 0) {
    close(watch->kq_fd);
    close(watch->stop_pipe[0]);
    close(watch->stop_pipe[1]);
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: open failed\n");
    return rc;
  }

  EV_SET(&change[0], watch->fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR,
         NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME |
             NOTE_REVOKE,
         0, 0);
  EV_SET(&change[1], watch->stop_pipe[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
         0);
  kevent(watch->kq_fd, change, 2, NULL, 0, NULL);

  pthread_create(&watch->thread_handle, NULL, cmp_math_vfs_watch_worker, watch);
#else
  if (pipe(watch->stop_pipe) == -1) {
    cfs_path_destroy(&p);
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE virtual_path failed\n");
    }
    free_rc = CMP_FREE(watch);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_watch_path: CMP_FREE watch failed\n");
    }
    rc = CMP_ERROR_NOT_FOUND;
    LOG_DEBUG("cmp_vfs_watch_path: pipe failed\n");
    return rc;
  }
  pthread_create(&watch->thread_handle, NULL, cmp_math_vfs_watch_worker, watch);
#endif

  cfs_path_destroy(&p);
  *out_watch = watch;
  return rc;
}

/**
 * @brief Documented function cmp_vfs_unwatch.
 *
 * @param watch Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_unwatch(cmp_vfs_watch_t *watch) {
  int rc = CMP_SUCCESS;
  int free_rc;
#if !defined(_WIN32)
  char dummy = 1;
#endif

  if (watch == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_unwatch: Invalid argument\n");
    return rc;
  }

  watch->running = 0;
#if defined(_WIN32)
  if (watch->stop_event) {
    SetEvent(watch->stop_event);
  }
  if (watch->thread_handle) {
    WaitForSingleObject(watch->thread_handle, 0xFFFFFFFF);
    CloseHandle(watch->thread_handle);
  }
  if (watch->stop_event) {
    CloseHandle(watch->stop_event);
  }
  if (watch->dir_handle && watch->dir_handle != (void *)-1) {
    CloseHandle(watch->dir_handle);
  }
#else
  write(watch->stop_pipe[1], &dummy, 1);
  pthread_join(watch->thread_handle, NULL);
  close(watch->stop_pipe[0]);
  close(watch->stop_pipe[1]);
#if defined(__linux__)
  if (watch->inotify_fd >= 0) {
    close(watch->inotify_fd);
  }
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||    \
    defined(__NetBSD__)
  if (watch->kq_fd >= 0) {
    close(watch->kq_fd);
  }
  if (watch->fd >= 0) {
    close(watch->fd);
  }
#endif
#endif

  if (watch->virtual_path != NULL) {
    free_rc = CMP_FREE(watch->virtual_path);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_vfs_unwatch: CMP_FREE virtual_path failed\n");
    }
  }

  free_rc = CMP_FREE(watch);
  if (free_rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_unwatch: CMP_FREE watch failed\n");
  }
  return rc;
}

/**
 * @brief Documented function cmp_vfs_get_standard_path.
 *
 * @param type Parameter description.
 * @param out_path Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_vfs_get_standard_path(int type, cmp_string_t *out_path) {
  int rc = CMP_SUCCESS;
  if (out_path == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_get_standard_path: Invalid argument\n");
    return rc;
  }

  rc = cmp_string_init(out_path);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_get_standard_path: cmp_string_init failed\n");
    return rc;
  }

  switch (type) {
  case 1: /* AppData */
#if defined(_WIN32)
  {
    char env_buf[512];
    if (GetEnvironmentVariableA("APPDATA", env_buf, sizeof(env_buf)) > 0) {
      rc = cmp_string_append(out_path, env_buf);
    } else {
      rc = cmp_string_append(out_path, "."); /* Fallback */
    }
  }
#else
  {
    const char *home = getenv("HOME");
    if (home != NULL) {
      rc = cmp_string_append(out_path, home);
      if (rc == CMP_SUCCESS) {
#if defined(__APPLE__)
        rc = cmp_string_append(out_path, "/Library/Application Support");
#else
        rc = cmp_string_append(out_path, "/.config");
#endif
      }
    } else {
      rc = cmp_string_append(out_path, "."); /* Fallback */
    }
  }
#endif
  break;
  case 2: /* Temp */
#if defined(_WIN32)
  {
    char env_buf[512];
    if (GetEnvironmentVariableA("TEMP", env_buf, sizeof(env_buf)) > 0) {
      rc = cmp_string_append(out_path, env_buf);
    } else {
      rc = cmp_string_append(out_path, "C:\\Temp"); /* Fallback */
    }
  }
#else
    rc = cmp_string_append(out_path, "/tmp");
#endif
  break;
  case 3: /* Cache */
#if defined(_WIN32)
  {
    char env_buf[512];
    if (GetEnvironmentVariableA("LOCALAPPDATA", env_buf, sizeof(env_buf)) > 0) {
      rc = cmp_string_append(out_path, env_buf);
    } else {
      rc = cmp_string_append(out_path, "."); /* Fallback */
    }
  }
#else
  {
    const char *home = getenv("HOME");
    if (home != NULL) {
      rc = cmp_string_append(out_path, home);
      if (rc == CMP_SUCCESS) {
#if defined(__APPLE__)
        rc = cmp_string_append(out_path, "/Library/Caches");
#else
        rc = cmp_string_append(out_path, "/.cache");
#endif
      }
    } else {
      rc = cmp_string_append(out_path, "/tmp"); /* Fallback */
    }
  }
#endif
  break;
  case 4: /* Documents */
#if defined(_WIN32)
  {
    char env_buf[512];
    if (GetEnvironmentVariableA("USERPROFILE", env_buf, sizeof(env_buf)) > 0) {
      rc = cmp_string_append(out_path, env_buf);
      if (rc == CMP_SUCCESS) {
        rc = cmp_string_append(out_path, "\\Documents");
      }
    } else {
      rc = cmp_string_append(out_path, "."); /* Fallback */
    }
  }
#else
  {
    const char *home = getenv("HOME");
    if (home != NULL) {
      rc = cmp_string_append(out_path, home);
      if (rc == CMP_SUCCESS) {
        rc = cmp_string_append(out_path, "/Documents");
      }
    } else {
      rc = cmp_string_append(out_path, "."); /* Fallback */
    }
  }
#endif
  break;
  case 5: /* Executable Directory */
#if defined(_WIN32)
  {
    char exe_buf[1024];
    unsigned long len = GetModuleFileNameA(NULL, exe_buf, sizeof(exe_buf));
    if (len > 0 && len < sizeof(exe_buf)) {
      /* Strip executable name to get directory */
      char *last_slash = strrchr(exe_buf, '\\');
      if (last_slash) {
        *last_slash = '\0';
      }
      rc = cmp_string_append(out_path, exe_buf);
    } else {
      rc = cmp_string_append(out_path, "."); /* Fallback */
    }
  }
#elif defined(__linux__) || defined(__CYGWIN__)
  {
    char exe_buf[1024];
    ssize_t len = readlink("/proc/self/exe", exe_buf, sizeof(exe_buf) - 1);
    if (len != -1) {
      char *last_slash;
      exe_buf[len] = '\0';
      last_slash = strrchr(exe_buf, '/');
      if (last_slash) {
        *last_slash = '\0';
      }
      rc = cmp_string_append(out_path, exe_buf);
    } else {
      rc = cmp_string_append(out_path, "."); /* Fallback */
    }
  }
#else
  {
    rc = cmp_string_append(out_path, "."); /* Fallback */
  }
#endif
  break;
  default:
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_vfs_get_standard_path: Unsupported type\n");
    return rc;
  }

  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_vfs_get_standard_path: cmp_string_append failed\n");
  }
  return rc;
}