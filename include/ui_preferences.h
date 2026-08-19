/**
 * @file ui_preferences.h
 * @brief Preferences and persistent key-value storage.
 */

#ifndef UI_PREFERENCES_H
#define UI_PREFERENCES_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include "ui_promise.h"
#include "ui_thread_pool.h"
#include "ui_execution_context.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle representing the preferences manager.
 */
struct ui_preferences;

/**
 * @brief Creates a new preferences manager.
 *
 * @param pool The thread pool to use for background operations.
 * @param ctx The execution context to use for resolving promises.
 * @param out_prefs Pointer to receive the new preferences handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_preferences_create(struct ui_thread_pool *pool,
                                 struct ui_execution_context *ctx,
                                 struct ui_preferences **out_prefs);

/**
 * @brief Destroys a preferences manager and frees its resources.
 *
 * @param prefs The preferences manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_preferences_destroy(struct ui_preferences *prefs);

/**
 * @brief Sets a string value for a given key.
 *
 * @param prefs The preferences manager.
 * @param key The key to set.
 * @param value The value to set.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_preferences_set_string(struct ui_preferences *prefs,
                                     const char *key, const char *value);

/**
 * @brief Gets a string value for a given key.
 *
 * @param prefs The preferences manager.
 * @param key The key to get.
 * @param out_value Pointer to receive the allocated string value. Must be freed
 * with C_MULTIPLATFORM_FREE.
 * @return UI_ERROR_NONE on success. UI_ERROR_NOT_FOUND if the key does not
 * exist.
 */
ui_error_t ui_preferences_get_string(struct ui_preferences *prefs,
                                     const char *key, char **out_value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PREFERENCES_H */

/**
 * @brief Saves binary data asynchronously to persistent storage (e.g.
 * IndexedDB).
 *
 * @param prefs The preferences manager.
 * @param key The key to save under.
 * @param data The binary data to save.
 * @param length The length of the binary data.
 * @param out_promise Pointer to receive the promise representing the async
 * operation.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_preferences_save_binary_async(struct ui_preferences *prefs,
                                            const char *key, const void *data,
                                            size_t length,
                                            struct ui_promise **out_promise);
