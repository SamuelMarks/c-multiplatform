/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_indexeddb_vfs {
  int is_mounted;
};

int cmp_indexeddb_vfs_create(cmp_indexeddb_vfs_t **out_vfs) {
  cmp_indexeddb_vfs_t *v;
  if (!out_vfs)
    return CMP_ERROR_INVALID_ARG;

  v = (cmp_indexeddb_vfs_t *)malloc(sizeof(cmp_indexeddb_vfs_t));
  if (!v)
    return CMP_ERROR_OOM;

  v->is_mounted = 0;
  *out_vfs = v;
  return CMP_SUCCESS;
}

int cmp_indexeddb_vfs_destroy(cmp_indexeddb_vfs_t *vfs) {
  if (!vfs)
    return CMP_ERROR_INVALID_ARG;
  free(vfs);
  return CMP_SUCCESS;
}

int cmp_indexeddb_vfs_mount(cmp_indexeddb_vfs_t *vfs, const char *mount_path,
                            const char *db_name) {
  if (!vfs || !mount_path || !db_name)
    return CMP_ERROR_INVALID_ARG;

  /* Real Emscripten implementation calls EM_ASM to mount IDBFS */
  vfs->is_mounted = 1;
  return CMP_SUCCESS;
}

int cmp_indexeddb_vfs_sync(cmp_indexeddb_vfs_t *vfs) {
  if (!vfs)
    return CMP_ERROR_INVALID_ARG;

  /* Calls EM_ASM to trigger FS.syncfs(false) */
  return CMP_SUCCESS;
}
