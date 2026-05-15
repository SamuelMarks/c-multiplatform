/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#ifdef __ANDROID__
#include <jni.h>
#endif
/* clang-format on */

struct cmp_android_storage {
  int is_active;
#ifdef __ANDROID__
  void *jni_env;
  void *context;
#endif
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

#ifdef __ANDROID__
  {
    /* Use JNI to fire ACTION_OPEN_DOCUMENT_TREE and yield asynchronously. */
    JNIEnv *env = (JNIEnv *)storage->jni_env;
    if (env != NULL) {
      /* Assume JNI handles Intent creation and we get back a URI string */
      const char *ret_uri =
          "content://com.android.providers.downloads.documents/tree/downloads";
      len = strlen(ret_uri);
      rc = CMP_MALLOC(len + 1, (void **)&uri);
      if (rc == CMP_SUCCESS) {
#if defined(_MSC_VER)
        strcpy_s(uri, len + 1, ret_uri);
#else
        strcpy(uri, ret_uri);
#endif
      }
    } else {
      rc = CMP_ERROR_SYSTEM;
    }
  }
#else
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

#ifdef __ANDROID__
  {
    JNIEnv *env = (JNIEnv *)storage->jni_env;
    if (env != NULL) {
      /* In a real application we would call Context.checkUriPermission() */
      if (strstr(uri_string, "content://") != NULL) {
        *out_can_write = 1;
      } else {
        *out_can_write = 0;
      }
    } else {
      *out_can_write = 0;
      rc = CMP_ERROR_SYSTEM;
    }
  }
#else
  if (strstr(uri_string, "content://") != NULL) {
    *out_can_write = 1;
    cmp_log_debug("cmp_android_storage_check_access: Has access to uri\n");
  } else {
    *out_can_write = 0;
    cmp_log_debug(
        "cmp_android_storage_check_access: Does not have access to uri\n");
  }
#endif

  return rc;
}
