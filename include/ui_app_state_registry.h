/**
 * @file ui_app_state_registry.h
 * @brief Global application state registry for top-level reactive signals and
 * state sharing.
 */

#ifndef UI_APP_STATE_REGISTRY_H
#define UI_APP_STATE_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_arena.h"
#include <stddef.h>
/* clang-format on */

struct ui_app_state_registry;

/**
 * @brief Creates a new application state registry.
 *
 * @param arena Memory arena to allocate from (or NULL for default heap).
 * @param out_registry Pointer to receive the allocated registry.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_app_state_registry_create(
    struct ui_arena *arena, struct ui_app_state_registry **out_registry);

/**
 * @brief Destroys an application state registry.
 *
 * @param registry The registry to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_app_state_registry_destroy(struct ui_app_state_registry *registry);

/**
 * @brief Registers a signal under a global string key (e.g. "cart_count").
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param signal The signal to register.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_app_state_registry_register_signal(
    struct ui_app_state_registry *registry, const char *key,
    struct ui_signal *signal);

/**
 * @brief Looks up a registered signal by key.
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param out_signal Pointer to receive the found signal.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not present.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_app_state_registry_get_signal(
    const struct ui_app_state_registry *registry, const char *key,
    struct ui_signal **out_signal);

/**
 * @brief Sets the value of a registered signal in the registry.
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param payload The new value payload.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_app_state_registry_set_value(
    struct ui_app_state_registry *registry, const char *key,
    union ui_signal_payload payload);

/**
 * @brief Retrieves the current value of a registered signal in the registry.
 *
 * @param registry The registry.
 * @param key The state key identifier.
 * @param out_payload Pointer to receive the signal payload.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_app_state_registry_get_value(
    const struct ui_app_state_registry *registry, const char *key,
    union ui_signal_payload *out_payload);

/**
 * @brief Retrieves the global singleton application state registry.
 *
 * @param out_registry Pointer to receive the global registry instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_app_state_registry_get_global(struct ui_app_state_registry **out_registry);

/**
 * @brief Shuts down and destroys the global singleton application state
 * registry.
 *
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_app_state_registry_shutdown_global(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_APP_STATE_REGISTRY_H */
