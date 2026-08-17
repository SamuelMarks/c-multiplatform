/**
 * @file ui_hotkey_registry.h
 * @brief Global hotkey registration and processing.
 *
 * This header provides a system for registering keyboard shortcuts (chords)
 * and triggering associated callbacks when matching input events are processed.
 */

/* clang-format off */
#ifndef UI_HOTKEY_REGISTRY_H
#define UI_HOTKEY_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_event.h"

/* clang-format on */

/**
 * @brief Forward declaration of the hotkey registry structure.
 */
struct ui_hotkey_registry;

/**
 * @brief Callback invoked when a hotkey is triggered.
 *
 * @param user_data Opaque user data provided during registration.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_hotkey_callback_t)(void *user_data);

/**
 * @brief Represents a registered hotkey chord.
 */
struct ui_hotkey_chord {
  int key_code; /**< The primary key code to match. */
  unsigned int
      modifiers; /**< The required modifier flags (from `ui_modifier_flags`). */
};

/**
 * @brief Creates a new hotkey registry.
 *
 * @param out_registry Pointer to receive the allocated registry.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_hotkey_registry_create(struct ui_hotkey_registry **out_registry);

/**
 * @brief Destroys a hotkey registry.
 *
 * @param registry Pointer to the registry to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_hotkey_registry_destroy(struct ui_hotkey_registry *registry);

/**
 * @brief Registers a hotkey chord with a callback.
 *
 * @param registry Pointer to the registry.
 * @param chord The chord to register (key + modifiers).
 * @param callback The function to invoke when triggered.
 * @param user_data Opaque user data for the callback.
 * @param out_id Pointer to receive a unique registration ID used for
 * unregistering.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_hotkey_registry_register(struct ui_hotkey_registry *registry,
                                       struct ui_hotkey_chord chord,
                                       ui_hotkey_callback_t callback,
                                       void *user_data, int *out_id);

/**
 * @brief Unregisters a previously registered hotkey chord.
 *
 * @param registry Pointer to the registry.
 * @param id The unique registration ID.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_hotkey_registry_unregister(struct ui_hotkey_registry *registry,
                                         int id);

/**
 * @brief Processes an incoming event against the registry.
 *
 * If the event matches a registered hotkey, the callback is invoked.
 *
 * @param registry Pointer to the registry.
 * @param event Pointer to the input event (usually from window level).
 * @param out_handled Pointer set to 1 if the event triggered a hotkey, 0
 * otherwise.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_hotkey_registry_process_event(struct ui_hotkey_registry *registry,
                                            const struct ui_event *event,
                                            int *out_handled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_HOTKEY_REGISTRY_H */
