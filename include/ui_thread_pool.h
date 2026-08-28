/**
 * @file ui_thread_pool.h
 * @brief Defines the thread pool task executor.
 */
/** @cond */
#ifndef UI_THREAD_POOL_H
#define UI_THREAD_POOL_H
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a thread pool task executor.
 */
struct ui_thread_pool;

/**
 * @brief Creates a new thread pool.
 *
 * @param num_threads The number of worker threads to spawn.
 * @param out_pool Pointer to receive the new pool handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_thread_pool_create(int num_threads,
                                 struct ui_thread_pool **out_pool);

/**
 * @brief Destroys a thread pool, waiting for all queued tasks to finish.
 *
 * @param pool The pool to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if pool is NULL.
 */
ui_error_t ui_thread_pool_destroy(struct ui_thread_pool *pool);

/**
 * @brief Schedules a task for background execution.
 *
 * @param pool The pool to schedule the task on.
 * @param callback The function to execute.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_thread_pool_schedule(struct ui_thread_pool *pool,
                                   ui_error_t (*callback)(void *),
                                   void *user_data);

/**
 * @brief Processes all scheduled tasks synchronously (used only when
 * UI_SINGLE_THREADED is defined).
 *
 * @param pool The pool to tick.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_thread_pool_tick(struct ui_thread_pool *pool);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_THREAD_POOL_H */
