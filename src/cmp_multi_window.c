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
 * @brief cmp_multi_window_init
 *
 * @return Returns 0 on success, or an error code on failure.
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
 * @brief cmp_multi_window_cleanup
 *
 * @return Returns 0 on success, or an error code on failure.
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
        LOG_DEBUG("Free failed\n");
      }
      g_windows[i] = NULL;
    }
  }
  g_initialized = 0;
  return rc;
}

/**
 * @brief cmp_multi_window_tear_off
 *
 * @param tab_id Parameter description.
 * @param out_window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_multi_window_tear_off(const char *tab_id,
                              cmp_multi_window_t **out_window) {
  int rc = CMP_SUCCESS;
  int i;
  cmp_multi_window_t *win = NULL;

  if (!g_initialized || tab_id == NULL || out_window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_multi_window_tear_off: Invalid argument or "
              "uninitialized\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_multi_window_t), (void **)&(win));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  if (win == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_multi_window_tear_off: Out of memory\n");
    return rc;
  }

#if defined(_MSC_VER)
  strncpy_s(win->tab_id, sizeof(win->tab_id), tab_id, _TRUNCATE);
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
    LOG_DEBUG("Free failed\n");
  }
  rc = CMP_ERROR_BOUNDS;
  LOG_DEBUG("Error in cmp_multi_window_tear_off: Maximum windows reached\n");
  return rc;
}

/**
 * @brief cmp_multi_window_merge_back
 *
 * @param window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_multi_window_merge_back(cmp_multi_window_t *window) {
  int rc = CMP_SUCCESS;
  int i;
  if (!g_initialized || window == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_multi_window_merge_back: Invalid argument or "
              "uninitialized\n");
    return rc;
  }

  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (g_windows[i] == window) {
      rc = CMP_FREE(g_windows[i]);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
      }
      g_windows[i] = NULL;
      return rc;
    }
  }

  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("Error in cmp_multi_window_merge_back: Window not found\n");
  return rc;
}

/**
 * @brief cmp_multi_window_update_all
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_multi_window_update_all(void) {
  int rc = CMP_SUCCESS;
  if (!g_initialized) {
    rc = CMP_ERROR_NOT_FOUND; /* Or invalid arg / invalid state */
    LOG_DEBUG("Error in cmp_multi_window_update_all: Not initialized\n");
    return rc;
  }
  /* In a real implementation, this would poll OS events for each window */
  return rc;
}
