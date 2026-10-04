/**
 * @file md2_overlays.c
 * @brief Implementation of Material Design 2 Overlays and Feedback.
 */

/* clang-format off */
#include "material2/md2_overlays.h"
#include <stdlib.h>
/* clang-format on */

struct md2_dialog {
  struct ui_dialog_base *base;
};

ui_error_t md2_dialog_create(struct md2_dialog **out_dialog) {
  struct md2_dialog *dialog;
  ui_error_t rc;

  if (out_dialog == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog = (struct md2_dialog *)malloc(sizeof(struct md2_dialog));
  if (dialog == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_dialog_base_create(&dialog->base);
  if (rc != UI_ERROR_NONE) {
    free(dialog);
    return rc;
  }

  *out_dialog = dialog;
  return UI_ERROR_NONE;
}

ui_error_t md2_dialog_destroy(struct md2_dialog *dialog) {
  if (dialog == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_dialog_base_destroy(dialog->base);
  free(dialog);
  return UI_ERROR_NONE;
}

ui_error_t md2_dialog_get_base(struct md2_dialog *dialog,
                               struct ui_dialog_base **out_base) {
  if (dialog == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = dialog->base;
  return UI_ERROR_NONE;
}

struct md2_bottom_sheet {
  struct ui_bottom_sheet_base *base;
};

ui_error_t md2_bottom_sheet_create(struct md2_bottom_sheet **out_sheet) {
  struct md2_bottom_sheet *sheet;
  ui_error_t rc;

  if (out_sheet == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet = (struct md2_bottom_sheet *)malloc(sizeof(struct md2_bottom_sheet));
  if (sheet == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_bottom_sheet_base_create(&sheet->base);
  if (rc != UI_ERROR_NONE) {
    free(sheet);
    return rc;
  }

  *out_sheet = sheet;
  return UI_ERROR_NONE;
}

ui_error_t md2_bottom_sheet_destroy(struct md2_bottom_sheet *sheet) {
  if (sheet == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_bottom_sheet_base_destroy(sheet->base);
  free(sheet);
  return UI_ERROR_NONE;
}

ui_error_t md2_bottom_sheet_get_base(struct md2_bottom_sheet *sheet,
                                     struct ui_bottom_sheet_base **out_base) {
  if (sheet == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = sheet->base;
  return UI_ERROR_NONE;
}

struct md2_snackbar {
  struct ui_snackbar_base *base;
};

ui_error_t md2_snackbar_create(struct ui_timer *timer,
                               struct ui_overlay_director *director,
                               struct md2_snackbar **out_snackbar) {
  struct md2_snackbar *snackbar;
  ui_error_t rc;

  if (out_snackbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  snackbar = (struct md2_snackbar *)malloc(sizeof(struct md2_snackbar));
  if (snackbar == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_snackbar_base_create(timer, director, &snackbar->base);
  if (rc != UI_ERROR_NONE) {
    free(snackbar);
    return rc;
  }

  *out_snackbar = snackbar;
  return UI_ERROR_NONE;
}

ui_error_t md2_snackbar_destroy(struct md2_snackbar *snackbar) {
  if (snackbar == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_snackbar_base_destroy(snackbar->base);
  free(snackbar);
  return UI_ERROR_NONE;
}

ui_error_t md2_snackbar_get_base(struct md2_snackbar *snackbar,
                                 struct ui_snackbar_base **out_base) {
  if (snackbar == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = snackbar->base;
  return UI_ERROR_NONE;
}

struct md2_tabs {
  struct ui_tabs_base *base;
};

ui_error_t md2_tabs_create(struct md2_tabs **out_tabs) {
  struct md2_tabs *tabs;
  ui_error_t rc;

  if (out_tabs == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tabs = (struct md2_tabs *)malloc(sizeof(struct md2_tabs));
  if (tabs == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_tabs_base_create(&tabs->base);
  if (rc != UI_ERROR_NONE) {
    free(tabs);
    return rc;
  }

  *out_tabs = tabs;
  return UI_ERROR_NONE;
}

ui_error_t md2_tabs_destroy(struct md2_tabs *tabs) {
  if (tabs == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_tabs_base_destroy(tabs->base);
  free(tabs);
  return UI_ERROR_NONE;
}

ui_error_t md2_tabs_get_base(struct md2_tabs *tabs,
                             struct ui_tabs_base **out_base) {
  if (tabs == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = tabs->base;
  return UI_ERROR_NONE;
}
