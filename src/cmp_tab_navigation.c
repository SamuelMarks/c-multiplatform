/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_tab_nav {
  cmp_editor_tab_t **tabs;
  size_t count;
  size_t capacity;
  size_t active_index;
};

/**
 * @brief Creates a new tab navigation context.
 *
 * @param out_nav Pointer to store the newly created tab navigation context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tab_nav_create(cmp_tab_nav_t **out_nav) {
  int rc = CMP_SUCCESS;
  cmp_tab_nav_t *nav;
  if (!out_nav) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_tab_nav_t), (void **)&(nav));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  nav->capacity = 16;
  nav->count = 0;
  nav->active_index = (size_t)-1;
  rc = CMP_MALLOC(nav->capacity * sizeof(cmp_editor_tab_t *),
                  (void **)&nav->tabs);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(nav);
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  *out_nav = nav;
  return rc;
}

/**
 * @brief Destroys a tab navigation context and frees its resources.
 *
 * @param nav Pointer to the tab navigation context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tab_nav_destroy(cmp_tab_nav_t *nav) {
  int rc = CMP_SUCCESS;
  size_t i;
  if (!nav) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < nav->count; i++) {
    rc = CMP_FREE(nav->tabs[i]);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  rc = CMP_FREE(nav->tabs);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  rc = CMP_FREE(nav);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }

  return rc;
}

/**
 * @brief Adds a new tab to the tab navigation context.
 *
 * @param nav Pointer to the tab navigation context.
 * @param title The display title for the tab.
 * @param file_path The file path associated with the tab.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tab_nav_add_tab(cmp_tab_nav_t *nav, const char *title,
                        const char *file_path) {
  int rc = CMP_SUCCESS;
  cmp_editor_tab_t *tab;
  cmp_editor_tab_t **new_tabs;
  size_t i;

  if (!nav || !title || !file_path) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (nav->count == nav->capacity) {
    size_t new_cap = nav->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_editor_tab_t *), (void **)&new_tabs);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM\n");
      return CMP_ERROR_OOM;
    }
    memcpy(new_tabs, nav->tabs, nav->count * sizeof(cmp_editor_tab_t *));
    CMP_FREE(nav->tabs);
    nav->tabs = new_tabs;
    nav->capacity = new_cap;
  }

  rc = CMP_MALLOC(sizeof(cmp_editor_tab_t), (void **)&(tab));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
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

  return rc;
}

/**
 * @brief Closes a tab at the specified index.
 *
 * @param nav Pointer to the tab navigation context.
 * @param index The index of the tab to close.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tab_nav_close_tab(cmp_tab_nav_t *nav, size_t index) {
  int rc = CMP_SUCCESS;
  size_t i;
  if (!nav || index >= nav->count) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(nav->tabs[index]);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }

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

  return rc;
}

/**
 * @brief Moves a tab from one index to another.
 *
 * @param nav Pointer to the tab navigation context.
 * @param from_index The current index of the tab to move.
 * @param to_index The new index for the tab.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tab_nav_move_tab(cmp_tab_nav_t *nav, size_t from_index,
                         size_t to_index) {
  int rc = CMP_SUCCESS;
  cmp_editor_tab_t *tmp;
  size_t i;

  if (!nav || from_index >= nav->count || to_index >= nav->count) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (from_index == to_index) {
    return rc;
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

  return rc;
}

/**
 * @brief Gets the number of open tabs.
 *
 * @param nav Pointer to the tab navigation context.
 * @param out_count Pointer to store the count.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tab_nav_get_count(const cmp_tab_nav_t *nav, size_t *out_count) {
  int rc = CMP_SUCCESS;
  if (!nav || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_count = nav->count;

  return rc;
}

/**
 * @brief Gets a tab at the specified index.
 *
 * @param nav Pointer to the tab navigation context.
 * @param index The index of the tab to retrieve.
 * @param out_tab Pointer to store the retrieved tab pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tab_nav_get_tab(const cmp_tab_nav_t *nav, size_t index,
                        cmp_editor_tab_t **out_tab) {
  int rc = CMP_SUCCESS;
  if (!nav || !out_tab || index >= nav->count) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_tab = nav->tabs[index];

  return rc;
}
