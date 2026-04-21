/* clang-format off */
#include "cmp.h"

#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_clipboard {
  char *text_data;
};

int cmp_clipboard_create(cmp_clipboard_t **out_clipboard) {
  int rc;
  cmp_clipboard_t *clipboard;
  if (!out_clipboard) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_clipboard_create: %s\n", cmp_strerror(rc));
    return rc;
  }
  rc = CMP_MALLOC(sizeof(cmp_clipboard_t), (void **)&clipboard);
  if (rc != CMP_SUCCESS) {
    if (rc == CMP_SUCCESS)
      rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_clipboard_create CMP_MALLOC: %s\n", cmp_strerror(rc));
    return rc;
  }
  memset(clipboard, 0, sizeof(cmp_clipboard_t));
  *out_clipboard = clipboard;
  return CMP_SUCCESS;
}

int cmp_clipboard_destroy(cmp_clipboard_t *clipboard) {
  int rc;
  if (!clipboard) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_clipboard_destroy: %s\n", cmp_strerror(rc));
    return rc;
  }
  if (clipboard->text_data) {
    CMP_FREE(clipboard->text_data);
  }
  CMP_FREE(clipboard);
  return CMP_SUCCESS;
}

int cmp_clipboard_set_text(cmp_clipboard_t *clipboard, const char *text) {
  int rc;
  size_t len;
  if (!clipboard || !text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_clipboard_set_text: %s\n", cmp_strerror(rc));
    return rc;
  }

  if (clipboard->text_data) {
    CMP_FREE(clipboard->text_data);
    clipboard->text_data = NULL;
  }

  len = strlen(text);
  rc = CMP_MALLOC(len + 1, (void **)&clipboard->text_data);
  if (rc != CMP_SUCCESS) {
    if (rc == CMP_SUCCESS)
      rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_clipboard_set_text CMP_MALLOC: %s\n", cmp_strerror(rc));
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(clipboard->text_data, len + 1, text);
#else
  strcpy(clipboard->text_data, text);
#endif

  return CMP_SUCCESS;
}

int cmp_clipboard_get_text(const cmp_clipboard_t *clipboard, char **out_text) {
  int rc;
  size_t len;
  if (!clipboard || !out_text) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_clipboard_get_text: %s\n", cmp_strerror(rc));
    return rc;
  }

  if (!clipboard->text_data) {
    *out_text = NULL;
    return CMP_SUCCESS;
  }

  len = strlen(clipboard->text_data);
  rc = CMP_MALLOC(len + 1, (void **)out_text);
  if (rc != CMP_SUCCESS) {
    if (rc == CMP_SUCCESS)
      rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_clipboard_get_text CMP_MALLOC: %s\n", cmp_strerror(rc));
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(*out_text, len + 1, clipboard->text_data);
#else
  strcpy(*out_text, clipboard->text_data);
#endif

  return CMP_SUCCESS;
}

int cmp_clipboard_clear(cmp_clipboard_t *clipboard) {
  int rc;
  if (!clipboard) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_clipboard_clear: %s\n", cmp_strerror(rc));
    return rc;
  }
  if (clipboard->text_data) {
    CMP_FREE(clipboard->text_data);
    clipboard->text_data = NULL;
  }
  return CMP_SUCCESS;
}
