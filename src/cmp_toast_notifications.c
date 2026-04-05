/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
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

int cmp_toast_manager_create(cmp_toast_manager_t **out_manager) {
  cmp_toast_manager_t *manager;

  if (!out_manager) {
    return CMP_ERROR_INVALID_ARG;
  }

  manager = (cmp_toast_manager_t *)malloc(sizeof(cmp_toast_manager_t));
  if (!manager) {
    return CMP_ERROR_OOM;
  }

  manager->capacity = 10;
  manager->count = 0;
  manager->active_toasts =
      (cmp_toast_t **)malloc(manager->capacity * sizeof(cmp_toast_t *));
  if (!manager->active_toasts) {
    free(manager);
    return CMP_ERROR_OOM;
  }

  *out_manager = manager;
  return CMP_SUCCESS;
}

int cmp_toast_manager_destroy(cmp_toast_manager_t *manager) {
  size_t i;
  if (!manager) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < manager->count; ++i) {
    free(manager->active_toasts[i]);
  }
  free(manager->active_toasts);
  free(manager);

  return CMP_SUCCESS;
}

int cmp_toast_manager_push(cmp_toast_manager_t *manager,
                           cmp_toast_level_t level, const char *message,
                           unsigned int duration_ms) {
  cmp_toast_t *toast;
  cmp_toast_t **new_array;

  if (!manager || !message) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (manager->count == manager->capacity) {
    manager->capacity *= 2;
    new_array = (cmp_toast_t **)realloc(
        manager->active_toasts, manager->capacity * sizeof(cmp_toast_t *));
    if (!new_array) {
      return CMP_ERROR_OOM;
    }
    manager->active_toasts = new_array;
  }

  toast = (cmp_toast_t *)malloc(sizeof(cmp_toast_t));
  if (!toast) {
    return CMP_ERROR_OOM;
  }

  toast->level = level;
  strncpy(toast->message, message, MAX_TOAST_LENGTH - 1);
  toast->message[MAX_TOAST_LENGTH - 1] = '\0';
  toast->duration_ms = duration_ms;
  toast->elapsed_ms = 0;

  manager->active_toasts[manager->count++] = toast;

  return CMP_SUCCESS;
}

int cmp_toast_manager_tick(cmp_toast_manager_t *manager,
                           unsigned int delta_time_ms) {
  size_t i, j;

  if (!manager) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < manager->count;) {
    manager->active_toasts[i]->elapsed_ms += delta_time_ms;

    if (manager->active_toasts[i]->elapsed_ms >=
        manager->active_toasts[i]->duration_ms) {
      /* Dismiss toast */
      free(manager->active_toasts[i]);

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

  return CMP_SUCCESS;
}

int cmp_toast_manager_get_active_count(const cmp_toast_manager_t *manager,
                                       size_t *out_count) {
  if (!manager || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_count = manager->count;
  return CMP_SUCCESS;
}
