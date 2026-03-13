#ifndef CMP_SHM_H
#define CMP_SHM_H

/**
 * @file cmp_shm.h
 * @brief Shared Memory Allocator for LibCMPC IPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
/* clang-format on */

/**
 * @brief Opaque handle for a shared memory region.
 */
typedef struct CMPSharedMemory CMPSharedMemory;

/**
 * @brief Create a named shared memory region.
 * @param name The globally unique name of the region.
 * @param size The size of the region in bytes.
 * @param out_shm Output shared memory handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_shm_create(const char *name, cmp_usize size,
                                    CMPSharedMemory **out_shm);

/**
 * @brief Open an existing named shared memory region.
 * @param name The name of the region to open.
 * @param size The size of the region in bytes.
 * @param out_shm Output shared memory handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_shm_open(const char *name, cmp_usize size,
                                  CMPSharedMemory **out_shm);

/**
 * @brief Get the memory pointer of the shared region.
 * @param shm The shared memory handle.
 * @return Pointer to the mapped memory, or NULL on error.
 */
CMP_API int CMP_CALL cmp_shm_get_ptr(CMPSharedMemory *shm, void **out_ptr);

/**
 * @brief Close the shared memory handle (unmap it from this process).
 * @param shm The handle to close.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_shm_close(CMPSharedMemory *shm);

/**
 * @brief Unlink/destroy the named shared memory region.
 * Once unlinked, new open requests will fail. The memory is freed when
 * the last process closes it.
 * @param name The name of the region to unlink.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_shm_unlink(const char *name);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_SHM_H */