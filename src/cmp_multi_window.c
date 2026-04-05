/* clang-format off */
#include "cmp.h"
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

int cmp_multi_window_init(void) {
  int i;
  if (g_initialized) {
    return 0;
  }
  for (i = 0; i < MAX_WINDOWS; ++i) {
    g_windows[i] = NULL;
  }
  g_initialized = 1;
  return 0;
}

int cmp_multi_window_cleanup(void) {
  int i;
  if (!g_initialized) {
    return 0;
  }
  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (g_windows[i] != NULL) {
      free(g_windows[i]);
      g_windows[i] = NULL;
    }
  }
  g_initialized = 0;
  return 0;
}

int cmp_multi_window_tear_off(const char *tab_id,
                              cmp_multi_window_t **out_window) {
  int i;
  cmp_multi_window_t *win = NULL;

  if (!g_initialized || tab_id == NULL || out_window == NULL) {
    return 1;
  }

  win = (cmp_multi_window_t *)malloc(sizeof(cmp_multi_window_t));
  if (win == NULL) {
    return 1;
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
      return 0;
    }
  }

  free(win);
  return 1; /* Maximum windows reached */
}

int cmp_multi_window_merge_back(cmp_multi_window_t *window) {
  int i;
  if (!g_initialized || window == NULL) {
    return 1;
  }

  for (i = 0; i < MAX_WINDOWS; ++i) {
    if (g_windows[i] == window) {
      free(g_windows[i]);
      g_windows[i] = NULL;
      return 0;
    }
  }

  return 1; /* Window not found */
}

int cmp_multi_window_update_all(void) {
  if (!g_initialized) {
    return 1;
  }
  /* In a real implementation, this would poll OS events for each window */
  return 0;
}
