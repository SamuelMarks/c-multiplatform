/* clang-format off */
#include "ui_key_manager.h"
#include "ui_internal_mem.h"
/* clang-format on */

#define INITIAL_CAPACITY 8

/**
 * @brief ui_key_manager_init.
 * @param manager Parameter manager.
 * @return Return value.
 */
ui_error_t ui_key_manager_init(struct ui_key_manager *manager) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!manager) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  manager->hotkeys = (struct ui_hotkey *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_hotkey) * INITIAL_CAPACITY);
  if (!manager->hotkeys) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  manager->count = 0;
  manager->capacity = INITIAL_CAPACITY;

cleanup:
  return rc;
}

/**
 * @brief ui_key_manager_cleanup.
 * @param manager Parameter manager.
 * @return Return value.
 */
ui_error_t ui_key_manager_cleanup(struct ui_key_manager *manager) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!manager) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (manager->hotkeys) {
    C_MULTIPLATFORM_FREE(manager->hotkeys);
    manager->hotkeys = NULL;
  }
  manager->count = 0;
  manager->capacity = 0;

cleanup:
  return rc;
}

/**
 * @brief ui_key_manager_register.
 * @param manager Parameter manager.
 * @param hotkey Parameter hotkey.
 * @return Return value.
 */
ui_error_t ui_key_manager_register(struct ui_key_manager *manager,
                                   const struct ui_hotkey *hotkey) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_hotkey *new_array;
  size_t new_capacity;

  if (!manager || !hotkey) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (manager->count >= manager->capacity) {
    new_capacity = manager->capacity * 2;
    if (new_capacity == 0) {
      new_capacity = INITIAL_CAPACITY;
    }
    if (new_capacity <= manager->count) {
      new_capacity = manager->count + 1;
    }
    new_array = (struct ui_hotkey *)C_MULTIPLATFORM_REALLOC(
        manager->hotkeys, sizeof(struct ui_hotkey) * new_capacity);
    if (!new_array) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
    manager->hotkeys = new_array;
    manager->capacity = new_capacity;
  }

  manager->hotkeys[manager->count] = *hotkey;
  manager->count++;

cleanup:
  return rc;
}

/**
 * @brief ui_key_manager_unregister.
 * @param manager Parameter manager.
 * @param id Parameter id.
 * @return Return value.
 */
ui_error_t ui_key_manager_unregister(struct ui_key_manager *manager, int id) {
  ui_error_t rc = UI_ERROR_NOT_FOUND;
  size_t i;
  size_t j;

  if (!manager) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  for (i = 0; i < manager->count; ++i) {
    if (manager->hotkeys[i].id == id) {
      for (j = i; j < manager->count - 1; ++j) {
        manager->hotkeys[j] = manager->hotkeys[j + 1];
      }
      manager->count--;
      rc = UI_ERROR_NONE;
      break;
    }
  }

cleanup:
  return rc;
}

/** \brief ui_error */
ui_error_t ui_key_manager_process_event(const struct ui_key_manager *manager,
                                        const struct ui_keyboard_event *event,
                                        int *out_handled) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t i;
  int handled = 0;

  if (!manager || !event || !out_handled) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  for (i = 0; i < manager->count; ++i) {
    const struct ui_hotkey *hk = &manager->hotkeys[i];
    if (hk->key_code == event->key_code && hk->modifiers == event->modifiers) {
      if (hk->callback) {
        hk->callback(hk->user_data);
      }
      handled = 1;
      break;
    }
  }

  *out_handled = handled;

cleanup:
  return rc;
}

/**
 * @brief ui_key_manager_format_hotkey.
 * @param hotkey Parameter hotkey.
 * @param out_buffer Parameter out_buffer.
 * @param buffer_size Parameter buffer_size.
 * @return Return value.
 */
ui_error_t ui_key_manager_format_hotkey(const struct ui_hotkey *hotkey,
                                        char *out_buffer, size_t buffer_size) {
  ui_error_t rc = UI_ERROR_NONE;
  char key_str[2];

  if (!hotkey || !out_buffer || buffer_size == 0) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  out_buffer[0] = '\0';

  if (hotkey->modifiers & UI_MODIFIER_CTRL) {
#if defined(_MSC_VER)
    strcat_s(out_buffer, buffer_size, "Ctrl+");
#else
    strcat(out_buffer, "Ctrl+");
#endif
  }
  if (hotkey->modifiers & UI_MODIFIER_SHIFT) {
#if defined(_MSC_VER)
    strcat_s(out_buffer, buffer_size, "Shift+");
#else
    strcat(out_buffer, "Shift+");
#endif
  }
  if (hotkey->modifiers & UI_MODIFIER_ALT) {
#if defined(_MSC_VER)
    strcat_s(out_buffer, buffer_size, "Alt+");
#else
    strcat(out_buffer, "Alt+");
#endif
  }
  if (hotkey->modifiers & UI_MODIFIER_SUPER) {
#if defined(_MSC_VER)
    strcat_s(out_buffer, buffer_size, "Super+");
#else
    strcat(out_buffer, "Super+");
#endif
  }

  if (hotkey->key_code == UI_KEY_SPACE) {
#if defined(_MSC_VER)
    strcat_s(out_buffer, buffer_size, "Space");
#else
    strcat(out_buffer, "Space");
#endif
  } else if (hotkey->key_code >= 'A' && hotkey->key_code <= 'Z') {
    key_str[0] = (char)hotkey->key_code;
    key_str[1] = '\0';
#if defined(_MSC_VER)
    strcat_s(out_buffer, buffer_size, key_str);
#else
    strcat(out_buffer, key_str);
#endif
  } else {
#if defined(_MSC_VER)
    strcat_s(out_buffer, buffer_size, "Key");
#else
    strcat(out_buffer, "Key");
#endif
  }

cleanup:
  return rc;
}
