/* clang-format off */
#include "ui_toolbar_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_toolbar_base {
  char *title;
  enum ui_toolbar_mode mode;
  enum ui_toolbar_alignment alignment;
  struct ui_signal *data_signal;
};

ui_error_t ui_toolbar_base_create(struct ui_toolbar_base **out_toolbar) {
  struct ui_toolbar_base *tb;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_toolbar) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  tb = (struct ui_toolbar_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_toolbar_base));
  if (!tb) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  tb->title = NULL;
  tb->mode = UI_TOOLBAR_MODE_STATIC;
  tb->alignment = UI_TOOLBAR_ALIGN_ROW;

  *out_toolbar = tb;

cleanup:
  return rc;
}

ui_error_t ui_toolbar_base_destroy(struct ui_toolbar_base *toolbar) {
  if (!toolbar) {
    return UI_ERROR_NONE;
  }
  if (toolbar->title) {
    C_MULTIPLATFORM_FREE(toolbar->title);
  }
  C_MULTIPLATFORM_FREE(toolbar);
  return UI_ERROR_NONE;
}

ui_error_t ui_toolbar_base_set_title(struct ui_toolbar_base *toolbar,
                                     const char *title) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t len;
  char *new_title = NULL;

  if (!toolbar || !title) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  len = strlen(title);
  new_title = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!new_title) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  (void)UI_STRCPY(new_title, len + 1, title);

  if (toolbar->title) {
    C_MULTIPLATFORM_FREE(toolbar->title);
  }
  toolbar->title = new_title;

cleanup:
  return rc;
}

ui_error_t ui_toolbar_base_get_title(const struct ui_toolbar_base *toolbar,
                                     const char **out_title) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!toolbar || !out_title) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  *out_title = toolbar->title;

cleanup:
  return rc;
}

ui_error_t ui_toolbar_base_set_mode(struct ui_toolbar_base *toolbar,
                                    enum ui_toolbar_mode mode) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!toolbar) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  toolbar->mode = mode;

cleanup:
  return rc;
}

ui_error_t ui_toolbar_base_get_mode(const struct ui_toolbar_base *toolbar,
                                    enum ui_toolbar_mode *out_mode) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!toolbar || !out_mode) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  *out_mode = toolbar->mode;

cleanup:
  return rc;
}

/** \brief ui_error */
ui_error_t ui_toolbar_base_set_alignment(struct ui_toolbar_base *toolbar,
                                         enum ui_toolbar_alignment alignment) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!toolbar) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  toolbar->alignment = alignment;

cleanup:
  return rc;
}

/** \brief ui_error */
ui_error_t
ui_toolbar_base_get_alignment(const struct ui_toolbar_base *toolbar,
                              enum ui_toolbar_alignment *out_alignment) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!toolbar || !out_alignment) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  *out_alignment = toolbar->alignment;

cleanup:
  return rc;
}

ui_error_t ui_toolbar_base_bind_data(struct ui_toolbar_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
