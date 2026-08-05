#ifndef UI_KEY_MANAGER_H
#define UI_KEY_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_event.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents a registered hotkey.
 */
struct ui_hotkey {
  int id;                 /**< Unique identifier for the hotkey */
  int key_code;           /**< Key code (e.g., from ui_key_code or ASCII) */
  unsigned int modifiers; /**< Bitmask of ui_modifier_flags */
  void (*callback)(
      void *user_data); /**< Callback triggered when hotkey is matched */
  void *user_data;      /**< User data passed to the callback */
};

/**
 * @brief Manages a collection of registered hotkeys.
 */
struct ui_key_manager {
  struct ui_hotkey *hotkeys; /**< Array of registered hotkeys */
  size_t count;              /**< Number of registered hotkeys */
  size_t capacity;           /**< Capacity of the hotkey array */
};

/**
 * @brief Initializes a key manager instance.
 *
 * @param manager Pointer to the manager to initialize.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_key_manager_init(struct ui_key_manager *manager);

/**
 * @brief Cleans up a key manager and frees its resources.
 *
 * @param manager Pointer to the manager to cleanup.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_key_manager_cleanup(struct ui_key_manager *manager);

/**
 * @brief Registers a new hotkey.
 *
 * @param manager Pointer to the key manager.
 * @param hotkey The hotkey definition to register.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_key_manager_register(struct ui_key_manager *manager,
                                   const struct ui_hotkey *hotkey);

/**
 * @brief Unregisters a hotkey by its ID.
 *
 * @param manager Pointer to the key manager.
 * @param id The ID of the hotkey to unregister.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if ID doesn't exist.
 */
ui_error_t ui_key_manager_unregister(struct ui_key_manager *manager, int id);

/**
 * @brief Processes a keyboard event and triggers callbacks for matching
 * hotkeys.
 *
 * @param manager Pointer to the key manager.
 * @param event The keyboard event to process.
 * @param out_handled Pointer to an integer set to non-zero if a hotkey was
 * triggered.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_key_manager_process_event(const struct ui_key_manager *manager,
                                        const struct ui_keyboard_event *event,
                                        int *out_handled);

/**
 * @brief Formats a hotkey into a human-readable string (e.g., "Ctrl+Shift+A").
 *
 * @param hotkey The hotkey to format.
 * @param out_buffer Pointer to the output string buffer.
 * @param buffer_size Size of the output buffer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_key_manager_format_hotkey(const struct ui_hotkey *hotkey,
                                        char *out_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_KEY_MANAGER_H */
