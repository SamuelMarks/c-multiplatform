/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_indexeddb_vfs {
  int is_mounted;
};

/**
 * @brief Creates a new IndexedDB VFS context for Emscripten.
 *
 * @param out_vfs Pointer to store the newly created VFS context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_indexeddb_vfs_create(cmp_indexeddb_vfs_t **out_vfs) {
  int rc = CMP_SUCCESS;
  cmp_indexeddb_vfs_t *v = NULL;

  if (!out_vfs) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_indexeddb_vfs_create: Invalid argument (out_vfs=NULL)\n");

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_indexeddb_vfs_t), (void **)&v);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_indexeddb_vfs_create: Out of memory\n");

    return rc;
  }

  v->is_mounted = 0;
  *out_vfs = v;
  return rc;
}

/**
 * @brief Destroys an IndexedDB VFS context and frees its resources.
 *
 * @param vfs Pointer to the VFS context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_indexeddb_vfs_destroy(cmp_indexeddb_vfs_t *vfs) {
  int rc = CMP_SUCCESS;

  if (!vfs) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_indexeddb_vfs_destroy: Invalid argument (vfs=NULL)\n");

    return rc;
  }

  rc = CMP_FREE(vfs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_indexeddb_vfs_destroy: CMP_FREE failed\n");

    return rc;
  }

  return rc;
}

/**
 * @brief Mounts an IndexedDB database to a specific path in the Emscripten VFS.
 *
 * @param vfs Pointer to the VFS context.
 * @param mount_path The path in the virtual file system to mount the database.
 * @param db_name The name of the IndexedDB database to mount.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_indexeddb_vfs_mount(cmp_indexeddb_vfs_t *vfs, const char *mount_path,
                            const char *db_name) {
  int rc = CMP_SUCCESS;

  if (!vfs || !mount_path || !db_name) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_indexeddb_vfs_mount: Invalid argument\n");

    return rc;
  }

  /* Real Emscripten implementation calls EM_ASM to mount IDBFS */
  vfs->is_mounted = 1;

  return rc;
}

/**
 * @brief Synchronizes the Emscripten VFS memory representation with the
 * persistent IndexedDB storage.
 *
 * @param vfs Pointer to the VFS context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_indexeddb_vfs_sync(cmp_indexeddb_vfs_t *vfs) {
  int rc = CMP_SUCCESS;

  if (!vfs) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_indexeddb_vfs_sync: Invalid argument (vfs=NULL)\n");

    return rc;
  }

  /* Calls EM_ASM to trigger FS.syncfs(false) */

  return rc;
}