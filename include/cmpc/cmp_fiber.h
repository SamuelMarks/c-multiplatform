#ifndef CMP_FIBER_H
#define CMP_FIBER_H

/**
 * @file cmp_fiber.h
 * @brief Cooperative greenthreads/fibers for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_fs.h"
/* clang-format on */

/**
 * @brief Opaque handle for a fiber.
 */
typedef struct CMPFiber CMPFiber;

/**
 * @brief Signature for a fiber's execution routine.
 * @param arg User-defined argument passed to the fiber.
 * @return None.
 */
typedef void(CMP_CALL *CMPFiberRoutine)(void *arg);

/**
 * @brief Configuration for spawning a fiber.
 */
typedef struct CMPFiberConfig {
  CMPFiberRoutine routine; /**< The routine to execute. */
  void *arg;               /**< Argument to pass to the routine. */
  cmp_usize stack_size;    /**< Stack size in bytes (0 for default). */
} CMPFiberConfig;

/**
 * @brief Opaque handle for a fiber scheduler.
 */
typedef struct CMPFiberScheduler CMPFiberScheduler;

/**
 * @brief Create a new fiber scheduler.
 * @param alloc Allocator to use.
 * @param out_scheduler Output scheduler handle.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_fiber_scheduler_create(
    CMPAllocator *alloc, CMPFiberScheduler **out_scheduler);

/**
 * @brief Destroy a fiber scheduler.
 * @param alloc Allocator used during creation.
 * @param scheduler The scheduler to destroy.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_fiber_scheduler_destroy(CMPAllocator *alloc,
                                                 CMPFiberScheduler *scheduler);

/**
 * @brief Spawn a new fiber on the given scheduler.
 * @param scheduler The scheduler to own the fiber.
 * @param config Fiber configuration.
 * @param out_fiber Output fiber handle (optional).
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_fiber_spawn(CMPFiberScheduler *scheduler,
                                     const CMPFiberConfig *config,
                                     CMPFiber **out_fiber);

/**
 * @brief Yield the currently executing fiber back to the scheduler.
 * Must only be called from within a executing fiber routine.
 * @return None.
 */
CMP_API void CMP_CALL cmp_fiber_yield(void);

/**
 * @brief Run the fiber scheduler, executing ready fibers round-robin.
 * Usually called from the main event loop or a dedicated worker thread.
 * @param scheduler The scheduler to run.
 * @return Number of fibers that were executed in this pass, or < 0 on error.
 */
CMP_API int CMP_CALL cmp_fiber_scheduler_run_pass(CMPFiberScheduler *scheduler);

/**
 * @brief Wait for a specified number of milliseconds inside a fiber without
 * blocking the thread. Yields back to the scheduler until the time has elapsed.
 * @param ms The number of milliseconds to sleep.
 * @return None.
 */
CMP_API void CMP_CALL cmp_fiber_sleep(cmp_u32 ms);

/**
 * @brief Read a file asynchronously from within a fiber, returning only when
 * complete. This makes asynchronous I/O look synchronous.
 * @param env Environment interface.
 * @param tasks Task interface.
 * @param loop Event loop interface.
 * @param alloc Allocator.
 * @param utf8_path Path to the file.
 * @param out_data Output data pointer (must be freed by caller).
 * @param out_size Output size of data.
 * @return CMP_OK on success or an error code.
 */
CMP_API int CMP_CALL cmp_fiber_fs_read(CMPEnv *env, CMPTasks *tasks,
                                       CMPEventLoop *loop,
                                       const CMPAllocator *alloc,
                                       const char *utf8_path, void **out_data,
                                       cmp_usize *out_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_FIBER_H */