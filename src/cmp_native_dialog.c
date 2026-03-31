/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_native_dialog {
  char *result_string;
  int is_showing;
};

int cmp_native_dialog_create(cmp_native_dialog_t **out_dialog) {
  cmp_native_dialog_t *dialog;
  if (!out_dialog) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_native_dialog_t), (void **)&dialog) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(dialog, 0, sizeof(cmp_native_dialog_t));
  *out_dialog = dialog;
  return CMP_SUCCESS;
}

int cmp_native_dialog_destroy(cmp_native_dialog_t *dialog) {
  if (!dialog) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (dialog->result_string) {
    CMP_FREE(dialog->result_string);
  }
  CMP_FREE(dialog);
  return CMP_SUCCESS;
}

int cmp_native_dialog_show(cmp_native_dialog_t *dialog,
                           cmp_dialog_type_t type) {
  if (!dialog) {
    return CMP_ERROR_INVALID_ARG;
  }
  (void)type;
  /* In a real implementation this would block or show async dialogs, depending
     on modality. For this stub, we just simulate showing it. */
  dialog->is_showing = 1;
  return CMP_SUCCESS;
}

int cmp_native_dialog_get_result_string(const cmp_native_dialog_t *dialog,
                                        char **out_result) {
  size_t len;
  if (!dialog || !out_result) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (!dialog->result_string) {
    *out_result = NULL;
    return CMP_SUCCESS;
  }
  len = strlen(dialog->result_string);
  if (CMP_MALLOC(len + 1, (void **)out_result) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(*out_result, len + 1, dialog->result_string);
#else
  strcpy(*out_result, dialog->result_string);
#endif
  return CMP_SUCCESS;
}

int cmp_native_dialog_set_result_string(cmp_native_dialog_t *dialog,
                                        const char *result) {
  size_t len;
  if (!dialog || !result) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (dialog->result_string) {
    CMP_FREE(dialog->result_string);
    dialog->result_string = NULL;
  }
  len = strlen(result);
  if (CMP_MALLOC(len + 1, (void **)&dialog->result_string) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(dialog->result_string, len + 1, result);
#else
  strcpy(dialog->result_string, result);
#endif
  return CMP_SUCCESS;
}
