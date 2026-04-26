/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_android_storage {
  int is_active;
};

/**
 * @brief cmp_android_storage_create
 *
 * @param out_storage Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_storage_create(cmp_android_storage_t **out_storage) {
  cmp_android_storage_t *st;
  int rc;

  if (!out_storage) {
    LOG_DEBUG("cmp_android_storage_create: out_storage is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_android_storage_t), (void **)&st);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_android_storage_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  st->is_active = 1;
  *out_storage = st;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_android_storage_destroy
 *
 * @param storage Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_storage_destroy(cmp_android_storage_t *storage) {
  int rc;
  if (!storage) {
    LOG_DEBUG("cmp_android_storage_destroy: storage is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(storage);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_android_storage_destroy: CMP_FREE failed\n");
    return rc;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_android_storage_request_tree_access
 *
 * @param storage Parameter description.
 * @param out_uri_string Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_storage_request_tree_access(cmp_android_storage_t *storage,
                                            char **out_uri_string) {
  char *uri;
  size_t len;
  int rc;
  const char *mock_uri =
      "content://com.android.providers.downloads.documents/tree/downloads";

  if (!storage || !out_uri_string) {
    LOG_DEBUG("cmp_android_storage_request_tree_access: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* In a real implementation, we would query JNI to fire the Intent and yield
   * asynchronously. Since we just compile for C89 mock logic here on PC: */
  len = strlen(mock_uri);
  rc = CMP_MALLOC(len + 1, (void **)&uri);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_android_storage_request_tree_access: OOM\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  if (memcpy_s(uri, len + 1, mock_uri, len + 1) != 0) {
    LOG_DEBUG("cmp_android_storage_request_tree_access: memcpy_s failed\n");
    CMP_FREE(uri);
    return CMP_ERROR_GENERAL;
  }
#else
  memcpy(uri, mock_uri, len + 1);
#endif

  *out_uri_string = uri;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_android_storage_check_access
 *
 * @param storage Parameter description.
 * @param uri_string Parameter description.
 * @param out_can_write Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_storage_check_access(cmp_android_storage_t *storage,
                                     const char *uri_string,
                                     int *out_can_write) {
  if (!storage || !uri_string || !out_can_write)
    return CMP_ERROR_INVALID_ARG;

  /* Basic mock check: if uri contains "content", we "have" access */
  if (strstr(uri_string, "content://") != NULL) {
    *out_can_write = 1;
  } else {
    *out_can_write = 0;
  }

  return CMP_SUCCESS;
}
