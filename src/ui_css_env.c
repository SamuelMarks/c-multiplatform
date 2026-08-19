/* clang-format off */
#include "ui_css_env.h"
#include <stdlib.h>
#include <string.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_css_env_entry {
  char name[64];
  struct ui_css_value_ext *value;
  struct ui_css_env_entry *next;
};

/** \brief ui_css_env_manager */
struct ui_css_env_manager {
  struct ui_css_env_entry *head;
};

/** \brief ui_error */
ui_error_t ui_css_env_manager_create(struct ui_css_env_manager **out_manager) {
  struct ui_css_env_manager *manager;

  if (!out_manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  manager = (struct ui_css_env_manager *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_env_manager));
  if (!manager) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  manager->head = NULL;
  *out_manager = manager;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_css_env_manager_destroy.
 * @param manager Parameter manager.
 * @return Return value.
 */
ui_error_t ui_css_env_manager_destroy(struct ui_css_env_manager *manager) {
  struct ui_css_env_entry *current;
  struct ui_css_env_entry *next;

  if (!manager) {
    return UI_ERROR_NONE;
  }

  current = manager->head;
  while (current) {
    next = current->next;
    ui_css_value_ext_destroy(current->value);
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  C_MULTIPLATFORM_FREE(manager);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_css_env_manager_set.
 * @param manager Parameter manager.
 * @param name Parameter name.
 * @param value Parameter value.
 * @return Return value.
 */
ui_error_t ui_css_env_manager_set(struct ui_css_env_manager *manager,
                                  const char *name,
                                  struct ui_css_value_ext *value) {
  struct ui_css_env_entry *current;
  struct ui_css_env_entry *new_entry;

  if (!manager || !name || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Update if exists */
  current = manager->head;
  while (current) {
    if (strcmp(current->name, name) == 0) {
      ui_css_value_ext_destroy(current->value);
      current->value = value;
      return UI_ERROR_NONE;
    }
    current = current->next;
  }

  /* Add new entry */
  new_entry = (struct ui_css_env_entry *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_css_env_entry));
  if (!new_entry) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  UI_STRNCPY(new_entry->name, sizeof(new_entry->name), name,
             sizeof(new_entry->name) - 1);
  new_entry->name[sizeof(new_entry->name) - 1] = '\0';
  new_entry->value = value;

  new_entry->next = manager->head;
  manager->head = new_entry;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_css_env_manager_get(const struct ui_css_env_manager *manager,
                                  const char *name,
                                  const struct ui_css_value_ext **out_value) {
  struct ui_css_env_entry *current;

  if (!manager || !name || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = manager->head;
  while (current) {
    if (strcmp(current->name, name) == 0) {
      *out_value = current->value;
      return UI_ERROR_NONE;
    }
    current = current->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief ui_css_env_manager_remove.
 * @param manager Parameter manager.
 * @param name Parameter name.
 * @return Return value.
 */
ui_error_t ui_css_env_manager_remove(struct ui_css_env_manager *manager,
                                     const char *name) {
  struct ui_css_env_entry *current;
  struct ui_css_env_entry *prev = NULL;

  if (!manager || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = manager->head;
  while (current) {
    if (strcmp(current->name, name) == 0) {
      if (prev) {
        prev->next = current->next;
      } else {
        manager->head = current->next;
      }

      ui_css_value_ext_destroy(current->value);
      C_MULTIPLATFORM_FREE(current);
      return UI_ERROR_NONE;
    }
    prev = current;
    current = current->next;
  }

  return UI_ERROR_NOT_FOUND;
}
