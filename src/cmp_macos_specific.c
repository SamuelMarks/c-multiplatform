/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_macos_features {
  int window_controls_visible;
  int has_document_proxy;
  int has_menu_bar_extra;
};

int cmp_macos_features_create(cmp_macos_features_t **out_features) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = NULL;

  if (!out_features) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_features_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_macos_features), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_macos_features_create: Out of memory\n");
    return rc;
  }

  ctx->window_controls_visible = 1;
  ctx->has_document_proxy = 0;
  ctx->has_menu_bar_extra = 0;

  *out_features = (cmp_macos_features_t *)ctx;
  return rc;
}

int cmp_macos_features_destroy(cmp_macos_features_t *features_opaque) {
  int rc = CMP_SUCCESS;

  if (features_opaque) {
    CMP_FREE(features_opaque);
  }
  return rc;
}

int cmp_macos_set_window_controls_visible(cmp_macos_features_t *features_opaque,
                                          int visible) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_macos_set_window_controls_visible: Invalid argument\n");
    return rc;
  }

  ctx->window_controls_visible = visible;
  /* NSWindow.standardWindowButton */
  return rc;
}

int cmp_macos_set_document_proxy(cmp_macos_features_t *features_opaque,
                                 const char *file_url) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_document_proxy: Invalid argument\n");
    return rc;
  }

  if (file_url) {
    ctx->has_document_proxy = 1;
  } else {
    ctx->has_document_proxy = 0;
  }
  /* NSWindow.representedURL */
  return rc;
}

int cmp_macos_set_menu_bar_extra(cmp_macos_features_t *features_opaque,
                                 int enabled) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_menu_bar_extra: Invalid argument\n");
    return rc;
  }

  ctx->has_menu_bar_extra = enabled;
  /* NSStatusBar.systemStatusBar.statusItemWithLength */
  return rc;
}

int cmp_macos_invoke_print_panel(cmp_macos_features_t *features_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_invoke_print_panel: Invalid argument\n");
    return rc;
  }

  /* NSPrintOperation */
  return rc;
}

int cmp_macos_setup_global_menu_bar(cmp_macos_features_t *features_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_setup_global_menu_bar: Invalid argument\n");
    return rc;
  }
  /* NSApplication.sharedApplication.mainMenu */
  return rc;
}

int cmp_macos_setup_context_menu(cmp_macos_features_t *features_opaque,
                                 void *node) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx || !node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_setup_context_menu: Invalid argument\n");
    return rc;
  }
  /* NSMenu, popUpMenuPositioningItem */
  return rc;
}

int cmp_macos_set_hover_state(cmp_macos_features_t *features_opaque, void *node,
                              int is_hovered) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx || !node || is_hovered < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_hover_state: Invalid argument\n");
    return rc;
  }
  /* Visual highlight changes specific to macOS precision */
  return rc;
}

int cmp_macos_set_window_shadow(cmp_macos_features_t *features_opaque,
                                int is_active) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx || is_active < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_window_shadow: Invalid argument\n");
    return rc;
  }
  /* NSWindow.hasShadow */
  return rc;
}

int cmp_macos_setup_toolbar_customization(
    cmp_macos_features_t *features_opaque) {
  int rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_macos_setup_toolbar_customization: Invalid argument\n");
    return rc;
  }
  /* NSToolbar, allowsUserCustomization */
  return rc;
}
