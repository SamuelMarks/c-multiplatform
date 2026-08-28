/**
 * @file ui_toolbar_base.c
 * @brief Implementation of the toolbar base component.
 * @details Provides the core logic and memory management for toolbars.
 */
/* clang-format off */
#include "ui_toolbar_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_toolbar_base
 * @brief Internal implementation of the toolbar component.
 */
struct ui_toolbar_base {
  /* @brief The toolbar title string. */
  char *title; /**< title */
  /* @brief The toolbar layout mode. */
  enum ui_toolbar_mode mode; /**< mode */
  /* @brief The alignment mode. */
  enum ui_toolbar_alignment alignment; /**< alignment */
  /* @brief Data signal bound to the toolbar. */
  struct ui_signal *data_signal; /**< data_signal */
};

/**
 * @brief Creates a new toolbar base instance.
 * @param out_toolbar Pointer to receive the newly created toolbar base.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Destroys a toolbar base instance.
 * @param toolbar The toolbar base to destroy.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Sets the title of the toolbar.
 * @param toolbar The toolbar base.
 * @param title The string title to set.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Gets the title of the toolbar.
 * @param toolbar The toolbar base.
 * @param out_title Pointer to receive the string title.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Sets the layout mode of the toolbar.
 * @param toolbar The toolbar base.
 * @param mode The mode to set.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Gets the layout mode of the toolbar.
 * @param toolbar The toolbar base.
 * @param out_mode Pointer to receive the mode.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Sets the alignment of the toolbar.
 * @param toolbar The toolbar base.
 * @param alignment The alignment to set.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Gets the alignment of the toolbar.
 * @param toolbar The toolbar base.
 * @param out_alignment Pointer to receive the alignment.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
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

/**
 * @brief Binds a data signal to the toolbar widget.
 * @param widget The toolbar base.
 * @param signal The signal to bind.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t ui_toolbar_base_bind_data(struct ui_toolbar_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
