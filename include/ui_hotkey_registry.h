/* clang-format off */
#ifndef UI_HOTKEY_REGISTRY_H
#define UI_HOTKEY_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_event.h"

/* clang-format on */

struct ui_hotkey_registry;

/**
 * @brief Callback invoked when a hotkey is triggered.
 */
typedef ui_error_t (*ui_hotkey_callback_t)(void *user_data);

/**
 * @brief Represents a registered hotkey chord.
 */
struct ui_hotkey_chord {
  int key_code;
  unsigned int modifiers;
};

/**
 * @brief Creates a new hotkey registry.
 *
 * @param out_registry Pointer to receive the allocated registry.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_create(struct ui_hotkey_registry **out_registry);

/**
 * @brief Destroys a hotkey registry.
 *
 * @param registry The registry to destroy.
 */
ui_error_t ui_hotkey_registry_destroy(struct ui_hotkey_registry *registry);

/**
 * @brief Registers a hotkey chord with a callback.
 *
 * @param registry The registry.
 * @param chord The chord to register (key + modifiers).
 * @param callback The function to invoke.
 * @param user_data Opaque user data for the callback.
 * @param out_id Pointer to receive a unique registration ID for unregistering.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_register(struct ui_hotkey_registry *registry,
                                       struct ui_hotkey_chord chord,
                                       ui_hotkey_callback_t callback,
                                       void *user_data, int *out_id);

/**
 * @brief Unregisters a previously registered hotkey chord.
 *
 * @param registry The registry.
 * @param id The unique registration ID.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_unregister(struct ui_hotkey_registry *registry,
                                         int id);

/**
 * @brief Processes an incoming event. If the event matches a registered hotkey,
 * the callback is invoked and out_handled is set to 1.
 *
 * @param registry The registry.
 * @param event The input event (usually from window level).
 * @param out_handled Pointer set to 1 if the event was a hotkey, 0 otherwise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_hotkey_registry_process_event(struct ui_hotkey_registry *registry,
                                            const struct ui_event *event,
                                            int *out_handled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_HOTKEY_REGISTRY_H */
