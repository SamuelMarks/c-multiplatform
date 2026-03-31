/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_window_manager {
  cmp_window_t **windows;
  unsigned int count;
  unsigned int capacity;
};

int cmp_window_manager_create(cmp_window_manager_t **out_manager) {
  cmp_window_manager_t *mgr;
  if (!out_manager)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_window_manager_t), (void **)&mgr) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  mgr->windows = NULL;
  mgr->count = 0;
  mgr->capacity = 0;

  *out_manager = mgr;
  return CMP_SUCCESS;
}

int cmp_window_manager_destroy(cmp_window_manager_t *manager) {
  unsigned int i;
  if (!manager)
    return CMP_ERROR_INVALID_ARG;

  if (manager->windows) {
    for (i = 0; i < manager->count; ++i) {
      if (manager->windows[i]) {
        cmp_window_destroy(manager->windows[i]);
      }
    }
    CMP_FREE(manager->windows);
  }

  CMP_FREE(manager);
  return CMP_SUCCESS;
}

int cmp_window_manager_add_window(cmp_window_manager_t *manager,
                                  cmp_window_t *window) {
  cmp_window_t **new_windows;
  if (!manager || !window)
    return CMP_ERROR_INVALID_ARG;

  if (manager->count >= manager->capacity) {
    unsigned int new_capacity =
        manager->capacity == 0 ? 4 : manager->capacity * 2;
    if (CMP_MALLOC(new_capacity * sizeof(cmp_window_t *),
                   (void **)&new_windows) != CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }

    if (manager->windows) {
      unsigned int i;
      for (i = 0; i < manager->count; ++i) {
        new_windows[i] = manager->windows[i];
      }
      CMP_FREE(manager->windows);
    }
    manager->windows = new_windows;
    manager->capacity = new_capacity;
  }

  manager->windows[manager->count++] = window;
  return CMP_SUCCESS;
}

int cmp_window_manager_remove_window(cmp_window_manager_t *manager,
                                     cmp_window_t *window) {
  unsigned int i, j;
  if (!manager || !window)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < manager->count; ++i) {
    if (manager->windows[i] == window) {
      for (j = i; j < manager->count - 1; ++j) {
        manager->windows[j] = manager->windows[j + 1];
      }
      manager->count--;
      return CMP_SUCCESS;
    }
  }

  return CMP_ERROR_NOT_FOUND;
}

int cmp_window_manager_get_window_count(const cmp_window_manager_t *manager,
                                        unsigned int *out_count) {
  if (!manager || !out_count)
    return CMP_ERROR_INVALID_ARG;
  *out_count = manager->count;
  return CMP_SUCCESS;
}

int cmp_window_manager_get_window_at(const cmp_window_manager_t *manager,
                                     unsigned int index,
                                     cmp_window_t **out_window) {
  if (!manager || !out_window)
    return CMP_ERROR_INVALID_ARG;
  if (index >= manager->count)
    return CMP_ERROR_BOUNDS;

  *out_window = manager->windows[index];
  return CMP_SUCCESS;
}

int cmp_window_manager_poll_events(cmp_window_manager_t *manager) {
  unsigned int i;
  if (!manager)
    return CMP_ERROR_INVALID_ARG;

  for (i = 0; i < manager->count; ++i) {
    if (manager->windows[i]) {
      cmp_window_poll_events(manager->windows[i]);
    }
  }
  return CMP_SUCCESS;
}

int cmp_window_manager_should_close(const cmp_window_manager_t *manager) {
  unsigned int i;
  if (!manager)
    return 1;
  if (manager->count == 0)
    return 1;

  for (i = 0; i < manager->count; ++i) {
    if (manager->windows[i] && !cmp_window_should_close(manager->windows[i])) {
      return 0; /* At least one window wants to stay open */
    }
  }
  return 1; /* All windows want to close */
}
