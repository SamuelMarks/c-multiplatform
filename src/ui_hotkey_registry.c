/* clang-format off */
#include "ui_hotkey_registry.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

#define MAX_HOTKEYS 128

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

/** \brief ui_error */
enum ui_error
ui_hotkey_registry_create(struct ui_hotkey_registry **out_registry) {
  struct ui_hotkey_registry *registry;
  int i;

  if (!out_registry) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  registry =
      (struct ui_hotkey_registry *)UI_MALLOC(sizeof(struct ui_hotkey_registry));
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

void ui_hotkey_registry_destroy(struct ui_hotkey_registry *registry) {
  if (!registry) {
    return;
  }
  UI_FREE(registry);
}

enum ui_error ui_hotkey_registry_register(struct ui_hotkey_registry *registry,
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

enum ui_error ui_hotkey_registry_unregister(struct ui_hotkey_registry *registry,
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

/** \brief ui_error */
enum ui_error
ui_hotkey_registry_process_event(struct ui_hotkey_registry *registry,
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

      registry->entries[i].callback(registry->entries[i].user_data);
      *out_handled = 1;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NONE;
}
