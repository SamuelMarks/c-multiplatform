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

/**
 * @brief cmp_macos_features_create
 *
 * @param out_features Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_features_create(cmp_macos_features_t **out_features) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = NULL;

  if (!out_features) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_features_create: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(struct cmp_macos_features), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_macos_features_create: Out of memory\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->window_controls_visible = 1;
  ctx->has_document_proxy = 0;
  ctx->has_menu_bar_extra = 0;

  *out_features = (cmp_macos_features_t *)ctx;
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_features_destroy
 *
 * @param features_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_features_destroy(cmp_macos_features_t *features_opaque) {
  int rc;
  rc = CMP_SUCCESS;

  if (features_opaque) {
    rc = CMP_FREE(features_opaque);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_macos_features_destroy: CMP_FREE failed\n");
    }
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_set_window_controls_visible
 *
 * @param features_opaque Parameter description.
 * @param visible Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_set_window_controls_visible(cmp_macos_features_t *features_opaque,
                                          int visible) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_macos_set_window_controls_visible: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->window_controls_visible = visible;
  /* NSWindow.standardWindowButton */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_set_document_proxy
 *
 * @param features_opaque Parameter description.
 * @param file_url Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_set_document_proxy(cmp_macos_features_t *features_opaque,
                                 const char *file_url) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_document_proxy: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (file_url) {
    ctx->has_document_proxy = 1;
  } else {
    ctx->has_document_proxy = 0;
  }
  /* NSWindow.representedURL */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_set_menu_bar_extra
 *
 * @param features_opaque Parameter description.
 * @param enabled Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_set_menu_bar_extra(cmp_macos_features_t *features_opaque,
                                 int enabled) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_menu_bar_extra: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  ctx->has_menu_bar_extra = enabled;
  /* NSStatusBar.systemStatusBar.statusItemWithLength */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_invoke_print_panel
 *
 * @param features_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_invoke_print_panel(cmp_macos_features_t *features_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_invoke_print_panel: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* NSPrintOperation */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_setup_global_menu_bar
 *
 * @param features_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_setup_global_menu_bar(cmp_macos_features_t *features_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_setup_global_menu_bar: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  /* NSApplication.sharedApplication.mainMenu */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_setup_context_menu
 *
 * @param features_opaque Parameter description.
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_setup_context_menu(cmp_macos_features_t *features_opaque,
                                 void *node) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx || !node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_setup_context_menu: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  /* NSMenu, popUpMenuPositioningItem */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_set_hover_state
 *
 * @param features_opaque Parameter description.
 * @param node Parameter description.
 * @param is_hovered Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_set_hover_state(cmp_macos_features_t *features_opaque, void *node,
                              int is_hovered) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx || !node || is_hovered < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_hover_state: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  /* Visual highlight changes specific to macOS precision */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_set_window_shadow
 *
 * @param features_opaque Parameter description.
 * @param is_active Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_set_window_shadow(cmp_macos_features_t *features_opaque,
                                int is_active) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx || is_active < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_macos_set_window_shadow: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  /* NSWindow.hasShadow */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_macos_setup_toolbar_customization
 *
 * @param features_opaque Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_macos_setup_toolbar_customization(
    cmp_macos_features_t *features_opaque) {
  int rc;
  rc = CMP_SUCCESS;
  struct cmp_macos_features *ctx = (struct cmp_macos_features *)features_opaque;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_macos_setup_toolbar_customization: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  /* NSToolbar, allowsUserCustomization */
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
