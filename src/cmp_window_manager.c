/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

struct cmp_window_manager {
  cmp_window_t **windows;
  unsigned int count;
  unsigned int capacity;
};

/**
 * @brief cmp_window_manager_create
 *
 * @param out_manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_create(cmp_window_manager_t **out_manager) {
  cmp_window_manager_t *mgr;
  int rc = CMP_SUCCESS;

  if (!out_manager) {
    LOG_DEBUG("cmp_window_manager_create: out_manager is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_window_manager_t), (void **)&mgr);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_manager_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  mgr->windows = NULL;
  mgr->count = 0;
  mgr->capacity = 0;

  *out_manager = mgr;
  return rc;
}

/**
 * @brief cmp_window_manager_destroy
 *
 * @param manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_destroy(cmp_window_manager_t *manager) {
  unsigned int i;
  int rc = CMP_SUCCESS;

  if (!manager) {
    LOG_DEBUG("cmp_window_manager_destroy: manager is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (manager->windows) {
    for (i = 0; i < manager->count; ++i) {
      if (manager->windows[i]) {
        rc = cmp_window_destroy(manager->windows[i]);
        if (rc != CMP_SUCCESS)
          LOG_DEBUG("cmp_window_manager_destroy: cmp_window_destroy failed\n");
      }
    }
    rc = CMP_FREE(manager->windows);
    if (rc != CMP_SUCCESS)
      LOG_DEBUG("cmp_window_manager_destroy: CMP_FREE windows failed\n");
  }

  rc = CMP_FREE(manager);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_window_manager_destroy: CMP_FREE failed\n");

    return rc;
  }
  return rc;
}

/**
 * @brief cmp_window_manager_add_window
 *
 * @param manager Parameter description.
 * @param window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_add_window(cmp_window_manager_t *manager,
                                  cmp_window_t *window) {
  cmp_window_t **new_windows;
  int rc = CMP_SUCCESS;

  if (!manager || !window) {
    LOG_DEBUG("cmp_window_manager_add_window: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (manager->count >= manager->capacity) {
    unsigned int new_capacity =
        manager->capacity == 0 ? 4 : manager->capacity * 2;
    rc = CMP_MALLOC(new_capacity * sizeof(cmp_window_t *),
                    (void **)&new_windows);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_window_manager_add_window: OOM\n");
      return CMP_ERROR_OOM;
    }

    if (manager->windows) {
      unsigned int i;
      for (i = 0; i < manager->count; ++i) {
        new_windows[i] = manager->windows[i];
      }
      rc = CMP_FREE(manager->windows);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG("cmp_window_manager_add_window: CMP_FREE failed\n");
    }
    manager->windows = new_windows;
    manager->capacity = new_capacity;
  }

  manager->windows[manager->count++] = window;
  return rc;
}

/**
 * @brief cmp_window_manager_remove_window
 *
 * @param manager Parameter description.
 * @param window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_remove_window(cmp_window_manager_t *manager,
                                     cmp_window_t *window) {
  int rc = CMP_SUCCESS;
  unsigned int i, j;
  if (!manager || !window) {
    LOG_DEBUG("cmp_window_manager_remove_window: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < manager->count; ++i) {
    if (manager->windows[i] == window) {
      for (j = i; j < manager->count - 1; ++j) {
        manager->windows[j] = manager->windows[j + 1];
      }
      manager->count--;
      return rc;
    }
  }

  LOG_DEBUG("cmp_window_manager_remove_window: Window not found\n");

  rc = CMP_ERROR_NOT_FOUND;
  return rc;
}

/**
 * @brief cmp_window_manager_get_window_count
 *
 * @param manager Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_get_window_count(const cmp_window_manager_t *manager,
                                        unsigned int *out_count) {
  int rc = CMP_SUCCESS;
  if (!manager || !out_count) {
    LOG_DEBUG("cmp_window_manager_get_window_count: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_count = manager->count;

  return rc;
}

/**
 * @brief cmp_window_manager_get_window_at
 *
 * @param manager Parameter description.
 * @param index Parameter description.
 * @param out_window Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_get_window_at(const cmp_window_manager_t *manager,
                                     unsigned int index,
                                     cmp_window_t **out_window) {
  int rc = CMP_SUCCESS;
  if (!manager || !out_window) {
    LOG_DEBUG("cmp_window_manager_get_window_at: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (index >= manager->count) {
    LOG_DEBUG("cmp_window_manager_get_window_at: Bounds error\n");
    return CMP_ERROR_BOUNDS;
  }

  *out_window = manager->windows[index];

  return rc;
}

/**
 * @brief cmp_window_manager_poll_events
 *
 * @param manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_poll_events(cmp_window_manager_t *manager) {
  unsigned int i;
  int rc = CMP_SUCCESS;
  if (!manager) {
    LOG_DEBUG("cmp_window_manager_poll_events: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < manager->count; ++i) {
    if (manager->windows[i]) {
      rc = cmp_window_poll_events(manager->windows[i]);
      if (rc != CMP_SUCCESS)
        LOG_DEBUG(
            "cmp_window_manager_poll_events: cmp_window_poll_events failed\n");
    }
  }
  return rc;
}

/**
 * @brief cmp_window_manager_should_close
 *
 * @param manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_window_manager_should_close(const cmp_window_manager_t *manager) {
  int rc = CMP_SUCCESS;
  unsigned int i;
  if (!manager)
    return 1;
  if (manager->count == 0)
    return 1;

  for (i = 0; i < manager->count; ++i) {
    if (manager->windows[i] && !cmp_window_should_close(manager->windows[i])) {
      return rc; /* At least one window wants to stay open */
    }
  }

  rc = 1; /* All windows want to close */
  return rc;
}