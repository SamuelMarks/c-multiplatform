/**
 * @file ui_handle_manager.h
 * @brief Thread-safe handle manager for opaque resource tracking.
 */

#ifndef UI_HANDLE_MANAGER_H
#define UI_HANDLE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

/**
 * @brief Opaque handle representing the thread-safe handle manager context.
 */
struct ui_handle_manager;

/**
 * @brief Represents an invalid handle.
 */
#define UI_INVALID_HANDLE ((ui_uint64)0)

/**
 * @brief Creates a handle manager.
 *
 * @param capacity Initial capacity of the handle table.
 * @param out_manager Pointer to receive the new handle manager.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_handle_manager_create(ui_uint32 capacity,
                                    struct ui_handle_manager **out_manager);

/**
 * @brief Destroys a handle manager.
 *
 * @param manager The manager to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_handle_manager_destroy(struct ui_handle_manager *manager);

/**
 * @brief Allocates a new handle and associates it with the given data pointer.
 *
 * @param manager The handle manager.
 * @param data The data pointer to associate.
 * @param out_handle Pointer to receive the allocated handle.
 * @return UI_ERROR_NONE on success, UI_ERROR_QUEUE_FULL if out of handles.
 */
ui_error_t ui_handle_manager_alloc(struct ui_handle_manager *manager,
                                   void *data, ui_uint64 *out_handle);

/**
 * @brief Gets the data pointer associated with a handle.
 *
 * @param manager The handle manager.
 * @param handle The handle to look up.
 * @param out_data Pointer to receive the data pointer.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if invalid or
 * expired handle.
 */
ui_error_t ui_handle_manager_get(struct ui_handle_manager *manager,
                                 ui_uint64 handle, void **out_data);

/**
 * @brief Frees a handle, making its slot available for reuse and incrementing
 * the generation.
 *
 * @param manager The handle manager.
 * @param handle The handle to free.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if invalid.
 */
ui_error_t ui_handle_manager_free(struct ui_handle_manager *manager,
                                  ui_uint64 handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_HANDLE_MANAGER_H */
