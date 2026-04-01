/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_macos_features {
  int window_controls_visible;
  int has_document_proxy;
  int has_menu_bar_extra;
};

int cmp_macos_features_create(cmp_macos_features_t **out_features) {
  struct cmp_macos_features *ctx;
  if (!out_features)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_macos_features), (void **)&ctx) !=
      CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->window_controls_visible = 1;
  ctx->has_document_proxy = 0;
  ctx->has_menu_bar_extra = 0;

  *out_features = (cmp_macos_features_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_macos_features_destroy(cmp_macos_features_t *features_opaque) {
  if (features_opaque)
    CMP_FREE(features_opaque);
  return CMP_SUCCESS;
}

int cmp_macos_set_window_controls_visible(cmp_macos_features_t *features_opaque,
                                          int visible) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->window_controls_visible = visible;
  /* NSWindow.standardWindowButton */
  return CMP_SUCCESS;
}

int cmp_macos_set_document_proxy(cmp_macos_features_t *features_opaque,
                                 const char *file_url) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  if (file_url) {
    ctx->has_document_proxy = 1;
  } else {
    ctx->has_document_proxy = 0;
  }
  /* NSWindow.representedURL */
  return CMP_SUCCESS;
}

int cmp_macos_set_menu_bar_extra(cmp_macos_features_t *features_opaque,
                                 int enabled) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  ctx->has_menu_bar_extra = enabled;
  /* NSStatusBar.systemStatusBar.statusItemWithLength */
  return CMP_SUCCESS;
}

int cmp_macos_invoke_print_panel(cmp_macos_features_t *features_opaque) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;

  /* NSPrintOperation */
  return CMP_SUCCESS;
}
/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

int cmp_macos_setup_global_menu_bar(cmp_macos_features_t *features_opaque) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  /* NSApplication.sharedApplication.mainMenu */
  return CMP_SUCCESS;
}

int cmp_macos_setup_context_menu(cmp_macos_features_t *features_opaque,
                                 void *node) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx || !node)
    return CMP_ERROR_INVALID_ARG;
  /* NSMenu, popUpMenuPositioningItem */
  return CMP_SUCCESS;
}

int cmp_macos_set_hover_state(cmp_macos_features_t *features_opaque, void *node,
                              int is_hovered) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx || !node || is_hovered < 0)
    return CMP_ERROR_INVALID_ARG;
  /* Visual highlight changes specific to macOS precision */
  return CMP_SUCCESS;
}

int cmp_macos_set_window_shadow(cmp_macos_features_t *features_opaque,
                                int is_active) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx || is_active < 0)
    return CMP_ERROR_INVALID_ARG;
  /* NSWindow.hasShadow */
  return CMP_SUCCESS;
}

int cmp_macos_setup_toolbar_customization(
    cmp_macos_features_t *features_opaque) {
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  /* NSToolbar, allowsUserCustomization */
  return CMP_SUCCESS;
}
