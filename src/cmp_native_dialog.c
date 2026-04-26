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

/**
 * @brief Create a native dialog.
 *
 * @param out_dialog Pointer to store the created dialog.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_native_dialog_create(cmp_native_dialog_t **out_dialog) {
  int rc;
  cmp_native_dialog_t *dialog;

  rc = CMP_SUCCESS;
  dialog = NULL;

  if (out_dialog == NULL) {
    LOG_DEBUG("Error in cmp_native_dialog_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_native_dialog_t), (void **)&dialog);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_native_dialog_create: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  memset(dialog, 0, sizeof(cmp_native_dialog_t));
  *out_dialog = dialog;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy a native dialog.
 *
 * @param dialog The dialog to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_native_dialog_destroy(cmp_native_dialog_t *dialog) {
  int rc;

  rc = CMP_SUCCESS;

  if (dialog == NULL) {
    LOG_DEBUG("Error in cmp_native_dialog_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (dialog->result_string != NULL) {
    rc = CMP_FREE(dialog->result_string);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_native_dialog_destroy: CMP_FREE failed for "
                "result_string\n");
    }
  }

  rc = CMP_FREE(dialog);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_native_dialog_destroy: CMP_FREE failed for dialog\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Show a native dialog.
 *
 * @param dialog The dialog to show.
 * @param type The type of dialog.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_native_dialog_show(cmp_native_dialog_t *dialog,
                           cmp_dialog_type_t type) {
  if (dialog == NULL) {
    LOG_DEBUG("Error in cmp_native_dialog_show: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  (void)type;

  /* In a fully implemented renderer, this invokes the OS-specific native dialog
   * UI */
  dialog->is_showing = 1;
  return CMP_SUCCESS;
}

/**
 * @brief Get the result string from a native dialog.
 *
 * @param dialog The dialog.
 * @param out_result Pointer to store the result string.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_native_dialog_get_result_string(const cmp_native_dialog_t *dialog,
                                        char **out_result) {
  int rc;
  size_t len;

  rc = CMP_SUCCESS;

  if (dialog == NULL || out_result == NULL) {
    LOG_DEBUG(
        "Error in cmp_native_dialog_get_result_string: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (dialog->result_string == NULL) {
    *out_result = NULL;
    return CMP_SUCCESS;
  }

  len = strlen(dialog->result_string);
  rc = CMP_MALLOC(len + 1, (void **)out_result);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_native_dialog_get_result_string: CMP_MALLOC failed "
              "(OOM)\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  rc = strcpy_s(*out_result, len + 1, dialog->result_string);
  if (rc != 0) {
    LOG_DEBUG(
        "Error in cmp_native_dialog_get_result_string: strcpy_s failed\n");
    rc = CMP_FREE(*out_result);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_native_dialog_get_result_string: CMP_FREE failed "
                "during cleanup\n");
    }
    *out_result = NULL;
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(*out_result, dialog->result_string);
#endif

  return CMP_SUCCESS;
}

/**
 * @brief Set the result string for a native dialog.
 *
 * @param dialog The dialog.
 * @param result The result string.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_native_dialog_set_result_string(cmp_native_dialog_t *dialog,
                                        const char *result) {
  int rc;
  size_t len;

  rc = CMP_SUCCESS;

  if (dialog == NULL || result == NULL) {
    LOG_DEBUG(
        "Error in cmp_native_dialog_set_result_string: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (dialog->result_string != NULL) {
    rc = CMP_FREE(dialog->result_string);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_native_dialog_set_result_string: CMP_FREE failed\n");
    }
    dialog->result_string = NULL;
  }

  len = strlen(result);
  rc = CMP_MALLOC(len + 1, (void **)&dialog->result_string);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_native_dialog_set_result_string: CMP_MALLOC failed "
              "(OOM)\n");
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  rc = strcpy_s(dialog->result_string, len + 1, result);
  if (rc != 0) {
    LOG_DEBUG(
        "Error in cmp_native_dialog_set_result_string: strcpy_s failed\n");
    rc = CMP_FREE(dialog->result_string);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_native_dialog_set_result_string: CMP_FREE failed "
                "during cleanup\n");
    }
    dialog->result_string = NULL;
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(dialog->result_string, result);
#endif

  return CMP_SUCCESS;
}
