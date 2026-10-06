/**
 * @file sampler_preferences.h
 * @brief User preferences and favorite route persistence for Compose Material
 * Catalog.
 */

#ifndef SAMPLER_PREFERENCES_H
#define SAMPLER_PREFERENCES_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include "sampler/sampler_theme.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct sampler_preferences;

/**
 * @brief Create an in-memory preferences storage instance (ideal for tests and
 * volatile sessions).
 * @param out_prefs Pointer to receiving pointer to sampler_preferences.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_OUT_OF_MEMORY.
 */
sampler_error_t
sampler_preferences_create_in_memory(struct sampler_preferences **out_prefs);

/**
 * @brief Create a filesystem-backed preferences instance persisting to
 * file_path.
 * @param file_path Path on disk to the preferences JSON / INI file.
 * @param out_prefs Pointer to receiving pointer to sampler_preferences.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t
sampler_preferences_create_file(const char *file_path,
                                struct sampler_preferences **out_prefs);

/**
 * @brief Free all resources associated with the preferences instance and
 * nullify pointer.
 * @param prefs Double pointer to preferences instance to destroy.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_preferences_destroy(struct sampler_preferences **prefs);

/**
 * @brief Persist a favorite route string.
 * @param prefs Preferences handle.
 * @param route Route string to pin as default.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t
sampler_preferences_save_favorite_route(struct sampler_preferences *prefs,
                                        const char *route);

/**
 * @brief Retrieve the saved favorite route string, if any.
 * @param prefs Preferences handle.
 * @param out_route Character buffer receiving route string.
 * @param max_len Size of out_route buffer in bytes.
 * @param out_has_route Pointer receiving 1 if a route was found, 0 otherwise.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t
sampler_preferences_get_favorite_route(const struct sampler_preferences *prefs,
                                       char *out_route, size_t max_len,
                                       int *out_has_route);

/**
 * @brief Clear and unpin any saved favorite route.
 * @param prefs Preferences handle.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t
sampler_preferences_clear_favorite_route(struct sampler_preferences *prefs);

/**
 * @brief Persist theme settings.
 * @param prefs Preferences handle.
 * @param theme Theme settings to store.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t
sampler_preferences_save_theme(struct sampler_preferences *prefs,
                               const struct sampler_theme *theme);

/**
 * @brief Retrieve persisted theme settings, if any.
 * @param prefs Preferences handle.
 * @param out_theme Struct receiving loaded theme settings.
 * @param out_has_theme Pointer receiving 1 if saved theme was found, 0
 * otherwise.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t
sampler_preferences_get_theme(const struct sampler_preferences *prefs,
                              struct sampler_theme *out_theme,
                              int *out_has_theme);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_PREFERENCES_H */
