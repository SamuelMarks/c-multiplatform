/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_native_dialog {
  char *result_string;
  int is_showing;
};

int cmp_native_dialog_create(cmp_native_dialog_t **out_dialog) {
  int rc = CMP_SUCCESS;
  cmp_native_dialog_t *dialog = NULL;

  if (!out_dialog) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_native_dialog_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_native_dialog_t), (void **)&dialog);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_native_dialog_create: Out of memory\n");
    return rc;
  }

  memset(dialog, 0, sizeof(cmp_native_dialog_t));
  *out_dialog = dialog;
  return rc;
}

int cmp_native_dialog_destroy(cmp_native_dialog_t *dialog) {
  int rc = CMP_SUCCESS;

  if (!dialog) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_native_dialog_destroy: Invalid argument\n");
    return rc;
  }
  if (dialog->result_string) {
    CMP_FREE(dialog->result_string);
  }
  CMP_FREE(dialog);
  return rc;
}

int cmp_native_dialog_show(cmp_native_dialog_t *dialog,
                           cmp_dialog_type_t type) {
  int rc = CMP_SUCCESS;

  if (!dialog) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_native_dialog_show: Invalid argument\n");
    return rc;
  }
  (void)type;

  /* In a fully implemented renderer, this invokes the OS-specific native dialog
   * UI */
  dialog->is_showing = 1;
  return rc;
}

int cmp_native_dialog_get_result_string(const cmp_native_dialog_t *dialog,
                                        char **out_result) {
  int rc = CMP_SUCCESS;
  size_t len;

  if (!dialog || !out_result) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_native_dialog_get_result_string: Invalid argument\n");
    return rc;
  }

  if (!dialog->result_string) {
    *out_result = NULL;
    return rc;
  }

  len = strlen(dialog->result_string);
  rc = CMP_MALLOC(len + 1, (void **)out_result);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_native_dialog_get_result_string: Out of memory\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(*out_result, len + 1, dialog->result_string);
#else
  strcpy(*out_result, dialog->result_string);
#endif
  return rc;
}

int cmp_native_dialog_set_result_string(cmp_native_dialog_t *dialog,
                                        const char *result) {
  int rc = CMP_SUCCESS;
  size_t len;

  if (!dialog || !result) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_native_dialog_set_result_string: Invalid argument\n");
    return rc;
  }

  if (dialog->result_string) {
    CMP_FREE(dialog->result_string);
    dialog->result_string = NULL;
  }

  len = strlen(result);
  rc = CMP_MALLOC(len + 1, (void **)&dialog->result_string);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_native_dialog_set_result_string: Out of memory\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(dialog->result_string, len + 1, result);
#else
  strcpy(dialog->result_string, result);
#endif
  return rc;
}
