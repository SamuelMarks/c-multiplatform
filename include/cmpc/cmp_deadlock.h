#ifndef CMP_DEADLOCK_H
#define CMP_DEADLOCK_H

/**
 * @file cmp_deadlock.h
 * @brief Optional deadlock detection for LibCMPC mutexes.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_sync.h"
/* clang-format on */

/**
 * @brief Initialize the deadlock detector subsystem.
 * @param alloc Allocator to use for internal graph structures.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL cmp_deadlock_detector_init(CMPAllocator *alloc);

/**
 * @brief Destroy the deadlock detector subsystem.
 * @return None.
 */
CMP_API void CMP_CALL cmp_deadlock_detector_destroy(void);

/**
 * @brief Record that the current thread is attempting to lock a mutex.
 * Analyzes the wait graph to check if acquiring this lock would cause a
 * deadlock.
 * @param mutex The mutex being requested.
 * @param location String describing the source location (e.g. file:line).
 * @return CMP_OK if safe, CMP_ERR_UNKNOWN if a cycle is detected.
 */
CMP_API int CMP_CALL cmp_deadlock_check_before_lock(CMPMutex *mutex,
                                                    const char *location);

/**
 * @brief Record that the current thread has successfully acquired a mutex.
 * @param mutex The mutex acquired.
 * @return None.
 */
CMP_API void CMP_CALL cmp_deadlock_record_lock(CMPMutex *mutex);

/**
 * @brief Record that the current thread is releasing a mutex.
 * @param mutex The mutex being released.
 * @return None.
 */
CMP_API void CMP_CALL cmp_deadlock_record_unlock(CMPMutex *mutex);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_DEADLOCK_H */