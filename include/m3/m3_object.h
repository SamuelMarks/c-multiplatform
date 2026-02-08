#ifndef M3_OBJECT_H
#define M3_OBJECT_H

/**
 * @file m3_object.h
 * @brief Handle and object system ABI for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_core.h"

/**
 * @brief Opaque handle used to reference objects across API boundaries.
 */
typedef struct M3Handle {
    m3_u32 id; /**< Slot identifier within a handle table. */
    m3_u32 generation; /**< Generation counter for stale handle detection. */
} M3Handle;

/**
 * @brief Base object header embedded in all handle-managed objects.
 */
typedef struct M3ObjectHeader {
    M3Handle handle; /**< Handle for the object. */
    m3_u32 type_id; /**< Type identifier for RTTI. */
    m3_u32 flags; /**< Implementation-defined flags. */
    m3_u32 ref_count; /**< Reference count. */
    const struct M3ObjectVTable *vtable; /**< Object virtual table. */
} M3ObjectHeader;

/**
 * @brief Object virtual table for retain/release and destruction.
 */
typedef struct M3ObjectVTable {
    /**
     * @brief Retain a reference to the object.
     * @param obj Object instance.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *retain)(void *obj);
    /**
     * @brief Release a reference to the object.
     * @param obj Object instance.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *release)(void *obj);
    /**
     * @brief Destroy the object and free its resources.
     * @param obj Object instance.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *destroy)(void *obj);
    /**
     * @brief Retrieve the type identifier for the object.
     * @param obj Object instance.
     * @param out_type_id Receives the type identifier.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *get_type_id)(void *obj, m3_u32 *out_type_id);
} M3ObjectVTable;

/**
 * @brief Handle system virtual table.
 */
typedef struct M3HandleSystemVTable {
    /**
     * @brief Initialize the handle system.
     * @param sys Handle system instance.
     * @param capacity Maximum number of handles.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *init)(void *sys, m3_usize capacity);
    /**
     * @brief Shut down the handle system.
     * @param sys Handle system instance.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *shutdown)(void *sys);
    /**
     * @brief Register an object with the handle system.
     * @param sys Handle system instance.
     * @param obj Object header to register.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *register_object)(void *sys, M3ObjectHeader *obj);
    /**
     * @brief Unregister a handle from the system.
     * @param sys Handle system instance.
     * @param handle Handle to remove.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *unregister_object)(void *sys, M3Handle handle);
    /**
     * @brief Resolve a handle to its object pointer.
     * @param sys Handle system instance.
     * @param handle Handle to resolve.
     * @param out_obj Receives the resolved object pointer.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *resolve)(void *sys, M3Handle handle, void **out_obj);
    /**
     * @brief Retain a handle-managed object.
     * @param sys Handle system instance.
     * @param handle Handle to retain.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *retain)(void *sys, M3Handle handle);
    /**
     * @brief Release a handle-managed object.
     * @param sys Handle system instance.
     * @param handle Handle to release.
     * @return M3_OK on success or a failure code.
     */
    int (M3_CALL *release)(void *sys, M3Handle handle);
} M3HandleSystemVTable;

/**
 * @brief Handle system interface.
 */
typedef struct M3HandleSystem {
    void *ctx; /**< Handle system context pointer. */
    const M3HandleSystemVTable *vtable; /**< Handle system virtual table. */
} M3HandleSystem;

/**
 * @brief Check whether a handle is structurally valid.
 * @param handle Handle to validate.
 * @param out_valid Receives M3_TRUE if valid; M3_FALSE otherwise.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_handle_is_valid(M3Handle handle, M3Bool *out_valid);

/**
 * @brief Initialize an object header.
 * @param obj Object header to initialize.
 * @param type_id Type identifier for RTTI.
 * @param flags Initial object flags.
 * @param vtable Object virtual table.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_object_header_init(M3ObjectHeader *obj, m3_u32 type_id, m3_u32 flags, const M3ObjectVTable *vtable);

/**
 * @brief Retain a reference to an object header.
 * @param obj Object header.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_object_retain(M3ObjectHeader *obj);

/**
 * @brief Release a reference to an object header.
 * @param obj Object header.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_object_release(M3ObjectHeader *obj);

/**
 * @brief Retrieve the RTTI type identifier from an object header.
 * @param obj Object header.
 * @param out_type_id Receives the type identifier.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_object_get_type_id(const M3ObjectHeader *obj, m3_u32 *out_type_id);

/**
 * @brief Create the default handle system implementation.
 * @param capacity Maximum number of handles to manage.
 * @param allocator Allocator used for handle system memory; NULL uses the default allocator.
 * @param out_sys Receives the initialized handle system interface.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_handle_system_default_create(m3_usize capacity, const M3Allocator *allocator, M3HandleSystem *out_sys);

/**
 * @brief Destroy a handle system created by the default implementation.
 * @param sys Handle system to destroy.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_handle_system_default_destroy(M3HandleSystem *sys);

#ifdef M3_TESTING
/**
 * @brief Force handle validation to fail when enabled (tests only).
 * @param fail M3_TRUE to force failure, M3_FALSE to restore normal behavior.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_object_test_set_handle_is_valid_fail(M3Bool fail);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_OBJECT_H */
