/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_clipboard {
  char *text_data;
};

int cmp_clipboard_create(cmp_clipboard_t **out_clipboard) {
  cmp_clipboard_t *clipboard;
  if (!out_clipboard) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_clipboard_t), (void **)&clipboard) != 0) {
    return CMP_ERROR_OOM;
  }
  memset(clipboard, 0, sizeof(cmp_clipboard_t));
  *out_clipboard = clipboard;
  return CMP_SUCCESS;
}

int cmp_clipboard_destroy(cmp_clipboard_t *clipboard) {
  if (!clipboard) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (clipboard->text_data) {
    CMP_FREE(clipboard->text_data);
  }
  CMP_FREE(clipboard);
  return CMP_SUCCESS;
}

int cmp_clipboard_set_text(cmp_clipboard_t *clipboard, const char *text) {
  size_t len;
  if (!clipboard || !text) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (clipboard->text_data) {
    CMP_FREE(clipboard->text_data);
    clipboard->text_data = NULL;
  }

  len = strlen(text);
  if (CMP_MALLOC(len + 1, (void **)&clipboard->text_data) != 0) {
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(clipboard->text_data, len + 1, text);
#else
  strcpy(clipboard->text_data, text);
#endif

  return CMP_SUCCESS;
}

int cmp_clipboard_get_text(const cmp_clipboard_t *clipboard, char **out_text) {
  size_t len;
  if (!clipboard || !out_text) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!clipboard->text_data) {
    *out_text = NULL;
    return CMP_SUCCESS;
  }

  len = strlen(clipboard->text_data);
  if (CMP_MALLOC(len + 1, (void **)out_text) != 0) {
    return CMP_ERROR_OOM;
  }

#if defined(_MSC_VER)
  strcpy_s(*out_text, len + 1, clipboard->text_data);
#else
  strcpy(*out_text, clipboard->text_data);
#endif

  return CMP_SUCCESS;
}

int cmp_clipboard_clear(cmp_clipboard_t *clipboard) {
  if (!clipboard) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (clipboard->text_data) {
    CMP_FREE(clipboard->text_data);
    clipboard->text_data = NULL;
  }
  return CMP_SUCCESS;
}
