#ifndef CMP_PLUGIN_LOADER_H
#define CMP_PLUGIN_LOADER_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_plugin_loader.h
 * @brief Dynamic library loader and IPC mechanisms for plugins.
 */

typedef struct cmp_plugin_loader cmp_plugin_loader_t;

/**
 * @brief Creates a plugin loader instance.
 * @param out_loader Pointer to receive the created instance.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_plugin_loader_create(cmp_plugin_loader_t **out_loader);

/**
 * @brief Destroys a plugin loader instance.
 * @param loader The instance to destroy.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_plugin_loader_destroy(cmp_plugin_loader_t *loader);

/**
 * @brief Loads a plugin shared library (.dll, .so, .dylib).
 * @param loader The loader instance.
 * @param path The path to the plugin.
 * @param out_plugin_id Pointer to receive the plugin ID.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_plugin_loader_load(cmp_plugin_loader_t *loader,
                                   const char *path, int *out_plugin_id);

/**
 * @brief Unloads a plugin.
 * @param loader The loader instance.
 * @param plugin_id The plugin ID to unload.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_plugin_loader_unload(cmp_plugin_loader_t *loader,
                                     int plugin_id);

/**
 * @brief Executes a function inside the plugin via mock IPC or direct symbol
 * lookup.
 * @param loader The loader instance.
 * @param plugin_id The plugin ID.
 * @param function_name The function name.
 * @param payload JSON or string payload to pass.
 * @param out_response Pointer to receive the response. Must be freed.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_plugin_loader_execute(cmp_plugin_loader_t *loader,
                                      int plugin_id, const char *function_name,
                                      const char *payload, char **out_response);

/**
 * @brief Frees a string returned by execute.
 * @param response The string to free.
 * @return 0 on success, or an error code.
 */
int CMP_API cmp_plugin_loader_free_response(char *response);
#ifdef __cplusplus
}
#endif

#endif /* CMP_PLUGIN_LOADER_H */
