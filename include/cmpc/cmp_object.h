#ifndef CMP_OBJECT_H
#define CMP_OBJECT_H

/**
 * @file cmp_object.h
 * @brief Handle and object system ABI for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"

/**
 * @brief Opaque handle used to reference objects across API boundaries.
 */
typedef struct CMPHandle {
  cmp_u32 id;         /**< Slot identifier within a handle table. */
  cmp_u32 generation; /**< Generation counter for stale handle detection. */
} CMPHandle;

/**
 * @brief Base object header embedded in all handle-managed objects.
 */
typedef struct CMPObjectHeader {
  CMPHandle handle;                     /**< Handle for the object. */
  cmp_u32 type_id;                      /**< Type identifier for RTTI. */
  cmp_u32 flags;                        /**< Implementation-defined flags. */
  cmp_u32 ref_count;                    /**< Reference count. */
  const struct CMPObjectVTable *vtable; /**< Object virtual table. */
} CMPObjectHeader;

/**
 * @brief Object virtual table for retain/release and destruction.
 */
typedef struct CMPObjectVTable {
  /**
   * @brief Retain a reference to the object.
   * @param obj Object instance.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int retain(void *obj);
  #else
  int(CMP_CALL *retain)(void *obj);
  #endif
  /**
   * @brief Release a reference to the object.
   * @param obj Object instance.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int release(void *obj);
  #else
  int(CMP_CALL *release)(void *obj);
  #endif
  /**
   * @brief Destroy the object and free its resources.
   * @param obj Object instance.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int destroy(void *obj);
  #else
  int(CMP_CALL *destroy)(void *obj);
  #endif
  /**
   * @brief Retrieve the type identifier for the object.
   * @param obj Object instance.
   * @param out_type_id Receives the type identifier.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int get_type_id(void *obj, cmp_u32 *out_type_id);
  #else
  int(CMP_CALL *get_type_id)(void *obj, cmp_u32 *out_type_id);
  #endif
} CMPObjectVTable;

/**
 * @brief Handle system virtual table.
 */
typedef struct CMPHandleSystemVTable {
  /**
   * @brief Initialize the handle system.
   * @param handle_system Handle system instance.
   * @param capacity Maximum number of handles.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int init(void *handle_system, cmp_usize capacity);
  #else
  int(CMP_CALL *init)(void *handle_system, cmp_usize capacity);
  #endif
  /**
   * @brief Shut down the handle system.
   * @param handle_system Handle system instance.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int shutdown(void *handle_system);
  #else
  int(CMP_CALL *shutdown)(void *handle_system);
  #endif
  /**
   * @brief Register an object with the handle system.
   * @param handle_system Handle system instance.
   * @param obj Object header to register.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int register_object(void *handle_system, CMPObjectHeader *obj);
  #else
  int(CMP_CALL *register_object)(void *handle_system, CMPObjectHeader *obj);
  #endif
  /**
   * @brief Unregister a handle from the system.
   * @param handle_system Handle system instance.
   * @param handle Handle to remove.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int unregister_object(void *handle_system, CMPHandle handle);
  #else
  int(CMP_CALL *unregister_object)(void *handle_system, CMPHandle handle);
  #endif
  /**
   * @brief Resolve a handle to its object pointer.
   * @param handle_system Handle system instance.
   * @param handle Handle to resolve.
   * @param out_obj Receives the resolved object pointer.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int resolve(void *handle_system, CMPHandle handle, void **out_obj);
  #else
  int(CMP_CALL *resolve)(void *handle_system, CMPHandle handle, void **out_obj);
  #endif
  /**
   * @brief Retain a handle-managed object.
   * @param handle_system Handle system instance.
   * @param handle Handle to retain.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int retain(void *handle_system, CMPHandle handle);
  #else
  int(CMP_CALL *retain)(void *handle_system, CMPHandle handle);
  #endif
  /**
   * @brief Release a handle-managed object.
   * @param handle_system Handle system instance.
   * @param handle Handle to release.
   * @return CMP_OK on success or a failure code.
   */
  #ifdef CMP_DOXYGEN
  int release(void *handle_system, CMPHandle handle);
  #else
  int(CMP_CALL *release)(void *handle_system, CMPHandle handle);
  #endif
} CMPHandleSystemVTable;

/**
 * @brief Handle system interface.
 */
typedef struct CMPHandleSystem {
  void *ctx;                          /**< Handle system context pointer. */
  const CMPHandleSystemVTable *vtable; /**< Handle system virtual table. */
} CMPHandleSystem;

/**
 * @brief Check whether a handle is structurally valid.
 * @param handle Handle to validate.
 * @param out_valid Receives CMP_TRUE if valid; CMP_FALSE otherwise.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_handle_is_valid(CMPHandle handle, CMPBool *out_valid);

/**
 * @brief Initialize an object header.
 * @param obj Object header to initialize.
 * @param type_id Type identifier for RTTI.
 * @param flags Initial object flags.
 * @param vtable Object virtual table.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_object_header_init(CMPObjectHeader *obj, cmp_u32 type_id,
                                         cmp_u32 flags,
                                         const CMPObjectVTable *vtable);

/**
 * @brief Retain a reference to an object header.
 * @param obj Object header.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_object_retain(CMPObjectHeader *obj);

/**
 * @brief Release a reference to an object header.
 * @param obj Object header.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_object_release(CMPObjectHeader *obj);

/**
 * @brief Retrieve the RTTI type identifier from an object header.
 * @param obj Object header.
 * @param out_type_id Receives the type identifier.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_object_get_type_id(const CMPObjectHeader *obj,
                                         cmp_u32 *out_type_id);

/**
 * @brief Create the default handle system implementation.
 * @param capacity Maximum number of handles to manage.
 * @param allocator Allocator used for handle system memory; NULL uses the
 * default allocator.
 * @param out_sys Receives the initialized handle system interface.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_handle_system_default_create(cmp_usize capacity,
                                                   const CMPAllocator *allocator,
                                                   CMPHandleSystem *out_sys);

/**
 * @brief Destroy a handle system created by the default implementation.
 * @param sys Handle system to destroy.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_handle_system_default_destroy(CMPHandleSystem *sys);

#ifdef CMP_TESTING
/**
 * @brief Force handle validation to fail when enabled (tests only).
 * @param fail CMP_TRUE to force failure, CMP_FALSE to restore normal behavior.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_object_test_set_handle_is_valid_fail(CMPBool fail);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_OBJECT_H */
