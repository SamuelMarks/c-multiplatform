/* clang-format off */
#include "cmp.h"

#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_clipboard {
  char *text_data;
};

/**
 * @brief cmp_clipboard_create
 *
 * @param out_clipboard Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_create(cmp_clipboard_t **out_clipboard) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_clipboard_t *clipboard = NULL;

  if (out_clipboard == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_clipboard_create: Invalid argument (out_clipboard=NULL): %s\n",
        err_str);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_clipboard_t), (void **)&clipboard);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_create: Out of memory: %s\n", err_str);
    return rc;
  }

  memset(clipboard, 0, sizeof(cmp_clipboard_t));
  *out_clipboard = clipboard;
  cmp_log_debug(
      "cmp_clipboard_create: Successfully created clipboard context\n");
  return rc;
}

/**
 * @brief cmp_clipboard_destroy
 *
 * @param clipboard Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_destroy(cmp_clipboard_t *clipboard) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (clipboard == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_destroy: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (clipboard->text_data != NULL) {
    rc = CMP_FREE(clipboard->text_data);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_clipboard_destroy: Failed to free text_data\n");
    }
  }

  rc = CMP_FREE(clipboard);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_clipboard_destroy: Failed to free clipboard\n");
  }

  cmp_log_debug(
      "cmp_clipboard_destroy: Successfully destroyed clipboard context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_clipboard_set_text
 *
 * @param clipboard Parameter description.
 * @param text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_set_text(cmp_clipboard_t *clipboard, const char *text) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t len;

  if (clipboard == NULL || text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_set_text: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (clipboard->text_data != NULL) {
    rc = CMP_FREE(clipboard->text_data);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_clipboard_set_text: Failed to free previous text_data\n");
    }
    clipboard->text_data = NULL;
  }

  len = strlen(text);
  rc = CMP_MALLOC(len + 1, (void **)&clipboard->text_data);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_set_text: Out of memory: %s\n", err_str);
    return rc;
  }

#if defined(_MSC_VER)
  if (strcpy_s(clipboard->text_data, len + 1, text) != 0) {
    cmp_log_debug("cmp_clipboard_set_text: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(clipboard->text_data, text);
#endif

  cmp_log_debug(
      "cmp_clipboard_set_text: Successfully copied text to internal buffer\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_clipboard_get_text
 *
 * @param clipboard Parameter description.
 * @param out_text Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_get_text(const cmp_clipboard_t *clipboard, char **out_text) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t len;

  if (clipboard == NULL || out_text == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_get_text: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (clipboard->text_data == NULL) {
    *out_text = NULL;
    cmp_log_debug("cmp_clipboard_get_text: Clipboard is empty\n");
    return CMP_SUCCESS;
  }

  len = strlen(clipboard->text_data);
  rc = CMP_MALLOC(len + 1, (void **)out_text);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_get_text: Out of memory: %s\n", err_str);
    return rc;
  }

#if defined(_MSC_VER)
  if (strcpy_s(*out_text, len + 1, clipboard->text_data) != 0) {
    cmp_log_debug("cmp_clipboard_get_text: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(*out_text, clipboard->text_data);
#endif

  cmp_log_debug(
      "cmp_clipboard_get_text: Retrieved text from internal buffer\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_clipboard_clear
 *
 * @param clipboard Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_clipboard_clear(cmp_clipboard_t *clipboard) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (clipboard == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_clipboard_clear: Invalid argument: %s\n", err_str);
    return rc;
  }

  if (clipboard->text_data != NULL) {
    rc = CMP_FREE(clipboard->text_data);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_clipboard_clear: Failed to free text_data\n");
    }
    clipboard->text_data = NULL;
  }

  cmp_log_debug("cmp_clipboard_clear: Cleared clipboard buffer\n");
  return CMP_SUCCESS;
}
