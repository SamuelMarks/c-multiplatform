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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_android_storage_t *st = NULL;

  if (out_storage == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_storage_create: Invalid argument (out_storage=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_android_storage_t), (void **)&st);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_android_storage_create: Out of memory: %s\n", err_str);

    return rc;
  }

  st->is_active = 1;
  *out_storage = st;

  cmp_log_debug("cmp_android_storage_create: Successfully created android "
                "storage context\n");

  return rc;
}

/**
 * @brief cmp_android_storage_destroy
 *
 * @param storage Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_android_storage_destroy(cmp_android_storage_t *storage) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (storage == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_storage_destroy: Invalid argument (storage=NULL): %s\n",
        err_str);

    return rc;
  }

  CMP_FREE(storage);
  cmp_log_debug("cmp_android_storage_destroy: Successfully destroyed android "
                "storage context\n");

  return rc;
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  char *uri = NULL;
  size_t len;
  const char *mock_uri =
      "content://com.android.providers.downloads.documents/tree/downloads";

  if (storage == NULL || out_uri_string == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_storage_request_tree_access: Invalid argument: %s\n",
        err_str);

    return rc;
  }

  /* In a real implementation, we would query JNI to fire the Intent and yield
   * asynchronously. Since we just compile for C89 mock logic here on PC: */
  len = strlen(mock_uri);
  rc = CMP_MALLOC(len + 1, (void **)&uri);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_android_storage_request_tree_access: Out of memory: %s\n",
        err_str);

    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(uri, len + 1, mock_uri);
#else
  strcpy(uri, mock_uri);
#endif

  *out_uri_string = uri;
  cmp_log_debug(
      "cmp_android_storage_request_tree_access: Granted tree access to: %s\n",
      mock_uri);

  return rc;
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
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (rc != CMP_SUCCESS) {

    return rc;
  }

  if (storage == NULL || uri_string == NULL || out_can_write == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_android_storage_check_access: Invalid argument: %s\n",
                  err_str);

    return rc;
  }

  /* Basic mock check: if uri contains "content", we "have" access */
  if (strstr(uri_string, "content://") != NULL) {
    *out_can_write = 1;
    cmp_log_debug("cmp_android_storage_check_access: Has access to uri\n");
  } else {
    *out_can_write = 0;
    cmp_log_debug(
        "cmp_android_storage_check_access: Does not have access to uri\n");
  }

  return rc;
}
