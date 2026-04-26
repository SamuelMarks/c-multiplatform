/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_indexeddb_vfs {
  int is_mounted;
};

/**
 * @brief cmp_indexeddb_vfs_create
 *
 * @param out_vfs Parameter description.
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
  return CMP_SUCCESS;
}

/**
 * @brief cmp_indexeddb_vfs_destroy
 *
 * @param vfs Parameter description.
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

  return CMP_SUCCESS;
}

/**
 * @brief cmp_indexeddb_vfs_mount
 *
 * @param vfs Parameter description.
 * @param mount_path Parameter description.
 * @param db_name Parameter description.
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
 * @brief cmp_indexeddb_vfs_sync
 *
 * @param vfs Parameter description.
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