/**
 * @file ui_tray_manager.c
 * @brief Implementation of the system tray manager component.
 */

/* clang-format off */
#include "../include/ui_tray_manager.h"
#include "ui_internal_mem.h"
#include <string.h>

#if defined(_WIN32)
#ifndef UI_WINAPI
#if defined(_MSC_VER)
/** @cond */
#define UI_WINAPI __stdcall
/** @endcond */
#elif defined(__GNUC__)
/** @cond */
#define UI_WINAPI __attribute__((stdcall))
/** @endcond */
#else
/** @cond */
#define UI_WINAPI
/** @endcond */
#endif
#endif
#endif
/* clang-format on */

/**
 * @struct ui_tray_manager
 * @struct ui_tray_manager
 * @brief Internal state for the system tray / app indicator instance.
 */
struct ui_tray_manager {
  struct ui_image_base *icon; /**< icon */
  char tooltip[256];          /**< tooltip */
  struct ui_menu_base *menu;  /**< menu */
  int is_visible;             /**< is_visible */

  /* Platform specific handles would go here */
#if defined(_WIN32)
  void *notify_icon_data; /**< notify_icon_data */
#endif
};

ui_error_t ui_tray_manager_create(struct ui_tray_manager **out_tray) {
  struct ui_tray_manager *tray = NULL;

  if (!out_tray) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tray = (struct ui_tray_manager *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_tray_manager));
  if (!tray) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(tray, 0, sizeof(struct ui_tray_manager));
  tray->is_visible = 0;
  tray->tooltip[0] = '\0';

  *out_tray = tray;
  return UI_ERROR_NONE;
}

ui_error_t ui_tray_manager_destroy(struct ui_tray_manager *tray) {
  if (!tray) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tray->is_visible) {
    {
      ui_error_t rc_cleanup = ui_tray_manager_hide(tray);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  C_MULTIPLATFORM_FREE(tray);
  return UI_ERROR_NONE;
}

ui_error_t ui_tray_manager_set_icon(struct ui_tray_manager *tray,
                                    struct ui_image_base *image) {
  if (!tray || !image) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tray->icon = image;
  /* Sync with native API if visible */
  return UI_ERROR_NONE;
}

ui_error_t ui_tray_manager_set_tooltip(struct ui_tray_manager *tray,
                                       const char *tooltip) {
  if (!tray || !tooltip) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strcpy_s(tray->tooltip, sizeof(tray->tooltip), tooltip);
#else
  strncpy(tray->tooltip, tooltip, sizeof(tray->tooltip) - 1);
  tray->tooltip[sizeof(tray->tooltip) - 1] = '\0';
#endif

  /* Sync with native API if visible */
  return UI_ERROR_NONE;
}

ui_error_t ui_tray_manager_set_context_menu(struct ui_tray_manager *tray,
                                            struct ui_menu_base *menu) {
  if (!tray || !menu) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  tray->menu = menu;
  /* Sync with native API if visible */
  return UI_ERROR_NONE;
}

ui_error_t ui_tray_manager_show(struct ui_tray_manager *tray) {
  if (!tray) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (tray->is_visible) {
    return UI_ERROR_NONE; /* Already visible */
  }

  tray->is_visible = 1;

  /* Call into native OS APIs here: Shell_NotifyIcon (Win32), NSStatusBar
   * (macOS), AppIndicator (Linux) */

  return UI_ERROR_NONE;
}

ui_error_t ui_tray_manager_hide(struct ui_tray_manager *tray) {
  if (!tray) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!tray->is_visible) {
    return UI_ERROR_NONE; /* Already hidden */
  }

  tray->is_visible = 0;

  /* Call into native OS APIs here to remove icon */

  return UI_ERROR_NONE;
}
