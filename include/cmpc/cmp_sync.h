#ifndef CMP_SYNC_H
#define CMP_SYNC_H

/**
 * @file cmp_sync.h
 * @brief Synchronization primitives (Mutex, Semaphore, CondVar) for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
/* clang-format on */

/**
 * @brief Opaque handle for a Mutex.
 */
typedef struct CMPMutex CMPMutex;

/**
 * @brief Create a new mutex.
 * @param alloc Allocator to use.
 * @param out_mutex Output mutex handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_mutex_create(CMPAllocator *alloc,
                                      CMPMutex **out_mutex);

/**
 * @brief Destroy a mutex.
 * @param alloc Allocator used during creation.
 * @param mutex The mutex to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_mutex_destroy(CMPAllocator *alloc, CMPMutex *mutex);

/**
 * @brief Lock a mutex. Blocks until the lock is acquired.
 * @param mutex The mutex to lock.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_mutex_lock(CMPMutex *mutex);

/**
 * @brief Try to lock a mutex without blocking.
 * @param mutex The mutex to lock.
 * @return CMP_OK if acquired, CMP_ERR_NOT_READY if already locked.
 */
CMP_API int CMP_CALL cmp_mutex_trylock(CMPMutex *mutex);

/**
 * @brief Unlock a mutex.
 * @param mutex The mutex to unlock.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_mutex_unlock(CMPMutex *mutex);

/**
 * @brief Opaque handle for a Semaphore.
 */
typedef struct CMPSemaphore CMPSemaphore;

/**
 * @brief Create a new semaphore.
 * @param alloc Allocator to use.
 * @param initial_count The initial count of the semaphore.
 * @param out_sem Output semaphore handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_semaphore_create(CMPAllocator *alloc,
                                          cmp_u32 initial_count,
                                          CMPSemaphore **out_sem);

/**
 * @brief Destroy a semaphore.
 * @param alloc Allocator used during creation.
 * @param sem The semaphore to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_semaphore_destroy(CMPAllocator *alloc,
                                           CMPSemaphore *sem);

/**
 * @brief Wait (decrement) on a semaphore. Blocks until count > 0.
 * @param sem The semaphore to wait on.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_semaphore_wait(CMPSemaphore *sem);

/**
 * @brief Post (increment) a semaphore. Wakes one waiting thread.
 * @param sem The semaphore to post to.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_semaphore_post(CMPSemaphore *sem);

/**
 * @brief Opaque handle for a Condition Variable.
 */
typedef struct CMPCondVar CMPCondVar;

/**
 * @brief Create a new condition variable.
 * @param alloc Allocator to use.
 * @param out_cond Output condition variable handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_condvar_create(CMPAllocator *alloc,
                                        CMPCondVar **out_cond);

/**
 * @brief Destroy a condition variable.
 * @param alloc Allocator used during creation.
 * @param cond The condition variable to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_condvar_destroy(CMPAllocator *alloc, CMPCondVar *cond);

/**
 * @brief Wait on a condition variable. Atomically unlocks the mutex and blocks.
 * @param cond The condition variable.
 * @param mutex The locked mutex to release while waiting.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_condvar_wait(CMPCondVar *cond, CMPMutex *mutex);

/**
 * @brief Wake one thread waiting on the condition variable.
 * @param cond The condition variable.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_condvar_signal(CMPCondVar *cond);

/**
 * @brief Wake all threads waiting on the condition variable.
 * @param cond The condition variable.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_condvar_broadcast(CMPCondVar *cond);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_SYNC_H */