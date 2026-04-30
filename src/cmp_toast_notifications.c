/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

#define MAX_TOAST_LENGTH 256

struct cmp_toast {
  cmp_toast_level_t level;
  char message[MAX_TOAST_LENGTH];
  unsigned int duration_ms;
  unsigned int elapsed_ms;
};

struct cmp_toast_manager {
  cmp_toast_t **active_toasts;
  size_t count;
  size_t capacity;
};

/**
 * @brief cmp_toast_manager_create
 *
 * @param out_manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toast_manager_create(cmp_toast_manager_t **out_manager) {
  int rc = CMP_SUCCESS;
  cmp_toast_manager_t *manager;

  if (!out_manager) {
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_toast_manager_t), (void **)&(manager));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  manager->capacity = 10;
  manager->count = 0;
  rc = CMP_MALLOC(manager->capacity * sizeof(cmp_toast_t *),
                  (void **)&manager->active_toasts);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(manager);
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  *out_manager = manager;
  return rc;
}

/**
 * @brief cmp_toast_manager_destroy
 *
 * @param manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toast_manager_destroy(cmp_toast_manager_t *manager) {
  int rc = CMP_SUCCESS;
  size_t i;
  if (!manager) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < manager->count; ++i) {
    rc = CMP_FREE(manager->active_toasts[i]);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  rc = CMP_FREE(manager->active_toasts);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  rc = CMP_FREE(manager);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }

  return rc;
}

/**
 * @brief cmp_toast_manager_push
 *
 * @param manager Parameter description.
 * @param level Parameter description.
 * @param message Parameter description.
 * @param duration_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toast_manager_push(cmp_toast_manager_t *manager,
                           cmp_toast_level_t level, const char *message,
                           unsigned int duration_ms) {
  int rc = CMP_SUCCESS;
  cmp_toast_t *toast;
  cmp_toast_t **new_array;

  if (!manager || !message) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (manager->count == manager->capacity) {
    size_t new_cap = manager->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_toast_t *), (void **)&new_array);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("OOM\n");
      return CMP_ERROR_OOM;
    }
    memcpy(new_array, manager->active_toasts,
           manager->count * sizeof(cmp_toast_t *));
    CMP_FREE(manager->active_toasts);
    manager->active_toasts = new_array;
    manager->capacity = new_cap;
  }

  rc = CMP_MALLOC(sizeof(cmp_toast_t), (void **)&(toast));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  toast->level = level;
  strncpy(toast->message, message, MAX_TOAST_LENGTH - 1);
  toast->message[MAX_TOAST_LENGTH - 1] = '\0';
  toast->duration_ms = duration_ms;
  toast->elapsed_ms = 0;

  manager->active_toasts[manager->count++] = toast;

  return rc;
}

/**
 * @brief cmp_toast_manager_tick
 *
 * @param manager Parameter description.
 * @param delta_time_ms Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toast_manager_tick(cmp_toast_manager_t *manager,
                           unsigned int delta_time_ms) {
  int rc = CMP_SUCCESS;
  size_t i, j;

  if (!manager) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < manager->count;) {
    manager->active_toasts[i]->elapsed_ms += delta_time_ms;

    if (manager->active_toasts[i]->elapsed_ms >=
        manager->active_toasts[i]->duration_ms) {
      /* Dismiss toast */
      rc = CMP_FREE(manager->active_toasts[i]);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Free failed\n");
      }

      /* Shift remaining elements */
      for (j = i; j < manager->count - 1; ++j) {
        manager->active_toasts[j] = manager->active_toasts[j + 1];
      }
      manager->count--;
    } else {
      /* Next */
      i++;
    }
  }

  return rc;
}

/**
 * @brief cmp_toast_manager_get_active_count
 *
 * @param manager Parameter description.
 * @param out_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_toast_manager_get_active_count(const cmp_toast_manager_t *manager,
                                       size_t *out_count) {
  int rc = CMP_SUCCESS;
  if (!manager || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_count = manager->count;

  return rc;
}
