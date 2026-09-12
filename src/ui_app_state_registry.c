/**
 * @file ui_app_state_registry.c
 * @brief Implementation of global application state registry.
 */

/* clang-format off */
#include "ui_app_state_registry.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @struct ui_app_state_entry
 * @brief Node in linked list of registered state signals.
 */
struct ui_app_state_entry {
  char *key;                       /**< Key identifier */
  struct ui_signal *signal;        /**< Bound signal */
  struct ui_app_state_entry *next; /**< Next entry */
};

/**
 * @struct ui_app_state_registry
 * @brief Application state registry container.
 */
struct ui_app_state_registry {
  struct ui_arena *arena;          /**< Optional arena used for allocations */
  struct ui_app_state_entry *head; /**< Head of entries linked list */
};

static struct ui_app_state_registry *g_global_app_state = NULL;

/**
 * @brief Creates a new application state registry.
 *
 * @param arena Memory arena to allocate from (or NULL for default heap).
 * @param out_registry Pointer to receive the allocated registry.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_app_state_registry_create(struct ui_arena *arena,
                             struct ui_app_state_registry **out_registry) {
  struct ui_app_state_registry *reg = NULL;
  ui_error_t rc;

  if (!out_registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (arena) {
    rc = ui_arena_alloc(arena, sizeof(struct ui_app_state_registry), 8,
                        (void **)&reg);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    reg = (struct ui_app_state_registry *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_app_state_registry));
    if (!reg) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  reg->arena = arena;
  reg->head = NULL;
  *out_registry = reg;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys an application state registry.
 *
 * @param registry The registry to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_app_state_registry_destroy(struct ui_app_state_registry *registry) {
  struct ui_app_state_entry *curr;

  if (!registry) {
    return UI_ERROR_NONE;
  }

  /* If allocated on an arena, the arena owns all memory */
  if (registry->arena) {
    return UI_ERROR_NONE;
  }

  curr = registry->head;
  while (curr) {
    struct ui_app_state_entry *next = curr->next;
    C_MULTIPLATFORM_FREE(curr->key);
    C_MULTIPLATFORM_FREE(curr);
    curr = next;
  }

  C_MULTIPLATFORM_FREE(registry);
  return UI_ERROR_NONE;
}

/**
 * @brief Registers a signal under a global string key.
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param signal The signal to register.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_app_state_registry_register_signal(struct ui_app_state_registry *registry,
                                      const char *key,
                                      struct ui_signal *signal) {
  struct ui_app_state_entry *curr;
  struct ui_app_state_entry *entry = NULL;
  size_t key_len;
  ui_error_t rc;

  if (!registry || !key || !signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = registry->head;
  while (curr) {
    if (strcmp(curr->key, key) == 0) {
      curr->signal = signal;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  key_len = strlen(key);
  if (registry->arena) {
    rc = ui_arena_alloc(registry->arena, sizeof(struct ui_app_state_entry), 8,
                        (void **)&entry);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = ui_arena_alloc(registry->arena, key_len + 1, 1, (void **)&entry->key);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    memcpy(entry->key, key, key_len + 1);
  } else {
    entry = (struct ui_app_state_entry *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_app_state_entry));
    if (!entry) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    entry->key = C_MULTIPLATFORM_STRDUP(key);
    if (!entry->key) {
      C_MULTIPLATFORM_FREE(entry);
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  entry->signal = signal;
  entry->next = registry->head;
  registry->head = entry;

  return UI_ERROR_NONE;
}

/**
 * @brief Looks up a registered signal by key.
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param out_signal Pointer to receive the found signal.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
ui_error_t
ui_app_state_registry_get_signal(const struct ui_app_state_registry *registry,
                                 const char *key,
                                 struct ui_signal **out_signal) {
  const struct ui_app_state_entry *curr;

  if (!registry || !key || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  curr = registry->head;
  while (curr) {
    if (strcmp(curr->key, key) == 0) {
      *out_signal = curr->signal;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * @brief Sets the value of a registered signal in the registry.
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param payload The new value payload.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t
ui_app_state_registry_set_value(struct ui_app_state_registry *registry,
                                const char *key,
                                union ui_signal_payload payload) {
  struct ui_signal *sig = NULL;
  ui_error_t rc;

  if (!registry || !key) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_app_state_registry_get_signal(registry, key, &sig);
  if (rc == UI_ERROR_NOT_FOUND) {
    rc = ui_signal_create(registry->arena, payload, UI_SIGNAL_TYPE_INT32, NULL,
                          NULL, UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    return ui_app_state_registry_register_signal(registry, key, sig);
  }

  return ui_signal_set(sig, payload);
}

/**
 * @brief Retrieves the current value of a registered signal in the registry.
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param out_payload Pointer to receive the signal payload.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t
ui_app_state_registry_get_value(const struct ui_app_state_registry *registry,
                                const char *key,
                                union ui_signal_payload *out_payload) {
  struct ui_signal *sig = NULL;
  ui_error_t rc;

  if (!registry || !key || !out_payload) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_app_state_registry_get_signal(registry, key, &sig);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_signal_get(sig, out_payload);
}

/**
 * @brief Retrieves the global singleton application state registry.
 *
 * @param out_registry Pointer to receive the global registry instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_app_state_registry_get_global(struct ui_app_state_registry **out_registry) {
  ui_error_t rc;

  if (!out_registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!g_global_app_state) {
    rc = ui_app_state_registry_create(NULL, &g_global_app_state);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  *out_registry = g_global_app_state;
  return UI_ERROR_NONE;
}

/**
 * @brief Shuts down and destroys the global singleton application state
 * registry.
 *
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_app_state_registry_shutdown_global(void) {
  if (g_global_app_state) {
    ui_app_state_registry_destroy(g_global_app_state);
    g_global_app_state = NULL;
  }
  return UI_ERROR_NONE;
}
