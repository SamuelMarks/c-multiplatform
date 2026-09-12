/**
 * @file ui_ffi_runtime.h
 * @brief Foreign Function Interface (FFI) bindings for runtime schema injection
 * and AoT mounting.
 */

#ifndef UI_FFI_RUNTIME_H
#define UI_FFI_RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_app_load.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief FFI entry point to mount an application from a serialized JSON schema.
 *
 * @param json_schema The JSON schema string.
 * @param mount_host Host DOM node to attach the screen to.
 * @param out_root Pointer to receive the constructed root DOM element.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_ffi_app_load_runtime(const char *json_schema, struct ui_dom_node *mount_host,
                        struct ui_dom_node **out_root);

/**
 * @brief FFI entry point to mount an application using a compiled Ahead-of-Time
 * function pointer.
 *
 * @param aot_mount The compiled C function pointer.
 * @param mount_host Host DOM node to attach the screen to.
 * @param out_root Pointer to receive the constructed root DOM element.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_ffi_app_load_aot(ui_aot_mount_fn aot_mount, struct ui_dom_node *mount_host,
                    struct ui_dom_node **out_root);

/**
 * @brief FFI entry point to mount a dynamic runtime schema snippet into an
 * existing DOM node.
 *
 * @param snippet_json Serialized JSON widget node.
 * @param parent_node DOM container node to append into.
 * @param out_snippet_root Pointer to receive the mounted root DOM element.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_ffi_app_load_snippet(
    const char *snippet_json, struct ui_dom_node *parent_node,
    struct ui_dom_node **out_snippet_root);

/**
 * @brief FFI entry point to set an integer value in the global app state.
 *
 * @param key State key string.
 * @param val Integer value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_ffi_app_state_set_int(const char *key, int val);

/**
 * @brief FFI entry point to get an integer value from the global app state.
 *
 * @param key State key string.
 * @param out_val Pointer to receive integer value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_ffi_app_state_get_int(const char *key, int *out_val);

/**
 * @brief FFI entry point to set a string value in the global app state.
 *
 * @param key State key string.
 * @param val String value.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_ffi_app_state_set_string(const char *key, const char *val);

/**
 * @brief FFI entry point to get a string value from the global app state.
 *
 * @param key State key string.
 * @param out_val Pointer to receive string pointer.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_ffi_app_state_get_string(const char *key, const char **out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FFI_RUNTIME_H */
