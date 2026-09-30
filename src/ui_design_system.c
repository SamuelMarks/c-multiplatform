/**
 * @file ui_design_system.c
 * @brief Implementation of the pluggable design system registry.
 */

/* clang-format off */
#include "ui_design_system.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/**
 * @struct ui_design_system_entry
 * @brief Internal linked list node storing a registered design system.
 */
struct ui_design_system_entry {
  char *name;                                   /**< Design system identifier */
  const struct ui_design_system_vtable *vtable; /**< Design system operations */
  struct ui_design_system_entry *next;          /**< Next node in linked list */
};

static struct ui_design_system_entry *g_design_systems_head = NULL;
static const char *g_active_design_system_name = NULL;

/**
 * @brief Duplicates a string using C_MULTIPLATFORM_MALLOC.
 *
 * @param src The source string to duplicate.
 * @param out_str Pointer to receive the allocated duplicate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t ui_design_system_strdup(const char *src, char **out_str) {
  size_t len;
  char *copy;

  len = strlen(src);
  copy = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!copy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(copy, len + 1, src);
#else
  memcpy(copy, src, len + 1);
#endif

  *out_str = copy;
  return UI_ERROR_NONE;
}

/**
 * @brief Registers a design system implementation with the global registry.
 *
 * @param name Unique name identifier for the design system.
 * @param vtable Pointer to the design system operations vtable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_design_system_register(const char *name,
                          const struct ui_design_system_vtable *vtable) {
  struct ui_design_system_entry *cur;
  struct ui_design_system_entry *entry;
  ui_error_t rc;

  if (!name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!vtable) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Check for duplicate registration */
  cur = g_design_systems_head;
  while (cur) {
    if (strcmp(cur->name, name) == 0) {
      return UI_ERROR_INVALID_ARGUMENT;
    }
    cur = cur->next;
  }

  entry = (struct ui_design_system_entry *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_design_system_entry));
  if (!entry) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_design_system_strdup(name, &entry->name);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(entry);
    return rc;
  }

  entry->vtable = vtable;
  entry->next = g_design_systems_head;
  g_design_systems_head = entry;

  return UI_ERROR_NONE;
}

/**
 * @brief Unregisters a design system by name.
 *
 * @param name Unique name identifier of the design system to unregister.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if not registered.
 */
ui_error_t ui_design_system_unregister(const char *name) {
  struct ui_design_system_entry *cur;
  struct ui_design_system_entry *prev;

  if (!name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cur = g_design_systems_head;
  prev = NULL;

  while (cur) {
    if (strcmp(cur->name, name) == 0) {
      if (prev) {
        prev->next = cur->next;
      } else {
        g_design_systems_head = cur->next;
      }

      if (g_active_design_system_name) {
        if (strcmp(g_active_design_system_name, name) == 0) {
          g_active_design_system_name = NULL;
        }
      }

      C_MULTIPLATFORM_FREE(cur->name);
      C_MULTIPLATFORM_FREE(cur);
      return UI_ERROR_NONE;
    }
    prev = cur;
    cur = cur->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Retrieves the registered vtable for a design system.
 *
 * @param name The name of the design system to query.
 * @param out_vtable Pointer to receive the vtable pointer.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if not registered.
 */
ui_error_t
ui_design_system_get(const char *name,
                     const struct ui_design_system_vtable **out_vtable) {
  struct ui_design_system_entry *cur;

  if (!name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!out_vtable) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cur = g_design_systems_head;
  while (cur) {
    if (strcmp(cur->name, name) == 0) {
      *out_vtable = cur->vtable;
      return UI_ERROR_NONE;
    }
    cur = cur->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Activates a registered design system on the given engine instance.
 *
 * @param engine Pointer to the UI engine instance.
 * @param name The name of the registered design system to activate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_design_system_set_active(struct ui_engine *engine,
                                       const char *name) {
  const struct ui_design_system_vtable *vtable;
  ui_error_t rc;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_design_system_get(name, &vtable);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (vtable->init) {
    rc = vtable->init(engine);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  g_active_design_system_name = name;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the name of the currently active design system.
 *
 * @param out_name Pointer to receive the constant string name of the active
 * system.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if none is active.
 */
ui_error_t ui_design_system_get_active(const char **out_name) {
  if (!out_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!g_active_design_system_name) {
    return UI_ERROR_NOT_FOUND;
  }

  *out_name = g_active_design_system_name;
  return UI_ERROR_NONE;
}

/**
 * @brief Resets and clears all registered design systems.
 *
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_design_system_registry_reset(void) {
  struct ui_design_system_entry *cur;
  struct ui_design_system_entry *next;

  cur = g_design_systems_head;
  while (cur) {
    next = cur->next;
    C_MULTIPLATFORM_FREE(cur->name);
    C_MULTIPLATFORM_FREE(cur);
    cur = next;
  }

  g_design_systems_head = NULL;
  g_active_design_system_name = NULL;
  return UI_ERROR_NONE;
}
