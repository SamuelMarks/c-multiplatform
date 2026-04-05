/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_tab_nav {
  cmp_editor_tab_t **tabs;
  size_t count;
  size_t capacity;
  size_t active_index;
};

int cmp_tab_nav_create(cmp_tab_nav_t **out_nav) {
  cmp_tab_nav_t *nav;
  if (!out_nav) {
    return CMP_ERROR_INVALID_ARG;
  }

  nav = (cmp_tab_nav_t *)malloc(sizeof(cmp_tab_nav_t));
  if (!nav) {
    return CMP_ERROR_OOM;
  }

  nav->capacity = 16;
  nav->count = 0;
  nav->active_index = (size_t)-1;
  nav->tabs =
      (cmp_editor_tab_t **)malloc(nav->capacity * sizeof(cmp_editor_tab_t *));
  if (!nav->tabs) {
    free(nav);
    return CMP_ERROR_OOM;
  }

  *out_nav = nav;
  return CMP_SUCCESS;
}

int cmp_tab_nav_destroy(cmp_tab_nav_t *nav) {
  size_t i;
  if (!nav) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < nav->count; i++) {
    free(nav->tabs[i]);
  }
  free(nav->tabs);
  free(nav);

  return CMP_SUCCESS;
}

int cmp_tab_nav_add_tab(cmp_tab_nav_t *nav, const char *title,
                        const char *file_path) {
  cmp_editor_tab_t *tab;
  cmp_editor_tab_t **new_tabs;
  size_t i;

  if (!nav || !title || !file_path) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (nav->count == nav->capacity) {
    nav->capacity *= 2;
    new_tabs = (cmp_editor_tab_t **)realloc(
        nav->tabs, nav->capacity * sizeof(cmp_editor_tab_t *));
    if (!new_tabs) {
      return CMP_ERROR_OOM;
    }
    nav->tabs = new_tabs;
  }

  tab = (cmp_editor_tab_t *)malloc(sizeof(cmp_editor_tab_t));
  if (!tab) {
    return CMP_ERROR_OOM;
  }

  strncpy(tab->title, title, sizeof(tab->title) - 1);
  tab->title[sizeof(tab->title) - 1] = '\0';

  strncpy(tab->file_path, file_path, sizeof(tab->file_path) - 1);
  tab->file_path[sizeof(tab->file_path) - 1] = '\0';

  tab->is_active = 0;
  tab->is_modified = 0;

  /* Deactivate all other tabs */
  for (i = 0; i < nav->count; i++) {
    nav->tabs[i]->is_active = 0;
  }

  tab->is_active = 1;
  nav->active_index = nav->count;
  nav->tabs[nav->count++] = tab;

  return CMP_SUCCESS;
}

int cmp_tab_nav_close_tab(cmp_tab_nav_t *nav, size_t index) {
  size_t i;
  if (!nav || index >= nav->count) {
    return CMP_ERROR_INVALID_ARG;
  }

  free(nav->tabs[index]);

  for (i = index; i < nav->count - 1; i++) {
    nav->tabs[i] = nav->tabs[i + 1];
  }
  nav->count--;

  if (nav->count > 0) {
    /* Adjust active index if necessary */
    if (nav->active_index == index) {
      nav->active_index = (index > 0) ? index - 1 : 0;
      nav->tabs[nav->active_index]->is_active = 1;
    } else if (nav->active_index > index) {
      nav->active_index--;
    }
  } else {
    nav->active_index = (size_t)-1;
  }

  return CMP_SUCCESS;
}

int cmp_tab_nav_move_tab(cmp_tab_nav_t *nav, size_t from_index,
                         size_t to_index) {
  cmp_editor_tab_t *tmp;
  size_t i;

  if (!nav || from_index >= nav->count || to_index >= nav->count) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (from_index == to_index) {
    return CMP_SUCCESS;
  }

  tmp = nav->tabs[from_index];

  if (from_index < to_index) {
    /* Shift left */
    for (i = from_index; i < to_index; i++) {
      nav->tabs[i] = nav->tabs[i + 1];
    }
  } else {
    /* Shift right */
    for (i = from_index; i > to_index; i--) {
      nav->tabs[i] = nav->tabs[i - 1];
    }
  }

  nav->tabs[to_index] = tmp;

  /* Recompute active index since memory pointers shifted */
  for (i = 0; i < nav->count; i++) {
    if (nav->tabs[i]->is_active) {
      nav->active_index = i;
      break;
    }
  }

  return CMP_SUCCESS;
}

int cmp_tab_nav_get_count(const cmp_tab_nav_t *nav, size_t *out_count) {
  if (!nav || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_count = nav->count;
  return CMP_SUCCESS;
}

int cmp_tab_nav_get_tab(const cmp_tab_nav_t *nav, size_t index,
                        cmp_editor_tab_t **out_tab) {
  if (!nav || !out_tab || index >= nav->count) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_tab = nav->tabs[index];
  return CMP_SUCCESS;
}
