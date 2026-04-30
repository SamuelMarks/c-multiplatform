/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define MAX_WINDOWS 128

struct cmp_multi_window {
  char tab_id[256];
  int is_active;
};

static cmp_multi_window_t *g_windows[MAX_WINDOWS] = {0};
static int g_initialized = 0;

/**
 * @brief Initialize the multi-window manager.
 *
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_multi_window_init(void) {
  int rc = CMP_SUCCESS;
  int i;
  if (g_initialized) {
    return rc;
  }
  for (i = 0; i < MAX_WINDOWS; ++i) {
    g_windows[i] = NULL;
  }
  g_initialized = 1;
  return rc;
}

/**
 * @brief Cleanup the multi-window manager.
 *
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_multi_window_cleanup(void) {
  int rc = CMP_SUCCESS;
  int i;

  if (!g_initialized) {
    return rc;
  }
  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (g_windows[i] != NULL) {
      rc = CMP_FREE(g_windows[i]);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_multi_window_cleanup: CMP_FREE failed\n");
      }
      g_windows[i] = NULL;
    }
  }
  g_initialized = 0;
  return rc;
}

/**
 * @brief Tear off a tab into a new independent OS window.
 *
 * @param tab_id The ID of the tab to tear off.
 * @param out_window Pointer to store the created window handle.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_multi_window_tear_off(const char *tab_id,
                              cmp_multi_window_t **out_window) {
  int rc = CMP_SUCCESS;
  int i;
  cmp_multi_window_t *win;

  rc = CMP_SUCCESS;
  win = NULL;

  if (!g_initialized || tab_id == NULL || out_window == NULL) {
    LOG_DEBUG("Error in cmp_multi_window_tear_off: Invalid argument or "
              "uninitialized\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_multi_window_t), (void **)&win);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_multi_window_tear_off: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  rc = strncpy_s(win->tab_id, sizeof(win->tab_id), tab_id, _TRUNCATE);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_multi_window_tear_off: strncpy_s failed\n");
    rc = CMP_FREE(win);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_multi_window_tear_off: CMP_FREE failed during "
                "cleanup\n");
    }
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(win->tab_id, tab_id, sizeof(win->tab_id) - 1);
  win->tab_id[sizeof(win->tab_id) - 1] = '\0';
#endif
  win->is_active = 1;

  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (g_windows[i] == NULL) {
      g_windows[i] = win;
      *out_window = win;
      return rc;
    }
  }

  rc = CMP_FREE(win);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_multi_window_tear_off: CMP_FREE failed\n");
  }
  LOG_DEBUG("Error in cmp_multi_window_tear_off: Maximum windows reached\n");
  rc = CMP_ERROR_BOUNDS;
  return rc;
}

/**
 * @brief Merge an independent window back into the main application window as a
 * tab.
 *
 * @param window The window to merge back.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_multi_window_merge_back(cmp_multi_window_t *window) {
  int rc = CMP_SUCCESS;
  int i;

  if (!g_initialized || window == NULL) {
    LOG_DEBUG("Error in cmp_multi_window_merge_back: Invalid argument or "
              "uninitialized\n");
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (g_windows[i] == window) {
      rc = CMP_FREE(g_windows[i]);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_multi_window_merge_back: CMP_FREE failed\n");
      }
      g_windows[i] = NULL;
      return rc;
    }
  }

  LOG_DEBUG("Error in cmp_multi_window_merge_back: Window not found\n");
  rc = CMP_ERROR_NOT_FOUND;
  return rc;
}

/**
 * @brief Update all multi-windows.
 *
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_multi_window_update_all(void) {
  int rc = CMP_SUCCESS;
  if (!g_initialized) {
    LOG_DEBUG("Error in cmp_multi_window_update_all: Not initialized\n");
    return CMP_ERROR_INVALID_STATE;
  }
  /* In a real implementation, this would poll OS events for each window */
  return rc;
}
