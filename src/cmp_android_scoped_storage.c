/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_android_storage {
  int is_active;
};

int cmp_android_storage_create(cmp_android_storage_t **out_storage) {
  cmp_android_storage_t *st;

  if (!out_storage)
    return CMP_ERROR_INVALID_ARG;

  st = (cmp_android_storage_t *)malloc(sizeof(cmp_android_storage_t));
  if (!st)
    return CMP_ERROR_OOM;

  st->is_active = 1;
  *out_storage = st;

  return CMP_SUCCESS;
}

int cmp_android_storage_destroy(cmp_android_storage_t *storage) {
  if (!storage)
    return CMP_ERROR_INVALID_ARG;

  free(storage);
  return CMP_SUCCESS;
}

int cmp_android_storage_request_tree_access(cmp_android_storage_t *storage,
                                            char **out_uri_string) {
  char *uri;
  size_t len;
  const char *mock_uri =
      "content://com.android.providers.downloads.documents/tree/downloads";

  if (!storage || !out_uri_string)
    return CMP_ERROR_INVALID_ARG;

  /* In a real implementation, we would query JNI to fire the Intent and yield
   * asynchronously. Since we just compile for C89 mock logic here on PC: */
  len = strlen(mock_uri);
  uri = (char *)malloc(len + 1);
  if (!uri)
    return CMP_ERROR_OOM;

  strcpy(uri, mock_uri);
  *out_uri_string = uri;

  return CMP_SUCCESS;
}

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
