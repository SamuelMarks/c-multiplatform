#ifndef CMP_GLOBAL_HOTKEY_H
#define CMP_GLOBAL_HOTKEY_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_global_hotkey.h
 * @brief Global OS hotkey registration primitives.
 */

typedef struct cmp_global_hotkey cmp_global_hotkey_t;

/**
 * @brief Creates a global hotkey manager instance.
 * @param out_hotkey Pointer to receive the created instance.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_global_hotkey_create(cmp_global_hotkey_t **out_hotkey);

/**
 * @brief Destroys a global hotkey manager instance.
 * @param hotkey The instance to destroy.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_global_hotkey_destroy(cmp_global_hotkey_t *hotkey);

/**
 * @brief Registers a global hotkey.
 * @param hotkey The hotkey manager instance.
 * @param key_code The key code (e.g., 'C' for Ctrl+C).
 * @param modifiers The modifiers bitmask (1=Ctrl, 2=Alt, 4=Shift, 8=Meta).
 * @param out_id Pointer to receive a hotkey ID.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_global_hotkey_register(cmp_global_hotkey_t *hotkey,
                                       int key_code, int modifiers,
                                       int *out_id);

/**
 * @brief Unregisters a global hotkey.
 * @param hotkey The hotkey manager instance.
 * @param id The hotkey ID.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_global_hotkey_unregister(cmp_global_hotkey_t *hotkey, int id);

#ifdef __cplusplus
}
#endif

#endif /* CMP_GLOBAL_HOTKEY_H */
