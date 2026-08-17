/**
 * \file ui_hotkey_registry.c
 * \brief Implementation of the hotkey registry.
 */
/* clang-format off */
#include "ui_hotkey_registry.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * \def MAX_HOTKEYS
 * \brief Maximum number of registered hotkeys.
 */
#define MAX_HOTKEYS 128

/**
 * \struct hotkey_entry
 * \brief Internal record of a registered hotkey.
 */
struct hotkey_entry {
  int id;
  struct ui_hotkey_chord chord;
  ui_hotkey_callback_t callback;
  void *user_data;
  int in_use;
};

/** \brief ui_hotkey_registry */
struct ui_hotkey_registry {
  struct hotkey_entry entries[MAX_HOTKEYS];
  int next_id;
};

/**
 * \brief Creates a new hotkey registry.
 * \param[out] out_registry Pointer to store the created registry.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_create(struct ui_hotkey_registry **out_registry) {
  struct ui_hotkey_registry *registry;
  int i;

  if (!out_registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  registry = (struct ui_hotkey_registry *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_hotkey_registry));
  if (!registry) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  registry->next_id = 1;
  for (i = 0; i < MAX_HOTKEYS; i++) {
    registry->entries[i].in_use = 0;
  }

  *out_registry = registry;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a hotkey registry.
 * \param[in,out] registry The registry to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_destroy(struct ui_hotkey_registry *registry) {
  if (!registry) {
    return UI_ERROR_NONE;
  }
  C_MULTIPLATFORM_FREE(registry);
  return UI_ERROR_NONE;
}

/**
 * \brief Registers a new hotkey callback for a specific key chord.
 * \param[in,out] registry The hotkey registry.
 * \param[in] chord The key chord to trigger the callback.
 * \param[in] callback The callback function.
 * \param[in] user_data User data to pass to the callback.
 * \param[out] out_id Pointer to store the assigned hotkey ID.
 * \return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_BOUNDS if registry is
 * full.
 */
ui_error_t ui_hotkey_registry_register(struct ui_hotkey_registry *registry,
                                       struct ui_hotkey_chord chord,
                                       ui_hotkey_callback_t callback,
                                       void *user_data, int *out_id) {
  int i;
  if (!registry || !callback || !out_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < MAX_HOTKEYS; i++) {
    if (!registry->entries[i].in_use) {
      registry->entries[i].id = registry->next_id++;
      registry->entries[i].chord = chord;
      registry->entries[i].callback = callback;
      registry->entries[i].user_data = user_data;
      registry->entries[i].in_use = 1;

      *out_id = registry->entries[i].id;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_OUT_OF_BOUNDS;
}

/**
 * \brief Unregisters a hotkey by its ID.
 * \param[in,out] registry The hotkey registry.
 * \param[in] id The ID of the hotkey to unregister.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_unregister(struct ui_hotkey_registry *registry,
                                         int id) {
  int i;
  if (!registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < MAX_HOTKEYS; i++) {
    if (registry->entries[i].in_use && registry->entries[i].id == id) {
      registry->entries[i].in_use = 0;
      return UI_ERROR_NONE;
    }
  }

  /* id not found is arguably not a hard error but we return none if already
   * cleared */
  return UI_ERROR_NONE;
}

/**
 * \brief Processes an incoming UI event and triggers matching hotkeys.
 * \param[in,out] registry The hotkey registry.
 * \param[in] event The UI event to evaluate.
 * \param[out] out_handled Set to 1 if a hotkey was triggered and handled the
 * event.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_process_event(struct ui_hotkey_registry *registry,
                                            const struct ui_event *event,
                                            int *out_handled) {
  int i;

  if (!registry || !event || !out_handled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_handled = 0;

  if (event->type != UI_EVENT_KEY_DOWN) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < MAX_HOTKEYS; i++) {
    if (registry->entries[i].in_use &&
        registry->entries[i].chord.key_code ==
            event->event_data.keyboard.key_code &&
        registry->entries[i].chord.modifiers ==
            event->event_data.keyboard.modifiers) {

      {
        ui_error_t cb_rc =
            registry->entries[i].callback(registry->entries[i].user_data);
        if (cb_rc != UI_ERROR_NONE)
          return cb_rc;
      }
      *out_handled = 1;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NONE;
}
